#pragma once

#include <stdbool.h>
#include <sys/types.h>

/* Daemonize the process, given the command and the paths to files for stdout and stderr. */
pid_t daemonize(char *cmd, char *path_out, char *path_err, bool restart);
