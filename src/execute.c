#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

int execute_command(char **argv) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }
    else if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    }
    else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return -1;
        }
        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        return -1;
    }
}
