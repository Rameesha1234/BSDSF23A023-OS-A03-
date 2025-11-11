#include "shell.h"

char* read_cmd(const char* prompt, FILE* fp) {
    printf("%s", prompt);
    fflush(stdout);

    char buffer[MAX_LEN];
    if (!fgets(buffer, MAX_LEN, fp))
        return NULL;

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';

    return strdup(buffer);
}

char** tokenize(char* cmdline) {
    if (!cmdline) return NULL;
    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    int argnum = 0;
    char* token = strtok(cmdline, " \t");

    while (token && argnum < MAXARGS) {
        arglist[argnum] = strdup(token);
        argnum++;
        token = strtok(NULL, " \t");
    }

    arglist[argnum] = NULL;
    return arglist;
}

void free_history_slot(history_entry *slot) {
    if (slot->cmd) {
        free(slot->cmd);
        slot->cmd = NULL;
    }
    slot->seq = 0;
}

void add_history(history_entry history[], long *hist_count, long *hist_next_idx, char *cmdline, long *global_seq) {
    if (!cmdline || strlen(cmdline) == 0) return;

    char *copy = strdup(cmdline);
    if (!copy) return;

    free_history_slot(&history[*hist_next_idx]);

    history[*hist_next_idx].cmd = copy;
    history[*hist_next_idx].seq = *global_seq;
    (*global_seq)++;

    *hist_next_idx = (*hist_next_idx + 1) % HISTORY_SIZE;

    if (*hist_count < HISTORY_SIZE)
        (*hist_count)++;
}

char* get_history_command(history_entry history[], long hist_count, long hist_base_seq, long n) {
    if (hist_count == 0) return NULL;

    long oldest_seq = hist_base_seq;
    long newest_seq = oldest_seq + hist_count - 1;

    if (n < oldest_seq || n > newest_seq)
        return NULL;

    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i].seq == n)
            return strdup(history[i].cmd);
    }
    return NULL;
}

int handle_builtin(char **arglist, history_entry history[], long hist_count, long hist_base_seq) {
    if (!arglist || !arglist[0])
        return 1;

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        if (!arglist[1]) {
            chdir(getenv("HOME"));
        } else if (chdir(arglist[1]) != 0) {
            perror("cd error");
        }
        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Commands: help, cd, exit, jobs, history, !n\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    if (strcmp(arglist[0], "history") == 0) {
        for (long seq = hist_base_seq; seq < hist_base_seq + hist_count; seq++) {
            for (int i = 0; i < HISTORY_SIZE; i++) {
                if (history[i].seq == seq) {
                    printf("%4ld  %s\n", history[i].seq, history[i].cmd);
                    break;
                }
            }
        }
        return 1;
    }

    return 0;
}
