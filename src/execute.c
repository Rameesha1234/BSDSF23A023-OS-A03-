#include "shell.h"

int execute(char* arglist[]) {
    int status;
    int cpid;
    int in_fd = -1, out_fd = -1;
    int pipe_pos = -1;

    // Detect redirection or pipes
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0 && arglist[i + 1] != NULL) {
            in_fd = open(arglist[i + 1], O_RDONLY);
            arglist[i] = NULL;
        } 
        else if (strcmp(arglist[i], ">") == 0 && arglist[i + 1] != NULL) {
            out_fd = open(arglist[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            arglist[i] = NULL;
        } 
        else if (strcmp(arglist[i], "|") == 0) {
            pipe_pos = i;
            arglist[i] = NULL;
        }
    }

    // Handle pipes
    if (pipe_pos != -1) {
        int pipefd[2];
        pipe(pipefd);

        // Left side of pipe
        if ((cpid = fork()) == 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            execvp(arglist[0], arglist);
            perror("pipe left command failed");
            exit(1);
        }

        // Right side
        if (fork() == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            execvp(arglist[pipe_pos + 1], &arglist[pipe_pos + 1]);
            perror("pipe right command failed");
            exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);
        wait(NULL);
        wait(NULL);
        return 0;
    }

    // Normal execution (no pipe)
    cpid = fork();
    if (cpid == 0) {
        if (in_fd != -1) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        execvp(arglist[0], arglist);
        perror("Command not found");
        exit(1);
    }

    if (in_fd != -1) close(in_fd);
    if (out_fd != -1) close(out_fd);

    waitpid(cpid, &status, 0);
    printf("child exited with status %d\n", WEXITSTATUS(status));
    return 0;
}
