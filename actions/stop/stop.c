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

int stop (const int stop_idx) {
    int count = 0;

    pidinfo_t *pidinfos = read_pidfiles(&count);
    if (pidinfos == NULL) {
        perror("read_pidfiles");
        return -1;
    }

    const pidinfo_t *stop_pidinfo = &pidinfos[stop_idx];

    printf("STOPPING: [%d] Parent: %d, Child: %d, CMD: %s", stop_idx, stop_pidinfo->parent_pid,
           stop_pidinfo->child_pid, stop_pidinfo->cmd);
    printf("Are you sure you want to stop this process? (y/n) ");

    int c = getchar();
    if (c == '\n' || c == 'y' || c == 'Y') {
        printf("Stopping process...\n");
    } else {
        printf("Process not stopped\n");
        exit(EXIT_FAILURE);
    }

    /* Send a SIGTERM to the parent (monitor) process */
    if (stop_pidinfo->parent_pid > 0) {
        if (kill(stop_pidinfo->parent_pid, SIGTERM) < 0) { perror("kill monitor"); }
    }

    if (kill(stop_pidinfo->child_pid, SIGTERM) < 0) {
        perror("kill child");
        return -1;
    }
    printf("Process stopped\n");

    return 0;
}
