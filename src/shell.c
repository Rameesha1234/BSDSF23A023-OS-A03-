#include "shell.h"

// ---------------------------------------------
// Handle Built-in Commands
// ---------------------------------------------
int handle_builtin(char **arglist) {
    if (arglist[0] == NULL)
        return 1;

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else if (chdir(arglist[1]) != 0) {
            perror("cd error");
        }
        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("Available built-in commands:\n");
        printf("  help  - Display this help message\n");
        printf("  cd <directory> - Change current working directory\n");
        printf("  exit  - Exit the shell\n");
        printf("  jobs  - Placeholder (not implemented yet)\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0;
}

// ---------------------------------------------
// Tokenize Input String (space-separated)
// ---------------------------------------------
char** tokenize(char* cmdline) {
    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    int argnum = 0;
    char* token = strtok(cmdline, " \t");

    while (token != NULL && argnum < MAXARGS) {
        arglist[argnum] = strdup(token);
        token = strtok(NULL, " \t");
        argnum++;
    }

    arglist[argnum] = NULL;
    return arglist;
}

// ---------------------------------------------
// Read Command from User
// ---------------------------------------------
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    fflush(stdout);

    char* cmdline = malloc(MAX_LEN);
    if (fgets(cmdline, MAX_LEN, fp) == NULL)
        return NULL;

    // remove newline
    size_t len = strlen(cmdline);
    if (len > 0 && cmdline[len - 1] == '\n')
        cmdline[len - 1] = '\0';

    return cmdline;
}
