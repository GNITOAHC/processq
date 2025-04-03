#pragma once

#include "../globals.h"
#include <sys/types.h>

typedef struct {
    int pid;
    char *cmd;
} pidinfo_t;

int write_pidfile(pid_t pid, char *cmd);
int remove_pidfile(pid_t pid);

/* pidinfo & pidinfo.cmd should be freed by the caller */
pidinfo_t *read_pidfiles(int *);
