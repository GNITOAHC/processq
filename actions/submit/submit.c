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
    char *cwd = get_path();
    if (cwd == NULL) {
        perror("submit: get_path");
        return -1;
    }

    char absolute_logdir[MAX_PATH_LEN];
    if (p.logdir == NULL) {
        /* Case 1: No logdir provided, use CWD. */
        strncpy(absolute_logdir, cwd, sizeof(absolute_logdir) - 1);
        absolute_logdir[sizeof(absolute_logdir) - 1] = '\0';
    } else if (p.logdir[0] == '/') {
        /* Case 2: Absolute path provided. */
        strncpy(absolute_logdir, p.logdir, sizeof(absolute_logdir) - 1);
        absolute_logdir[sizeof(absolute_logdir) - 1] = '\0';
    } else {
        /* Case 3: Relative path provided. Prepend CWD. */
        snprintf(absolute_logdir, sizeof(absolute_logdir), "%s/%s", cwd, p.logdir);
    }
    free(cwd);

    time_t now         = time(NULL);
    struct tm *tm_info = localtime(&now);
    char formatted_time[20]; /* yyyy-MM-dd-HH-mm-ss\0 */
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d-%H-%M-%S", tm_info);

    char *path_out = concat_path(absolute_logdir, "/", formatted_time, "_out", NULL);
    char *path_err = concat_path(absolute_logdir, "/", formatted_time, "_err", NULL);

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
