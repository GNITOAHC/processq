#include "../../core/state.h"
#include "./stop.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int sort_pidinfo (const void *a, const void *b) {
    return ((pidinfo_t *)a)->pid - ((pidinfo_t *)b)->pid;
}

int stop (const int stop_idx) {
    int count = 0;

    pidinfo_t *pidinfo = read_pidfiles(&count);
    if (pidinfo == NULL) {
        perror("read_pidfiles");
        return -1;
    }

    qsort(pidinfo, count, sizeof(pidinfo_t), sort_pidinfo);
    const pidinfo_t *stop_pidinfo = &pidinfo[stop_idx];

    printf("STOPPING: [%d] PID: %d, CMD: %s\n", stop_idx, stop_pidinfo->pid, stop_pidinfo->cmd);
    printf("Are you sure you want to stop this process? (y/n) ");

    int c = getchar();
    if (c == '\n' || c == 'y' || c == 'Y') {
        printf("Stopping process...\n");
    } else {
        printf("Process not stopped\n");
        exit(EXIT_FAILURE);
    }

    /* Send a SIGTERM to the monitor process */
    if (stop_pidinfo->monitor_pid > 0) {
        if (kill(stop_pidinfo->monitor_pid, SIGTERM) < 0) { perror("kill monitor"); }
    }

    if (kill(stop_pidinfo->pid, SIGTERM) < 0) {
        perror("kill child");
        return -1;
    }
    printf("Process stopped\n");

    return 0;
}
