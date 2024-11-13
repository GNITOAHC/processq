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

    pidinfo_t *pidinfo = read_pidfiles(&count);
    if (pidinfo == NULL) {
        perror("read_pidfiles");
        return -1;
    }

    for (int i = 0; i < count; ++i) {
        printf("PID: %d, CMD: %s\n", pidinfo[i].pid, pidinfo[i].cmd);
    }

    return 0;
}
