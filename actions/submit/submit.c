#include "../../core/daemonize.h"
#include "../../globals.h"
#include "submit.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Should free the returned pointer */
static char *get_path () {
    char *cwd = getcwd(NULL, 0);
    return cwd;
}

/* Should NOT free the returned pointer */
static char *get_time () {
    time_t now     = time(NULL);
    char *time_str = ctime(&now); /* time_str is ended with \n\0 */
    return time_str;
}

static char *concat_path (char *str, ...) {
    va_list args;
    va_start(args, str);
    char *path    = (char *)malloc(MAX_PATH_LEN);
    char *cur_str = str;

    char *p;
    size_t idx = 0;
    while (cur_str != NULL) {
        for (p = cur_str; *p != '\0' && *p != '\n'; ++p) {
            path[idx++] = *p;
        }
        cur_str = va_arg(args, char *);
    }
    path[idx] = '\0';

    va_end(args);
    return path;
}

int submit (subp_t p) {
    char logdir[MAX_PATH_LEN];
    char *cwd  = get_path();
    char *time = get_time();

    if (p.logdir == NULL) {
        snprintf(logdir, MAX_PATH_LEN, "%s", cwd);
        p.logdir = logdir;
    }
    free(cwd);

    char *formatted_time = (char *)malloc(strlen(time) - 2);
    strncpy(formatted_time, time, strlen(time) - 2);
    for (int i = 0; i < strlen(formatted_time) + 1; ++i)
        if (formatted_time[i] == ' ') formatted_time[i] = '_';

    char *path_out = concat_path(p.logdir, "/", formatted_time, "_out", NULL);
    char *path_err = concat_path(p.logdir, "/", formatted_time, "_err", NULL);
    free(formatted_time);

    printf("OUT: %s\n", path_out);
    printf("ERR: %s\n", path_err);

    daemonize_args_t args = {
        .cmd      = p.cmd,
        .path_out = path_out,
        .path_err = path_err,
        .workdir  = p.workdir,
        .restart  = p.restart,
    };
    pid_t pid = daemonize(&args);

    free(path_out);
    free(path_err);

    return pid;
}
