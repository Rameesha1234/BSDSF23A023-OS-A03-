#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "shell.h"

/* A dynamically built list of available commands (from PATH).
   We'll build this list on startup and reuse it for completion.
*/
static char **command_list = NULL;
static size_t command_count = 0;

/* Trim leading and trailing whitespace in-place. Returns same pointer.
*/
char *trim_whitespace(char *str) {
    char *end;
    if (!str) return NULL;

    /* leading */
    while (isspace((unsigned char)*str)) str++;

    if (str == 0)  / all spaces */
        return str;

    /* trailing */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

/* Parse command line into argv-style array. Caller must free with free_args. */
char **parse_command(const char *line, int *argc_out) {
    if (!line) return NULL;
    char *copy = strdup(line);
    if (!copy) return NULL;

    size_t capacity = 8;
    char *argv = malloc(capacity * sizeof(char));
    if (!argv) { free(copy); return NULL; }

    int argc = 0;
    char *tok = NULL;
    char *saveptr = NULL;
    tok = strtok_r(copy, " \t", &saveptr);
    while (tok) {
        if (argc + 1 >= (int)capacity) {
            capacity *= 2;
            char *tmp = realloc(argv, capacity * sizeof(char));
            if (!tmp) {
                free_args(argv);
                free(copy);
                return NULL;
            }
            argv = tmp;
        }
        argv[argc++] = strdup(tok);
        tok = strtok_r(NULL, " \t", &saveptr);
    }
    argv[argc] = NULL;
    if (argc_out) *argc_out = argc;
    free(copy);
    return argv;
}

/* Free argv array created by parse_command */
void free_args(char **args) {
    if (!args) return;
    for (size_t i = 0; args[i] != NULL; ++i) {
        free(args[i]);
    }
    free(args);
}

/* Helper: check if a file is executable regular file */
static int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    if (access(path, X_OK) != 0) return 0;
    return 1;
}

/* Build command_list by scanning each directory in PATH, adding unique executable names. */
int build_command_list(void) {
    char *path_env = getenv("PATH");
    if (!path_env) return -1;

    char *pathdup = strdup(path_env);
    if (!pathdup) return -1;

    /* temp hash via simple linear scan to avoid duplicates */
    char **tmp = NULL;
    size_t tmpcap = 0, tmpcount = 0;

    char *dir = NULL;
    char *saveptr = NULL;
    dir = strtok_r(pathdup, ":", &saveptr);
    while (dir) {
        DIR *d = opendir(dir);
        if (d) {
            struct dirent *entry;
            while ((entry = readdir(d)) != NULL) {
                if (entry->d_name[0] == '.') continue;
                /* build full path and check executable */
                char full[PATH_MAX];
                int n = snprintf(full, sizeof(full), "%s/%s", dir, entry->d_name);
                if (n < 0 || n >= (int)sizeof(full)) continue;
                if (!is_executable(full)) continue;

                /* unique check */
                int found = 0;
                for (size_t i = 0; i < tmpcount; ++i) {
                    if (strcmp(tmp[i], entry->d_name) == 0) { found = 1; break; }
                }
                if (found) continue;

                /* append */
                if (tmpcount + 1 >= tmpcap) {
                    tmpcap = tmpcap ? tmpcap * 2 : 128;
                    char *t2 = realloc(tmp, tmpcap * sizeof(char));
                    if (!t2) { closedir(d); free(pathdup); /* cleanup */ goto err; }
                    tmp = t2;
                }
                tmp[tmpcount++] = strdup(entry->d_name);
            }
            closedir(d);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(pathdup);

    /* finalize into global command_list */
    command_list = tmp;
    command_count = tmpcount;
    return 0;

err:
    if (tmp) {
        for (size_t i = 0; i < tmpcount; ++i) free(tmp[i]);
        free(tmp);
    }
    return -1;
}

/* Free command list */
void free_command_list(void) {
    if (!command_list) return;
    for (size_t i = 0; i < command_count; ++i) {
        free(command_list[i]);
    }
    free(command_list);
    command_list = NULL;
    command_count = 0;
}

/* Generator used by readline to complete commands */
char *command_generator(const char *text, int state) {
    static size_t idx;
    static size_t len;
    if (state == 0) {
        idx = 0;
        len = strlen(text);
    }

    while (idx < command_count) {
        char *cand = command_list[idx++];
        if (strncmp(cand, text, len) == 0) {
            return strdup(cand);
        }
    }
    return NULL;
}

/* The completion callback set to rl_attempted_completion_function */
char **myshell_completion(const char *text, int start, int end) {
    (void)end;  // silence unused parameter warning

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    } else {
        return rl_completion_matches(text, rl_filename_completion_function);
    }


}
