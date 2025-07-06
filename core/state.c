#include "state.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static short validate_pid_dir (const char *piddir) {
    struct stat st = { 0 };
    if (stat(piddir, &st) == -1) {
        if (mkdir(piddir, 0700) == 0) {
            return 1; // Directory created successfully
        } else {
            perror("mkdir");
            return 0; // Failed to create directory
        }
    }
    return 1; // Directory already exists
}

int write_pidfile (pid_t pid, char *cmd) {
    /* piddir and pidfile */
    char piddir[MAX_PATH_LEN];
    snprintf(piddir, MAX_PATH_LEN, "%s/.local/state/processq", getenv("HOME"));
    if (validate_pid_dir(piddir) == 0) return -1;

    char pidfile[MAX_PATH_LEN];
    snprintf(pidfile, MAX_PATH_LEN, "%s/.local/state/processq/%d", getenv("HOME"), pid);

    /* Check pidfile path */
    struct stat st = { 0 };
    if (stat(piddir, &st) == -1) mkdir(piddir, 0700);

    /* Open the file and write to it */
    const int fd = open(pidfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    /* Write command to the pidfile */
    if (dprintf(fd, "%s\n", cmd) < 0) {
        perror("write to pidfile");
        return -1;
    }

    /* Close the file descriptor */
    close(fd);
    return 0;
}

int remove_pidfile (pid_t pid) {
    char pidfile[MAX_PATH_LEN];
    snprintf(pidfile, MAX_PATH_LEN, "%s/.local/state/processq/%d", getenv("HOME"), pid);

    /* Remove the pidfile */
    if (remove(pidfile) != 0) {
        perror("remove pidfile");
        return -1;
    }

    return 0;
}

pidinfo_t *read_pidfiles (int *array_len) {
    char piddir[MAX_PATH_LEN];
    snprintf(piddir, MAX_PATH_LEN, "%s/.local/state/processq", getenv("HOME"));
    if (validate_pid_dir(piddir) == 0) return NULL;

    DIR *dir;
    if ((dir = opendir(piddir)) == NULL) { return NULL; }

    pidinfo_t *pidinfos = (pidinfo_t *)malloc(sizeof(pidinfo_t) * 10);
    size_t info_len = 0, info_cap = 10;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Skip the entries "." and ".." */
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }
        const int pid = atoi(entry->d_name);
        char pidfile[MAX_PATH_LEN];
        snprintf(pidfile, MAX_PATH_LEN, "%s/.local/state/processq/%d", getenv("HOME"), pid);
        FILE *file = fopen(pidfile, "r");
        if (file == NULL) {
            perror("fopen");
            return NULL;
        }
        char *cmd  = NULL;
        size_t len = 0;
        if (getline(&cmd, &len, file) == -1) {
            perror("getline");
            return NULL;
        }
        /* Resize the pidinfos array if ++len == cap */
        if (info_len + 1 == info_cap) {
            info_cap *= 2;
            pidinfos = (pidinfo_t *)realloc(pidinfos, sizeof(pidinfo_t) * info_cap);
        }

        pidinfos[info_len++] = (pidinfo_t) {
            .pid = pid,
            .cmd = cmd,
        };

        /* printf("PID: %d, CMD: %s\n", pid, cmd); */
        fclose(file);
    }

    closedir(dir);

    *array_len = info_len;

    return pidinfos;
}
