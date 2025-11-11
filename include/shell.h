#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXARGS 20
#define ARGLEN 100
#define MAX_LEN 1024
#define HISTORY_SIZE 50
#define PROMPT "myshell> "

// Structure for a single command in history
typedef struct {
    char *cmd;   /* stored command string */
    long seq;    /* sequence number */
} history_entry;

/* Function declarations — updated for Feature 3 */
char* read_cmd(const char* prompt, FILE* fp);

void free_history_slot(history_entry* slot);
void add_history(history_entry history[],
                 long* hist_count,
                 long* hist_next_idx,
                 char* cmdline,
                 long* global_seq);

char* get_history_command(history_entry history[],
                          long hist_count,
                          long hist_base_seq,
                          long n);

int handle_builtin(char** arglist,
                   history_entry history[],
                   long hist_count,
                   long hist_base_seq);

char** tokenize(char* cmdline);
int execute(char* arglist[]);

#endif
