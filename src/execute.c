#include "shell.h"

/* ---------- Execute a pipeline ---------- */
void execute_pipeline(pipeline_t *pl) {
    if (pl->count == 0) return;

    int num = pl->count;
    int pipefd[2 * (num - 1)];
    pid_t pids[num];

    for (int i = 0; i < num - 1; i++) {
        if (pipe(pipefd + 2 * i) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < num; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            if (i > 0)
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            if (i < num - 1)
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);

            for (int j = 0; j < 2 * (num - 1); j++)
                close(pipefd[j]);

            if (pl->commands[i].infile) {
                int fd = open(pl->commands[i].infile, O_RDONLY);
                if (fd < 0) { perror("open input"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (pl->commands[i].outfile) {
                int fd = open(pl->commands[i].outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open output"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(pl->commands[i].argv[0], pl->commands[i].argv);
            perror("execvp");
            exit(1);
        } else {
            pids[i] = pid;
        }
    }

    for (int j = 0; j < 2 * (num - 1); j++)
        close(pipefd[j]);

    for (int i = 0; i < num; i++)
        waitpid(pids[i], NULL, 0);
}
