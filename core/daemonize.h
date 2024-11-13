#pragma once

#include <sys/types.h>

/* Daemonize the process, given the command and the paths to files for stdout and stderr. */
pid_t daemonize(char *cmd, char *path_out, char *path_err);

pid_t test(char *cmd, char *path_out, char *path_err);
