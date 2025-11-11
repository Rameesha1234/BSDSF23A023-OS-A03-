#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"

/* prompt */
#define PROMPT "myshell> "

int main(void) {
    char *line;
    int build_ok;

    /* build a command list from PATH so we can complete commands */
    build_ok = build_command_list();
    if (build_ok != 0) {
        fprintf(stderr, "Warning: failed to build command list for completion\n");
        /* not fatal; filename completion will still work */
    }

    /* hook our completion function */
    rl_attempted_completion_function = myshell_completion;

    while (1) {
        line = readline(PROMPT);
        if (!line) { /* EOF, e.g., Ctrl-D */
            printf("\n");
            break;
        }

        /* trim leading/trailing whitespace */
        char *trimmed = trim_whitespace(line);
        if (trimmed && *trimmed != '\0') {
            add_history(trimmed); /* only add non-empty commands */
            int argc;
            char **argv = parse_command(trimmed, &argc);
            if (argv != NULL && argc > 0) {
                /* built-in exit */
                if (strcmp(argv[0], "exit") == 0) {
                    free_args(argv);
                    free(line);
                    break;
                }
                /* execute external/builtin commands via execute_command */
                execute_command(argv);
                free_args(argv);
            }
        }

        free(line);
    }

    free_command_list();
    return 0;
}
