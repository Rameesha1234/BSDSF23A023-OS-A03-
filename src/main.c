#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "shell.h"

#define PROMPT "myshell> "
#define MAX_LINE 1024

/* Execute multiple lines (for then/else blocks) */
static void execute_block(char **lines, int count) {
    for (int i = 0; i < count; i++) {
        if (!lines[i] || strlen(lines[i]) == 0) continue;
        char **tokens = tokenize(lines[i]);
        if (!tokens) continue;
        pipeline_t *pl = parse_pipeline(tokens);
        free_tokens(tokens);
        if (!pl) continue;
        execute_pipeline(pl, 0, lines[i]);
        free_pipeline(pl);
    }
}

/* Read if–then–else–fi block */
static int read_if_block(char *first_line, char **cond_cmd,
                         char ***then_lines, int *then_count,
                         char ***else_lines, int *else_count) {

    *cond_cmd = strdup(first_line + 3); // skip "if "
    if (!*cond_cmd) return -1;

    *then_lines = malloc(64 * sizeof(char *));
    *else_lines = malloc(64 * sizeof(char *));
    if (!*then_lines || !*else_lines) return -1;
    *then_count = *else_count = 0;

    int in_else = 0;
    char *line = NULL;

    while ((line = readline("> ")) != NULL) {
        char *trim = trim_whitespace(line);
        if (!trim || *trim == '\0') { free(line); continue; }

        if (strcmp(trim, "then") == 0) { free(line); continue; }
        else if (strcmp(trim, "else") == 0) { in_else = 1; free(line); continue; }
        else if (strcmp(trim, "fi") == 0) { free(line); break; }

        if (!in_else)
            (*then_lines)[(*then_count)++] = strdup(trim);
        else
            (*else_lines)[(*else_count)++] = strdup(trim);

        free(line);
    }

    return 0;
}

int main(void) {
    using_history();
    char *line = NULL;

    while (1) {
        jobs_reap();
        line = readline(PROMPT);
        if (!line) { printf("\n"); break; }

        char *trimmed = trim_whitespace(line);
        if (!trimmed || *trimmed == '\0') { free(line); continue; }

        add_history(trimmed);

        /* IF–THEN–ELSE–FI handling */
        if (strncmp(trimmed, "if ", 3) == 0) {
            char *cond_cmd = NULL;
            char **then_lines = NULL, **else_lines = NULL;
            int then_count = 0, else_count = 0;

            if (read_if_block(trimmed, &cond_cmd, &then_lines, &then_count,
                              &else_lines, &else_count) == 0) {

                char **ctoks = tokenize(cond_cmd);
                if (ctoks) {
                    pipeline_t *cpl = parse_pipeline(ctoks);
                    free_tokens(ctoks);
                    if (cpl) {
                        int status = execute_pipeline(cpl, 0, cond_cmd);
                        free_pipeline(cpl);

                        if (status == 0)
                            execute_block(then_lines, then_count);
                        else
                            execute_block(else_lines, else_count);
                    }
                }
            } else {
                fprintf(stderr, "Malformed if-block\n");
            }

            free(cond_cmd);
            if (then_lines) {
                for (int i = 0; i < then_count; i++) free(then_lines[i]);
                free(then_lines);
            }
            if (else_lines) {
                for (int i = 0; i < else_count; i++) free(else_lines[i]);
                free(else_lines);
            }

            free(line);
            continue;
        }

        /* Normal single command */
        char **tokens = tokenize(trimmed);
        if (tokens) {
            pipeline_t *pl = parse_pipeline(tokens);
            free_tokens(tokens);
            if (pl) {
                execute_pipeline(pl, 0, trimmed);
                free_pipeline(pl);
            }
        }

        free(line);
    }

    return 0;
}
