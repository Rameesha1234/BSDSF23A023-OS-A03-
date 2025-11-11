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



/* public function prototypes */
char *trim_whitespace(char *str);
char **parse_command(const char *line, int *argc);
int execute_command(char **argv);
void free_args(char **args);
int build_command_list(void);
void free_command_list(void);
char **myshell_completion(const char *text, int start, int _attribute_((unused)) end);

/* completion hooks used by readline */
char **myshell_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);

#endif /* SHELL_H */
