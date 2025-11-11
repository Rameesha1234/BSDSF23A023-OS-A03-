#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>

typedef struct {
    char **argv;
} command_t;

typedef struct {
    command_t *commands;
    int cmd_count;
} pipeline_t;

/* Core parsing and execution */
char **tokenize(const char *line);
void free_tokens(char **tokens);
pipeline_t *parse_pipeline(char **tokens);
void free_pipeline(pipeline_t *pl);
int execute_pipeline(pipeline_t *pl, int background, const char *cmdline);

/* Utility */
char *trim_whitespace(char *str);

/* Jobs management */
void jobs_add(pid_t pid, const char *cmd);
void jobs_reap(void);
void jobs_print(void);

#endif
