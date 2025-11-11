#include "shell.h"

int execute(char* arglist[]) {
    if (arglist == NULL || arglist[0] == NULL) return 0;

    int status;
    pid_t cpid;
    int in_fd = -1, out_fd = -1;
    int pipe_pos = -1;

    /* detect redirection and pipe position */
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) {
            if (arglist[i+1]) {
                in_fd = open(arglist[i+1], O_RDONLY);
                if (in_fd < 0) perror("open input");
                arglist[i] = NULL;
            }
        } else if (strcmp(arglist[i], ">") == 0) {
            if (arglist[i+1]) {
                out_fd = open(arglist[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd < 0) perror("open output");
                arglist[i] = NULL;
            }
        } else if (strcmp(arglist[i], "|") == 0) {
            pipe_pos = i;
            arglist[i] = NULL;
        }
    }

    /* handle pipe */
    if (pipe_pos != -1) {
        int pipefd[2];
        if (pipe(pipefd) < 0) { perror("pipe"); return -1; }

        pid_t left = fork();
        if (left < 0) { perror("fork"); return -1; }
        if (left == 0) {
            /* left child writes to pipe */
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            execvp(arglist[0], arglist);
            perror("execvp left");
            exit(1);
        }

        pid_t right = fork();
        if (right < 0) { perror("fork"); return -1; }
        if (right == 0) {
            /* right child reads from pipe */
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            execvp(arglist[pipe_pos + 1], &arglist[pipe_pos + 1]);
            perror("execvp right");
            exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        waitpid(left, &status, 0);
        waitpid(right, &status, 0);
        return 0;
    }

    /* normal single command */
    cpid = fork();
    if (cpid < 0) {
        perror("fork");
        return -1;
    }
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
        perror("execvp");
        exit(1);
    }

    if (in_fd != -1) close(in_fd);
    if (out_fd != -1) close(out_fd);

    waitpid(cpid, &status, 0);
    if (WIFEXITED(status)) {
        printf("child exited with status %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("child terminated by signal %d\n", WTERMSIG(status));
    }
    return 0;
}
