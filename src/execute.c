#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "shell.h"

/* Simple linked list for background jobs */
typedef struct job {
    pid_t pid;
    char *cmdline;        /* strdup stored */
    struct job *next;
} job_t;

static job_t *jobs_head = NULL;

/* Add a job (pid, cmdline) to list */
static void jobs_add(pid_t pid, const char *cmdline) {
    job_t *j = malloc(sizeof(job_t));
    if (!j) return;
    j->pid = pid;
    j->cmdline = cmdline ? strdup(cmdline) : strdup("");
    j->next = jobs_head;
    jobs_head = j;
}

/* Remove job with pid */
static void jobs_remove_pid(pid_t pid) {
    job_t **pp = &jobs_head;
    while (*pp) {
        if ((*pp)->pid == pid) {
            job_t *tmp = *pp;
            *pp = tmp->next;
            free(tmp->cmdline);
            free(tmp);
            return;
        }
        pp = &(*pp)->next;
    }
}

/* Print active background jobs */
void jobs_print(void) {
    job_t *p = jobs_head;
    if (!p) {
        printf("No background jobs\n");
        return;
    }
    printf("Background jobs:\n");
    while (p) {
        printf("PID %d\t%s\n", (int)p->pid, p->cmdline ? p->cmdline : "");
        p = p->next;
    }
}

/* Reap any finished background jobs (non-blocking) and remove them from list */
void jobs_reap(void) {
    int status;
    pid_t pid;
    /* loop until no more exited children */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        /* remove from job list and report */
        job_t *p = jobs_head;
        job_t *found = NULL;
        while (p) {
            if (p->pid == pid) { found = p; break; }
            p = p->next;
        }
        if (found) {
            printf("\n[Done] PID %d\t%s\n", (int)found->pid, found->cmdline ? found->cmdline : "");
            jobs_remove_pid(pid);
        } else {
            /* if not in our list, ignore */
        }
    }
}

/* Helper: open file for writing (truncate) */
static int open_output_file(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    return fd;
}

/* Helper: open file for reading */
static int open_input_file(const char *path) {
    int fd = open(path, O_RDONLY);
    return fd;
}

/* Execute a pipeline. If background == 1, parent will not wait and will add the
   first child's pid (and subsequent child pids) to job list.
   cmdline is used for job display in jobs list.
*/
void execute_pipeline(pipeline_t *pl, int background, const char *cmdline) {
    if (!pl || pl->count == 0) return;

    int n = pl->count;
    if (n == 1) {
        /* single command: handle redirection in child */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return;
        } else if (pid == 0) {
            command_t *c = &pl->commands[0];
            if (c->infile) {
                int fd = open_input_file(c->infile);
                if (fd < 0) { fprintf(stderr, "open %s: %s\n", c->infile, strerror(errno)); _exit(1); }
                dup2(fd, STDIN_FILENO); close(fd);
            }
            if (c->outfile) {
                int fd = open_output_file(c->outfile);
                if (fd < 0) { fprintf(stderr, "open %s: %s\n", c->outfile, strerror(errno)); _exit(1); }
                dup2(fd, STDOUT_FILENO); close(fd);
            }
            execvp(c->argv[0], c->argv);
            fprintf(stderr, "execvp(%s): %s\n", c->argv[0], strerror(errno));
            _exit(127);
        } else {
            if (background) {
                jobs_add(pid, cmdline);
                printf("[Background] PID %d\n", (int)pid);
                /* do not wait */
                return;
            } else {
                int st;
                waitpid(pid, &st, 0);
                return;
            }
        }
    }

    /* multiple commands: set up pipes */
    int pipes[2 * (n - 1)];
    for (int i = 0; i < n - 1; ++i) {
        if (pipe(pipes + i*2) < 0) {
            perror("pipe");
            return;
        }
    }

    pid_t *pids = malloc(n * sizeof(pid_t));
    if (!pids) {
        perror("malloc");
        for (int j = 0; j < 2*(n-1); ++j) close(pipes[j]);
        return;
    }

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            /* cleanup */
            for (int k = 0; k < i; ++k) waitpid(pids[k], NULL, 0);
            for (int j = 0; j < 2*(n-1); ++j) close(pipes[j]);
            free(pids);
            return;
        } else if (pid == 0) {
            /* child */
            /* stdin from previous pipe if any */
            if (i > 0) {
                if (dup2(pipes[(i-1)*2], STDIN_FILENO) < 0) { perror("dup2"); _exit(1); }
            }
            /* stdout to next pipe if any */
            if (i < n-1) {
                if (dup2(pipes[i*2 + 1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(1); }
            }
            /* close all pipe fds */
            for (int j = 0; j < 2*(n-1); ++j) close(pipes[j]);

            /* redirections that override pipes for this command */
            command_t *c = &pl->commands[i];
            if (c->infile) {
                int fd = open_input_file(c->infile);
                if (fd < 0) { fprintf(stderr, "open %s: %s\n", c->infile, strerror(errno)); _exit(1); }
                dup2(fd, STDIN_FILENO); close(fd);
            }
            if (c->outfile) {
                int fd = open_output_file(c->outfile);
                if (fd < 0) { fprintf(stderr, "open %s: %s\n", c->outfile, strerror(errno)); _exit(1); }
                dup2(fd, STDOUT_FILENO); close(fd);
            }

            execvp(c->argv[0], c->argv);
            fprintf(stderr, "execvp(%s): %s\n", c->argv[0], strerror(errno));
            _exit(127);
        } else {
            /* parent */
            pids[i] = pid;
        }
    }

    /* close pipes in parent */
    for (int j = 0; j < 2*(n-1); ++j) close(pipes[j]);

    if (background) {
        /* Add each child pid to job list */
        for (int i = 0; i < n; ++i) jobs_add(pids[i], cmdline);
        printf("[Background] PIDs:");
        for (int i = 0; i < n; ++i) printf(" %d", (int)pids[i]);
        printf("\n");
        free(pids);
        return; /* do not wait */
    } else {
        /* wait for all children */
        for (int i = 0; i < n; ++i) {
            int st;
            waitpid(pids[i], &st, 0);
        }
        free(pids);
        return;
    }
}
