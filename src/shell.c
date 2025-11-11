#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"

char *trim_whitespace(char *str) {
    if (!str) return NULL;
    while (isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

char **tokenize(const char *line) {
    if (!line) return NULL;

    size_t cap = 8, n = 0;
    char **tokens = malloc(cap * sizeof(char *));
    if (!tokens) return NULL;

    char *copy = strdup(line);
    if (!copy) {
        free(tokens);
        return NULL;
    }

    char *tok = strtok(copy, " \t");
    while (tok) {
        if (n + 1 >= cap) {
            cap *= 2;
            tokens = realloc(tokens, cap * sizeof(char *));
        }
        tokens[n++] = strdup(tok);
        tok = strtok(NULL, " \t");
    }
    tokens[n] = NULL;

    free(copy);
    return tokens;
}

void free_tokens(char **tokens) {
    if (!tokens) return;
    for (int i = 0; tokens[i]; i++) free(tokens[i]);
    free(tokens);
}

pipeline_t *parse_pipeline(char **tokens) {
    if (!tokens || !tokens[0]) return NULL;

    pipeline_t *pl = malloc(sizeof(pipeline_t));
    if (!pl) return NULL;

    pl->commands = malloc(sizeof(command_t));
    pl->cmd_count = 1;

    int argc = 0;
    while (tokens[argc]) argc++;

    pl->commands[0].argv = malloc((argc + 1) * sizeof(char *));
    for (int i = 0; i < argc; i++) {
        pl->commands[0].argv[i] = strdup(tokens[i]);
    }
    pl->commands[0].argv[argc] = NULL;

    pl->commands[0].infile = NULL;
    pl->commands[0].outfile = NULL;

    return pl;
}

void free_pipeline(pipeline_t *pl) {
    if (!pl) return;
    for (int i = 0; i < pl->cmd_count; i++) {
        for (int j = 0; pl->commands[i].argv[j]; j++)
            free(pl->commands[i].argv[j]);
        free(pl->commands[i].argv);
        free(pl->commands[i].infile);
        free(pl->commands[i].outfile);
    }
    free(pl->commands);
    free(pl);
}
