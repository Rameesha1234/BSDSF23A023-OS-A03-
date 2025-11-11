#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>

/* Command structure */
typedef struct {
    char **argv;       /* NULL-terminated argument vector */
    char *infile;      /* input redirection filename, or NULL */
    char *outfile;     /* output redirection filename, or NULL */
} command_t;

/* Pipeline of commands */
typedef struct {
    command_t *commands; /* dynamically allocated array */
    int cmd_count;       /* number of commands */
} pipeline_t;

/* --- Shell parsing & helpers --- */
char *trim_whitespace(char *s);
char **tokenize(const char *line);         /* returns NULL-terminated token array; caller frees with free_tokens */
void free_tokens(char **tokens);

pipeline_t *parse_pipeline(char **tokens); /* build pipeline from tokens; caller must free */
void free_pipeline(pipeline_t *pl);

/* --- Execution --- */
int execute_pipeline(pipeline_t *pl, int background, const char *cmdline);

/* --- Jobs --- */
void jobs_add(pid_t pid, const char *cmd);
void jobs_reap(void);
void jobs_print(void);

#endif /* SHELL_H */
