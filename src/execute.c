#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

#include "shell.h"

#define MAX_JOBS 64
#define MAX_VARS 128

/* ---------------- JOB CONTROL ---------------- */

static struct {
    pid_t pid;
    char cmd[256];
} jobs[MAX_JOBS];
static int job_count = 0;

/* ---------------- VARIABLES ---------------- */

static struct {
    char name[64];
    char value[256];
} vars[MAX_VARS];
static int var_count = 0;

static const char *var_get(const char *name) {
    for (int i = 0; i < var_count; i++)
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    return NULL;
}

static void var_set(const char *name, const char *value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            strncpy(vars[i].value, value, sizeof(vars[i].value) - 1);
            vars[i].value[sizeof(vars[i].value) - 1] = '\0';
            return;
        }
    }
    if (var_count < MAX_VARS) {
        strncpy(vars[var_count].name, name, sizeof(vars[var_count].name) - 1);
        vars[var_count].name[sizeof(vars[var_count].name) - 1] = '\0';
        strncpy(vars[var_count].value, value, sizeof(vars[var_count].value) - 1);
        vars[var_count].value[sizeof(vars[var_count].value) - 1] = '\0';
        var_count++;
    }
}

static void var_unset(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            for (int j = i; j < var_count - 1; j++)
                vars[j] = vars[j + 1];
            var_count--;
            return;
        }
    }
}

/* Expand $VAR inside a string */
static char *expand_vars(const char *input) {
    static char buf[512];
    buf[0] = '\0';

    const char *p = input;
    while (*p) {
        if (*p == '$') {
            p++;
            char name[64];
            int n = 0;
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                if (n < (int)sizeof(name) - 1)
                    name[n++] = *p;
                p++;
            }
            name[n] = '\0';
            const char *val = var_get(name);
            if (val)
                strncat(buf, val, sizeof(buf) - strlen(buf) - 1);
        } else {
            char tmp[2] = {*p, '\0'};
            strncat(buf, tmp, sizeof(buf) - strlen(buf) - 1);
            p++;
        }
    }
    return buf;
}

/* ---------------- JOBS FUNCTIONS ---------------- */

void jobs_add(pid_t pid, const char *cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].cmd, cmd, sizeof(jobs[job_count].cmd) - 1);
        jobs[job_count].cmd[sizeof(jobs[job_count].cmd) - 1] = '\0';
        job_count++;
    }
}

void jobs_reap(void) {
    int status;
    pid_t pid;
    for (int i = 0; i < job_count;) {
        pid = waitpid(jobs[i].pid, &status, WNOHANG);
        if (pid > 0) {
            printf("[done] %s (pid %d)\n", jobs[i].cmd, jobs[i].pid);
            for (int j = i; j < job_count - 1; j++)
                jobs[j] = jobs[j + 1];
            job_count--;
        } else {
            i++;
        }
    }
}

void jobs_print(void) {
    for (int i = 0; i < job_count; i++)
        printf("[%d] %s (pid %d)\n", i + 1, jobs[i].cmd, jobs[i].pid);
}

/* ---------------- EXECUTION ---------------- */

int execute_pipeline(pipeline_t *pl, int background, const char *cmdline) {
    if (!pl || pl->cmd_count == 0) return -1;

    /* Handle built-ins: assignment, unset, jobs */
    if (pl->commands[0].argv[0] == NULL) return -1;
    char *cmd = pl->commands[0].argv[0];

    /* VARIABLE ASSIGNMENT: NAME=value */
    char *eq = strchr(cmd, '=');
    if (eq && eq != cmd) {
        *eq = '\0';
        var_set(cmd, eq + 1);
        return 0;
    }

    /* UNSET built-in */
    if (strcmp(cmd, "unset") == 0 && pl->commands[0].argv[1]) {
        var_unset(pl->commands[0].argv[1]);
        return 0;
    }

    /* JOBS built-in */
    if (strcmp(cmd, "jobs") == 0) {
        jobs_print();
        return 0;
    }

    /* Expand variables in arguments */
    for (int i = 0; pl->commands[0].argv[i]; i++) {
        char *arg = pl->commands[0].argv[i];
        if (strchr(arg, '$')) {
            pl->commands[0].argv[i] = strdup(expand_vars(arg));
        }
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        execvp(pl->commands[0].argv[0], pl->commands[0].argv);
        perror("execvp");
        exit(1);
    }

    if (background) {
        jobs_add(pid, cmdline);
        printf("[bg pid %d] %s\n", pid, cmdline);
        return 0;
    } else {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}
