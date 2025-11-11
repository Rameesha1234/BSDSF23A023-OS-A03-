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

/* --- Helper: expand ~ to home for history path --- */
static char *get_history_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    return path;
}

/* --- Built-in help command --- */
static void show_help(void) {
    printf("\nMini Shell - Built-in Commands:\n");
    printf("  help   - show this help message\n");
    printf("  jobs   - list background jobs\n");
    printf("  set    - assign a shell variable (e.g., NAME=value)\n");
    printf("  unset  - remove a variable (e.g., unset NAME)\n");
    printf("  exit   - exit the shell\n");
    printf("\nSupports: I/O redirection (<, >), pipes (|), chaining (;),\n");
    printf("background jobs (&), conditionals (if/then/else/fi), variables ($VAR),\n");
    printf("and persistent history stored in ~/.myshell_history.\n\n");
}

int main(void) {
    using_history();

    /* Load command history */
    const char *hist_path = get_history_path();
    read_history(hist_path);

    char *line = NULL;

    while (1) {
        jobs_reap(); // reap finished background jobs
        line = readline(PROMPT);
        if (!line) {   /* EOF (Ctrl+D) */
            printf("\n");
            break;
        }

        char *trimmed = trim_whitespace(line);
        if (!trimmed || *trimmed == '\0') {
            free(line);
            continue;
        }

        /* Save to readline history */
        add_history(trimmed);

        /* Built-in 'exit' */
        if (strcmp(trimmed, "exit") == 0) {
            free(line);
            break;
        }

        /* Built-in 'help' */
        if (strcmp(trimmed, "help") == 0) {
            show_help();
            free(line);
            continue;
        }

        /* Built-in 'jobs' */
        if (strcmp(trimmed, "jobs") == 0) {
            jobs_print();
            free(line);
            continue;
        }

        /* Tokenize & execute */
        char **tokens = tokenize(trimmed);
        if (!tokens) {
            free(line);
            continue;
        }

        pipeline_t *pl = parse_pipeline(tokens);
        free_tokens(tokens);
        if (!pl) {
            free(line);
            continue;
        }

        execute_pipeline(pl, 0, trimmed);
        free_pipeline(pl);
        free(line);
    }

    /* Save command history on exit */
    write_history(hist_path);
    printf("History saved to %s\n", hist_path);

    return 0;
}
