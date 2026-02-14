#pragma once

#include <sys/types.h>

/*
 * State file format:
 *
 * Location: ~/.local/state/processq/<parent_pid>
 *
 * Contents (line-separated):
 *   Line 1: child_pid (PID of the actual running process)
 *   Line 2: cmd (the command being executed)
 *
 * Example file ~/.local/state/processq/12300:
 *   12345
 *   sleep 100
 *
 * To stop a restartable daemon, send SIGTERM to parent_pid (the monitor).
 * The filename (parent_pid) stays constant across restarts; only child_pid changes.
 */

typedef struct {
    pid_t parent_pid; /* Monitor process PID (filename, constant across restarts) */
    pid_t child_pid;  /* Child process PID (line 1, changes on restart) */
    char *cmd;        /* Command being executed */
} pidinfo_t;

int write_pidfile(pid_t parent_pid, pid_t child_pid, char *cmd);
int remove_pidfile(pid_t parent_pid);
int update_child_pid(pid_t parent_pid, pid_t new_child_pid);

/*
 * pidinfo & pidinfo.cmd should be freed by the caller
 * returned array is allready sorted by parent_pid
 */
pidinfo_t *read_pidfiles(int *);
