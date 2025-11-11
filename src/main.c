#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"

/* Trim leading/trailing whitespace in-place and return pointer within string */
char *trim_whitespace(char *s) {
    if (!s) return NULL;
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0') {
        s[0] = '\0';
        return s;
    }
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    /* If start != s, move the trimmed string to the front to make caller's life easier */
    if (start != s) memmove(s, start, strlen(start) + 1);
    return s;
}

/* Tokenizer - returns a NULL-terminated array of strings.
   Special tokens recognized as single tokens: '<', '>', '|'
   Caller must free with free_tokens().
*/
char **tokenize(const char *line) {
    if (!line) return NULL;
    size_t cap = 16, n = 0;
    char **tokens = malloc(cap * sizeof(char*));
    if (!tokens) return NULL;

    const char *p = line;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        if (*p == '<' || *p == '>' || *p == '|') {
            char tmp[2] = {*p, '\0'};
            if (n + 1 >= cap) { cap *= 2; char **t = realloc(tokens, cap * sizeof(char*)); if (!t) goto tok_err; tokens = t; }
            tokens[n++] = strdup(tmp);
            p++;
            continue;
        }

        const char *start = p;
        while (*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>' && *p != '|') p++;
        size_t len = (size_t)(p - start);
        char *tok = malloc(len + 1);
        if (!tok) goto tok_err;
        memcpy(tok, start, len);
        tok[len] = '\0';
        if (n + 1 >= cap) { cap *= 2; char **t = realloc(tokens, cap * sizeof(char*)); if (!t) { free(tok); goto tok_err; } tokens = t; }
        tokens[n++] = tok;
    }

    tokens[n] = NULL;
    return tokens;

tok_err:
    for (size_t i = 0; i < n; ++i) free(tokens[i]);
    free(tokens);
    return NULL;
}

void free_tokens(char **tokens) {
    if (!tokens) return;
    for (size_t i = 0; tokens[i] != NULL; ++i) free(tokens[i]);
    free(tokens);
}

/* Parse tokens into pipeline_t.
   Simple grammar:
     command := WORD*
     pipeline := command ('|' command)*
     redirections: inside a command, '<' filename and '>' filename set infile/outfile
*/
pipeline_t *parse_pipeline(char **tokens) {
    if (!tokens) return NULL;

    pipeline_t *pl = calloc(1, sizeof(pipeline_t));
    if (!pl) return NULL;

    int cmd_cap = 4;
    pl->commands = malloc(cmd_cap * sizeof(command_t));
    pl->count = 0;

    /* current command building */
    char **argv = NULL;
    int argc = 0;
    int argv_cap = 8;
    argv = malloc(argv_cap * sizeof(char*));
    if (!argv) goto parse_err;
    char *infile = NULL;
    char *outfile = NULL;

    for (size_t i = 0; tokens[i] != NULL; ++i) {
        char *tk = tokens[i];

        if (strcmp(tk, "|") == 0) {
            if (argc == 0) goto parse_err; /* empty command */
            argv[argc] = NULL;
            /* append command */
            if (pl->count + 1 > cmd_cap) {
                cmd_cap *= 2;
                command_t *tmp = realloc(pl->commands, cmd_cap * sizeof(command_t));
                if (!tmp) goto parse_err;
                pl->commands = tmp;
            }
            /* shrink argv */
            argv = realloc(argv, (argc + 1) * sizeof(char*));
            pl->commands[pl->count].argv = argv;
            pl->commands[pl->count].infile = infile;
            pl->commands[pl->count].outfile = outfile;
            pl->count++;

            /* new command */
            argv_cap = 8;
            argv = malloc(argv_cap * sizeof(char*));
            if (!argv) goto parse_err;
            argc = 0;
            infile = NULL;
            outfile = NULL;
            continue;
        } else if (strcmp(tk, "<") == 0) {
            /* input redirection */
            if (tokens[i+1] == NULL) goto parse_err;
            free(infile);
            infile = strdup(tokens[++i]);
            continue;
        } else if (strcmp(tk, ">") == 0) {
            if (tokens[i+1] == NULL) goto parse_err;
            free(outfile);
            outfile = strdup(tokens[++i]);
            continue;
        } else {
            /* argument */
            if (argc + 1 >= argv_cap) {
                argv_cap *= 2;
                char **tmp = realloc(argv, argv_cap * sizeof(char*));
                if (!tmp) goto parse_err;
                argv = tmp;
            }
            argv[argc++] = strdup(tk);
            continue;
        }
    }

    /* finish last command */
    if (argc == 0) {
        /* nothing to run; free and return NULL if no commands built */
        for (int j = 0; j < argc; ++j) free(argv[j]);
        free(argv);
        if (pl->count == 0) {
            free(pl->commands);
            free(pl);
            return NULL;
        }
    } else {
        argv[argc] = NULL;
        argv = realloc(argv, (argc + 1) * sizeof(char*));
        pl->commands[pl->count].argv = argv;
        pl->commands[pl->count].infile = infile;
        pl->commands[pl->count].outfile = outfile;
        pl->count++;
    }

    return pl;

parse_err:
    fprintf(stderr, "Syntax error in command\n");
    /* cleanup partial allocations */
    if (argv) {
        for (int j = 0; j < argc; ++j) free(argv[j]);
        free(argv);
    }
    for (int c = 0; c < pl->count; ++c) {
        command_t *cc = &pl->commands[c];
        if (cc->argv) {
            for (int a = 0; cc->argv[a] != NULL; ++a) free(cc->argv[a]);
            free(cc->argv);
        }
        if (cc->infile) free(cc->infile);
        if (cc->outfile) free(cc->outfile);
    }
    free(pl->commands);
    free(pl);
    return NULL;
}

void free_pipeline(pipeline_t *pl) {
    if (!pl) return;
    for (int i = 0; i < pl->count; ++i) {
        command_t *c = &pl->commands[i];
        if (c->argv) {
            for (int j = 0; c->argv[j] != NULL; ++j) free(c->argv[j]);
            free(c->argv);
        }
        if (c->infile) free(c->infile);
        if (c->outfile) free(c->outfile);
    }
    free(pl->commands);
    free(pl);
}
