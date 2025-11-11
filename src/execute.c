#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"

#define MAX_JOBS 64

static struct {
    pid_t pid;
    char cmd[256];
} jobs[MAX_JOBS];

static int job_count = 0;

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
    for (int i = 0; i < job_count; ) {
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

int execute_pipeline(pipeline_t *pl, int background, const char *cmdline) {
    if (!pl || pl->cmd_count == 0) return -1;

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
