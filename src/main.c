#include "shell.h"

int main() {
    char *cmdline;
    char **arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {
            // Handle built-in commands first
            if (!handle_builtin(arglist)) {
                execute(arglist);
            }

            // Free memory safely
            for (int j = 0; arglist[j] != NULL; j++)
                free(arglist[j]);
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nExiting shell...\n");
    return 0;
}
