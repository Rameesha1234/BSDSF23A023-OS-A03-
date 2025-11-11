#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>
#include <sys/types.h>

/* One command in a pipeline */
typedef struct {
    char **argv;    /* NULL-terminated argv */
    char *infile;   /* input redirection filename or NULL */
    char *outfile;  /* output redirection filename or NULL */
} command_t;

/* A pipeline is an array of commands joined by '|' */
typedef struct {
    command_t *commands; /* dynamic array */
    int count;           /* number of commands */
} pipeline_t;

/* Parsing & helpers (main.c / shell.c) */
char *trim_whitespace(char *s);
char **tokenize(const char *line);      /* returns NULL-terminated token array; free with free_tokens */
void free_tokens(char **tokens);
pipeline_t *parse_pipeline(char **tokens);
void free_pipeline(pipeline_t *pl);

/* Execution & jobs (execute.c) */
/* background: 0 = foreground, 1 = background
   cmdline is used for job list display (copied by caller if needed)
*/
void execute_pipeline(pipeline_t *pl, int background, const char *cmdline);

/* Job control helpers (execute.c) */
void jobs_print(void);       /* builtin: print active background jobs */
void jobs_reap(void);        /* reap finished background jobs (WNOHANG) */

#endif /* SHELL_H */
