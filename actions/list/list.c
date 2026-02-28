#include "../../core/state.h"
#include "list.h"

#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Return 1 if the process is alive, 0 if no such process, -1 if no permission to check. */
static int check_alive (pid_t pid) {
    if (kill(pid, 0) == 0) { /* Process is alive */
        return 0;
    } else { /* Process is dead */

        if (errno == ESRCH) /* No such process */
            return -1;
        if (errno == EPERM) /* Operation not permitted */
            return -1;

        return -1;
    }
    return 0;
}

int list () {
    int count = 0;

    pidinfo_t *pidinfos = read_pidfiles(&count);
    if (pidinfos == NULL) {
        perror("read_pidfiles");
        return -1;
    }

    for (int i = 0; i < count; ++i) {
        printf("[%d] Parent: %d, Child: %d, CMD: %s", i, pidinfos[i].parent_pid,
               pidinfos[i].child_pid, pidinfos[i].cmd);
    }

    for (int i = 0; i < count; ++i)
        free(pidinfos[i].cmd);
    free(pidinfos);

    return 0;
}

int list_by_id (const long id) {
    int count = 0;

    pidinfo_t *pidinfos = read_pidfiles(&count);
    if (pidinfos == NULL) {
        fprintf(stderr, "No process found with ID %ld\n", id);
        return -1;
    }

    if (count == 0) {
        fprintf(stderr, "No processes found\n");
        free(pidinfos);
        return -1;
    }

    /* Check if ID is within bounds */
    if (id < 0 || id >= count) {
        fprintf(stderr, "No process found with ID %ld\n", id);
        return -1;
    }

    /* Get the process info */
    const pidinfo_t *pidinfo = &pidinfos[id];
    printf("[%ld] Parent: %d, Child: %d, CMD: %s\nWorking Directory: %s\nLog Directory: %s\n", id,
           pidinfo->parent_pid, pidinfo->child_pid, pidinfo->cmd, pidinfo->workdir,
           pidinfo->logdir);

    for (int i = 0; i < count; ++i)
        free(pidinfos[i].cmd);
    free(pidinfos);

    return 0;
}
