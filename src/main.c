#include "shell.h"

int main() {
    char *cmdline;
    char **arglist;

    /* history data */
    history_entry history[HISTORY_SIZE] = {0};
    long hist_count = 0;
    long hist_next_idx = 0;
    long global_seq = 1;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        
        /* handle !n BEFORE adding to history */
        if (cmdline[0] == '!') {
            long n = atol(cmdline + 1);

            long hist_base_seq = (hist_count < HISTORY_SIZE)
                                 ? 1
                                 : (global_seq - HISTORY_SIZE);

            char *retrieved = get_history_command(history, hist_count, hist_base_seq, n);
            if (!retrieved) {
                printf("No such command in history.\n");
                free(cmdline);
                continue;
            }

            free(cmdline);
            cmdline = retrieved;
            printf("%s\n", cmdline);
        }

        /* add to history after full command is finalized */
        add_history(history, &hist_count, &hist_next_idx, cmdline, &global_seq);

        arglist = tokenize(cmdline);
        if (arglist != NULL) {

            long hist_base_seq = (hist_count < HISTORY_SIZE)
                                 ? 1
                                 : (global_seq - HISTORY_SIZE);

            if (!handle_builtin(arglist, history, hist_count, hist_base_seq)) {
                execute(arglist);
            }

            for (int j = 0; arglist[j] != NULL; j++)
                free(arglist[j]);
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nExiting shell...\n");
    return 0;
}
