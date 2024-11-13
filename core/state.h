#pragma once

#include "../globals.h"
#include <sys/types.h>

typedef struct {
    int pid;
    char *cmd;
} pidinfo_t;

int write_pidfile(pid_t pid, char *cmd);
int remove_pidfile(pid_t pid);
pidinfo_t *read_pidfiles(int *);
