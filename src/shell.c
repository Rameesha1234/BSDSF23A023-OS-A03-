#include "shell.h"

/* Trim leading/trailing whitespace */
char *trim_whitespace(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

/* Parse command into argv[] */
char **parse_command(const char *line, int *argc_out) {
    if (!line) return NULL;

    char *copy = strdup(line);
    if (!copy) return NULL;

    size_t cap = 8;
    char **argv = malloc(cap * sizeof(char *));
    if (!argv) {
        free(copy);
        return NULL;
    }

    int argc = 0;
    char *tok, *saveptr = NULL;

    tok = strtok_r(copy, " \t", &saveptr);
    while (tok) {
        if (argc + 1 >= (int)cap) {
            cap *= 2;
            char **tmp = realloc(argv, cap * sizeof(char *));
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

void free_args(char **args) {
    if (!args) return;
    for (int i = 0; args[i]; i++)
        free(args[i]);
    free(args);
}

/***********************
 * PATH command list
 ***********************/
static char **command_list = NULL;
static size_t command_count = 0;

/* Helper: check if file is executable */
static int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    return (access(path, X_OK) == 0);
}

/* Build list of executables from PATH */
int build_command_list(void) {
    char *path = getenv("PATH");
    if (!path) return -1;

    char *dup = strdup(path);
    if (!dup) return -1;

    char **tmp = NULL;
    size_t cap = 0, count = 0;

    char *dir, *saveptr = NULL;
    dir = strtok_r(dup, ":", &saveptr);

    while (dir) {
        DIR *d = opendir(dir);
        if (d) {
            struct dirent *e;
            while ((e = readdir(d)) != NULL) {
                if (e->d_name[0] == '.') continue;

                char full[PATH_MAX];
                snprintf(full, sizeof(full), "%s/%s", dir, e->d_name);

                if (!is_executable(full)) continue;

                int found = 0;
                for (size_t i = 0; i < count; i++) {
                    if (strcmp(tmp[i], e->d_name) == 0) {
                        found = 1;
                        break;
                    }
                }
                if (found) continue;

                if (count + 1 >= cap) {
                    cap = cap ? cap * 2 : 128;
                    char **t2 = realloc(tmp, cap * sizeof(char *));
                    if (!t2) {
                        closedir(d);
                        free(dup);
                        goto fail;
                    }
                    tmp = t2;
                }
                tmp[count++] = strdup(e->d_name);
            }
            closedir(d);
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(dup);
    command_list = tmp;
    command_count = count;
    return 0;

fail:
    if (tmp) {
        for (size_t i = 0; i < count; i++) free(tmp[i]);
        free(tmp);
    }
    return -1;
}

void free_command_list(void) {
    if (!command_list) return;
    for (size_t i = 0; i < command_count; i++)
        free(command_list[i]);
    free(command_list);

    command_list = NULL;
    command_count = 0;
}

/***********************
 * TAB COMPLETION
 ***********************/
char *command_generator(const char *text, int state) {
    static size_t idx;
    static size_t len;

    if (state == 0) {
        idx = 0;
        len = strlen(text);
    }

    while (idx < command_count) {
        char *cmd = command_list[idx++];
        if (strncmp(cmd, text, len) == 0)
            return strdup(cmd);
    }
    return NULL;
}

char **myshell_completion(const char *text, int start, int end) {
    (void) end;

    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    } else {
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}
