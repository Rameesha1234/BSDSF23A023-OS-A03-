#include "shell.h"

/* ---------- Trim whitespace ---------- */
char *trim_whitespace(char *s) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

/* ---------- Tokenize input ---------- */
char **tokenize(const char *line) {
    if (!line) return NULL;
    char *copy = strdup(line);
    if (!copy) return NULL;

    int cap = 8;
    char **tokens = malloc(cap * sizeof(char *));
    int count = 0;
    char *tok, *saveptr = NULL;

    tok = strtok_r(copy, " \t", &saveptr);
    while (tok) {
        if (count + 1 >= cap) {
            cap *= 2;
            tokens = realloc(tokens, cap * sizeof(char *));
        }
        tokens[count++] = strdup(tok);
        tok = strtok_r(NULL, " \t", &saveptr);
    }
    tokens[count] = NULL;
    free(copy);
    return tokens;
}

void free_tokens(char **tokens) {
    if (!tokens) return;
    for (int i = 0; tokens[i]; i++)
        free(tokens[i]);
    free(tokens);
}

/* ---------- Parse pipeline ---------- */
pipeline_t parse_pipeline(char **tokens) {
    pipeline_t pl;
    pl.count = 0;
    pl.commands = NULL;

    if (!tokens) return pl;

    int cap = 2;
    pl.commands = malloc(cap * sizeof(command_t));

    command_t current = {0};
    int argc = 0, argv_cap = 8;
    current.argv = malloc(argv_cap * sizeof(char *));
    current.infile = NULL;
    current.outfile = NULL;

    for (int i = 0; tokens[i]; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            current.argv[argc] = NULL;
            pl.commands[pl.count++] = current;
            if (pl.count >= cap) {
                cap *= 2;
                pl.commands = realloc(pl.commands, cap * sizeof(command_t));
            }
            current.argv = malloc(argv_cap * sizeof(char *));
            current.infile = NULL;
            current.outfile = NULL;
            argc = 0;
        } else if (strcmp(tokens[i], "<") == 0 && tokens[i + 1]) {
            current.infile = strdup(tokens[++i]);
        } else if (strcmp(tokens[i], ">") == 0 && tokens[i + 1]) {
            current.outfile = strdup(tokens[++i]);
        } else {
            if (argc + 1 >= argv_cap) {
                argv_cap *= 2;
                current.argv = realloc(current.argv, argv_cap * sizeof(char *));
            }
            current.argv[argc++] = strdup(tokens[i]);
        }
    }
    current.argv[argc] = NULL;
    pl.commands[pl.count++] = current;
    return pl;
}

void free_pipeline(pipeline_t *pl) {
    for (int i = 0; i < pl->count; i++) {
        for (int j = 0; pl->commands[i].argv[j]; j++)
            free(pl->commands[i].argv[j]);
        free(pl->commands[i].argv);
        free(pl->commands[i].infile);
        free(pl->commands[i].outfile);
    }
    free(pl->commands);
}
