#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"

#define PROMPT "myshell> "

int main(void) {
    char *line;
    int build_ok;

    /* Build command list from PATH for tab completion */
    build_ok = build_command_list();
    if (build_ok != 0) {
        fprintf(stderr, "Warning: failed to build command list for completion\n");
    }

    /* Set tab-completion handler */
    rl_attempted_completion_function = myshell_completion;

    while (1) {
        line = readline(PROMPT);
        if (!line) {
            printf("\n");
            break;
        }

        char *trimmed = trim_whitespace(line);

        if (trimmed && *trimmed != '\0') {
            add_history(trimmed);

            int argc = 0;
            char **argv = parse_command(trimmed, &argc);

            if (argv && argc > 0) {
                if (strcmp(argv[0], "exit") == 0) {
                    free_args(argv);
                    free(line);
                    break;
                }

                execute_command(argv);
                free_args(argv);
            }
        }

        free(line);
    }

    free_command_list();
    return 0;
}
