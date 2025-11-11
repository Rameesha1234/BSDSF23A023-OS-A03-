#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"

#define PROMPT "myshell> "
#define HISTORY_FILE ".myshell_history"

/* --- Helper: expand ~ to home --- */
static char *get_history_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    return path;
}

int main(void) {
    using_history();

    /* Load history from file */
    const char *hist_path = get_history_path();
    read_history(hist_path);

    char *line = NULL;

    while (1) {
        jobs_reap(); // handle background jobs
        line = readline(PROMPT);
        if (!line) { printf("\n"); break; }

        char *trimmed = trim_whitespace(line);
        if (!trimmed || *trimmed == '\0') {
            free(line);
            continue;
        }

        /* Save non-empty commands to history */
        add_history(trimmed);

        /* Built-in exit */
        if (strcmp(trimmed, "exit") == 0)
            break;

        /* Tokenize and execute */
        char **tokens = tokenize(trimmed);
        if (!tokens) { free(line); continue; }

        pipeline_t *pl = parse_pipeline(tokens);
        free_tokens(tokens);
        if (!pl) { free(line); continue; }

        execute_pipeline(pl, 0, trimmed);
        free_pipeline(pl);
        free(line);
    }

    /* Save history before exiting */
    write_history(hist_path);
    printf("History saved to %s\n", hist_path);

    return 0;
}

