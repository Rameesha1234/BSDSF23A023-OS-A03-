#ifndef SHELL_H
#define SHELL_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>

/* trim / parsing */
char *trim_whitespace(char *str);
char **parse_command(const char *line, int *argc);
void free_args(char **args);

/* executor */
int execute_command(char **argv);

/* PATH command list for completion */
int build_command_list(void);
void free_command_list(void);
char *command_generator(const char *text, int state);

/* readline completion callback */
char **myshell_completion(const char *text, int start, int end);

#endif
