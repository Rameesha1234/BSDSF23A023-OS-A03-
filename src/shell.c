#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <ctype.h>
#include "shell.h"

/* PROMPT */
#define PROMPT "myshell> "

/* Split input by semicolons into an array of strings.
   Returns dynamically-allocated array (NULL-terminated). Caller free each string and the array.
*/
static char **split_by_semicolon(const char *line) {
    if (!line) return NULL;
    size_t cap = 8, n = 0;
    char **arr = malloc(cap * sizeof(char*));
    if (!arr) return NULL;

    const char *p = line;
    const char *start = p;
    while (*p) {
        if (*p == ';') {
            size_t len = p - start;
            /* allocate and copy trimmed substring */
            char *s = malloc(len + 1);
            if (!s) goto err;
            memcpy(s, start, len);
            s[len] = '\0';
            if (n + 1 >= cap) { cap *= 2; char **t = realloc(arr, cap * sizeof(char*)); if (!t) goto err; arr = t; }
            arr[n++] = s;
            p++; start = p;
            continue;
        }
        p++;
    }
    /* tail */
    if (p != start) {
        size_t len = p - start;
        char *s = malloc(len + 1);
        if (!s) goto err;
        memcpy(s, start, len);
        s[len] = '\0';
        if (n + 1 >= cap) { cap *= 2; char **t = realloc(arr, cap * sizeof(char*)); if (!t) goto err; arr = t; }
        arr[n++] = s;
    }

    arr[n] = NULL;
    return arr;

err:
    for (size_t i = 0; i < n; ++i) free(arr[i]);
    free(arr);
    return NULL;
}

/* free split results */
static void free_split(char **arr) {
    if (!arr) return;
    for (size_t i = 0; arr[i] != NULL; ++i) free(arr[i]);
    free(arr);
}

int main(void) {
    char *line = NULL;

    while (1) {
        /* reap any finished background jobs before showing prompt */
        jobs_reap();

        line = readline(PROMPT);
        if (!line) { /* EOF */
            printf("\n");
            break;
        }

        char *trimmed_line = trim_whitespace(line);
        if (!trimmed_line || *trimmed_line == '\0') {
            free(line);
            continue;
        }

        /* Add to readline history (only non-empty commands) */
        add_history(trimmed_line);

        /* Split by semicolon for command chaining */
        char **parts = split_by_semicolon(trimmed_line);
        if (!parts) { free(line); continue; }

        for (size_t i = 0; parts[i] != NULL; ++i) {
            char *cmdstr = trim_whitespace(parts[i]);
            if (!cmdstr || *cmdstr == '\0') continue;

            /* detect background '&' at end */
            int background = 0;
            size_t L = strlen(cmdstr);
            /* skip trailing whitespace */
            while (L > 0 && isspace((unsigned char)cmdstr[L-1])) { cmdstr[L-1] = '\0'; --L; }
            if (L > 0 && cmdstr[L-1] == '&') {
                background = 1;
                cmdstr[L-1] = '\0';
                /* trim trailing spaces again */
                while (L > 0 && isspace((unsigned char)cmdstr[L-1])) { cmdstr[L-1] = '\0'; --L; }
            }

            if (*cmdstr == '\0') continue;

            /* built-in 'jobs' */
            if (!background && strcmp(cmdstr, "jobs") == 0) {
                jobs_print();
                continue;
            }

            /* tokenize and parse pipeline then execute */
            char **tokens = tokenize(cmdstr);
            if (!tokens) continue;
            pipeline_t *pl = parse_pipeline(tokens);
            free_tokens(tokens);
            if (!pl) continue;

            execute_pipeline(pl, background, cmdstr);

            free_pipeline(pl);
        }

        free_split(parts);
        free(line);
    }

    return 0;
}
