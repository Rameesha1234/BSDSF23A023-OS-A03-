#ifndef SHELL_H
#define SHELL_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

/* ---------- Command structure for pipelines ---------- */
typedef struct {
    char **argv;      /* NULL-terminated argument list */
    char *infile;     /* Input redirection file (if any) */
    char *outfile;    /* Output redirection file (if any) */
} command_t;

typedef struct {
    command_t *commands;  /* Dynamic array of commands */
    int count;            /* Number of commands */
} pipeline_t;

/* ---------- Function declarations ---------- */

/* String helpers */
char *trim_whitespace(char *s);

/* Tokenizer */
char **tokenize(const char *line);
void free_tokens(char **tokens);

/* Pipeline parser */
pipeline_t parse_pipeline(char **tokens);
void free_pipeline(pipeline_t *pl);

/* Executor */
void execute_pipeline(pipeline_t *pl);

#endif /* SHELL_H */
