#pragma once

#include <sys/types.h>

/*
 * State file format:
 *
 * Location: ~/.local/state/processq/<child_pid>
 *
 * Contents (line-separated):
 *   Line 1: monitor_pid (PID of the monitoring process)
 *   Line 2: cmd (the command being executed)
 *
 * Example file ~/.local/state/processq/12345:
 *   12300
 *   sleep 100
 *
 * To stop a restartable daemon, send SIGTERM to monitor_pid.
 */

typedef struct {
    pid_t pid;         /* Child process PID (filename) */
    pid_t monitor_pid; /* Monitor process PID (for stopping restartable daemons) */
    char *cmd;         /* Command being executed */
} pidinfo_t;

int write_pidfile(pid_t pid, pid_t monitor_pid, char *cmd);
int remove_pidfile(pid_t pid);

/* pidinfo & pidinfo.cmd should be freed by the caller */
pidinfo_t *read_pidfiles(int *);
