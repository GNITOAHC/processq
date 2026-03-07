#pragma once

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    char *cmd;      /* Command to execute */
    char *path_out; /* Path to file for stdout */
    char *path_err; /* Path to file for stderr */
    char *workdir;  /* Working directory for the process, NULL for current */
    bool restart;   /* Whether to restart the process on exit */
} daemonize_args_t;

/* Daemonize the process, given the command and the paths to files for stdout and stderr. */
pid_t daemonize(daemonize_args_t *args);
