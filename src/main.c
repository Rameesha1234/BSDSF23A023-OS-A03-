#include "shell.h"

#define PROMPT "myshell> "

int main(void) {
    char line[512];

    while (1) {
        printf("%s", PROMPT);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;

        char *trim = trim_whitespace(line);
        if (!trim || *trim == '\0') continue;

        if (strcmp(trim, "exit") == 0)
            break;

        char **tokens = tokenize(trim);
        pipeline_t pl = parse_pipeline(tokens);
        if (pl.count > 0) {
            execute_pipeline(&pl);
            free_pipeline(&pl);
        }
        free_tokens(tokens);
    }

    printf("Exiting myshell...\n");
    return 0;
}
