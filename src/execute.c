#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"

/* Execute a command given argv (null-terminated)
   Returns exit status (0 on success) or -1 on fork/exec errors.
*/
int execute_command(char **argv) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        /* child */
        execvp(argv[0], argv);
        /* execvp only returns on error */
        perror("execvp");
        _exit(127);
    } else {
        /* parent: wait for child */
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return -1;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
}
