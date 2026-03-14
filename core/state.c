#include "../globals.h"
#include "state.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static int mkdir_p (const char *path, mode_t _mode) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path);
    return system(cmd);
}

static short validate_pid_dir (const char *piddir) {
    struct stat st = { 0 };
    if (stat(piddir, &st) == -1) {
        if (mkdir_p(piddir, 0700) == 0) {
            return 1; // Directory created successfully
        } else {
            perror("mkdir");
            return 0; // Failed to create directory
        }
    }
    return 1; // Directory already exists
}

int write_pidfile (pid_t parent_pid, pid_t child_pid, char *cmd, char *workdir, char *logdir) {
    /* piddir and pidfile */
    char piddir[MAX_PATH_LEN];
    snprintf(piddir, MAX_PATH_LEN, "%s/.local/state/processq", getenv("HOME"));
    if (validate_pid_dir(piddir) == 0) return -1;

    char pidfile[MAX_PATH_LEN];
    snprintf(pidfile, MAX_PATH_LEN, "%s/.local/state/processq/%d", getenv("HOME"), parent_pid);

    /* Check pidfile path */
    struct stat st = { 0 };
    if (stat(piddir, &st) == -1) mkdir(piddir, 0700);

    /* Open the file and write to it */
    const int fd = open(pidfile, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    /* Fall back to CWD if workdir is NULL */
    char cwd_buf[MAX_PATH_LEN];
    char *effective_workdir = workdir;
    if (effective_workdir == NULL) {
        if (getcwd(cwd_buf, sizeof(cwd_buf)) != NULL) {
            effective_workdir = cwd_buf;
        } else {
            effective_workdir = ".";
        }
    }

    /* Write child_pid and command to the pidfile */
    if (dprintf(fd, "%d\n%s\n%s\n%s\n", child_pid, cmd, effective_workdir, logdir ? logdir : "") <
        0) {
        perror("write to pidfile");
        return -1;
    }

    /* Close the file descriptor */
    close(fd);
    return 0;
}

int remove_pidfile (pid_t parent_pid) {
    char pidfile[MAX_PATH_LEN];
    snprintf(pidfile, MAX_PATH_LEN, "%s/.local/state/processq/%d", getenv("HOME"), parent_pid);

    /* Remove the pidfile */
    if (remove(pidfile) != 0) {
        perror("remove pidfile");
        return -1;
    }

    return 0;
}

int update_child_pid (pid_t parent_pid, pid_t new_child_pid) {
    char pidfile[MAX_PATH_LEN];
    snprintf(pidfile, MAX_PATH_LEN, "%s/.local/state/processq/%d", getenv("HOME"), parent_pid);

    /* Read existing command from the file */
    FILE *file = fopen(pidfile, "r");
    if (file == NULL) {
        perror("fopen for update");
        return -1;
    }

    /* Skip the first line (old child_pid) */
    char *old_child_line = NULL;
    size_t old_len       = 0;
    if (getline(&old_child_line, &old_len, file) == -1) {
        perror("getline old child_pid");
        fclose(file);
        return -1;
    }
    free(old_child_line);

    /* Read command (second line) */
    char *cmd  = NULL;
    size_t len = 0;
    if (getline(&cmd, &len, file) == -1) {
        perror("getline cmd in update");
        free(cmd);
        fclose(file);
        return -1;
    }

    /* Read workdir (third line) */
    char *workdir      = NULL;
    size_t workdir_len = 0;
    if (getline(&workdir, &workdir_len, file) == -1) {
        perror("getline workdir in update");
        free(cmd);
        fclose(file);
        return -1;
    }

    /* Read logdir (fourth line) */
    char *logdir      = NULL;
    size_t logdir_len = 0;
    if (getline(&logdir, &logdir_len, file) == -1) {
        perror("getline logdir in update");
        free(cmd);
        free(workdir);
        fclose(file);
        return -1;
    }
    fclose(file);

    /* Remove trailing newlines */
    size_t cmd_len = strlen(cmd);
    if (cmd_len > 0 && cmd[cmd_len - 1] == '\n') { cmd[cmd_len - 1] = '\0'; }
    size_t workdir_strlen = strlen(workdir);
    if (workdir_strlen > 0 && workdir[workdir_strlen - 1] == '\n') {
        workdir[workdir_strlen - 1] = '\0';
    }
    size_t logdir_strlen = strlen(logdir);
    if (logdir_strlen > 0 && logdir[logdir_strlen - 1] == '\n') {
        logdir[logdir_strlen - 1] = '\0';
    }

    /* Rewrite the file with new child_pid */
    const int fd = open(pidfile, O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("open for update");
        free(cmd);
        free(workdir);
        free(logdir);
        return -1;
    }

    if (dprintf(fd, "%d\n%s\n%s\n%s\n", new_child_pid, cmd, workdir, logdir) < 0) {
        perror("write updated pidfile");
        free(cmd);
        free(workdir);
        free(logdir);
        close(fd);
        return -1;
    }

    free(cmd);
    free(workdir);
    free(logdir);
    close(fd);
    return 0;
}

static int sort_pidinfo (const void *a, const void *b) {
    return ((pidinfo_t *)a)->parent_pid - ((pidinfo_t *)b)->parent_pid;
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
        /* Read child_pid (first line) */
        char *child_pid_line = NULL;
        size_t child_len     = 0;
        if (getline(&child_pid_line, &child_len, file) == -1) {
            if (ferror(file)) {
                perror("getline child_pid");
            } else {
                fprintf(stderr, "read_pidfiles: empty or corrupted pid file: %s\n", pidfile);
            }
            free(child_pid_line);
            fclose(file);
            return NULL;
        }
        pid_t child_pid = atoi(child_pid_line);
        free(child_pid_line);

        /* Read command (second line) */
        char *cmd  = NULL;
        size_t len = 0;
        if (getline(&cmd, &len, file) == -1) {
            if (ferror(file)) {
                perror("getline cmd");
            } else {
                fprintf(stderr, "read_pidfiles: missing command in pid file: %s\n", pidfile);
            }
            free(cmd);
            fclose(file);
            return NULL;
        }

        /* Read workdir (third line) */
        char *workdir      = NULL;
        size_t workdir_len = 0;
        if (getline(&workdir, &workdir_len, file) == -1) {
            perror("getline workdir");
            free(cmd);
            fclose(file);
            return NULL;
        }
        /* Strip trailing newline from workdir */
        size_t workdir_strlen = strlen(workdir);
        if (workdir_strlen > 0 && workdir[workdir_strlen - 1] == '\n') {
            workdir[workdir_strlen - 1] = '\0';
        }

        /* Read logdir (fourth line) */
        char *logdir      = NULL;
        size_t logdir_len = 0;
        if (getline(&logdir, &logdir_len, file) == -1) {
            perror("getline logdir");
            free(cmd);
            free(workdir);
            fclose(file);
            return NULL;
        }
        /* Strip trailing newline from logdir */
        size_t logdir_strlen = strlen(logdir);
        if (logdir_strlen > 0 && logdir[logdir_strlen - 1] == '\n') {
            logdir[logdir_strlen - 1] = '\0';
        }

        /* Resize the pidinfos array if ++len == cap */
        if (info_len + 1 == info_cap) {
            info_cap *= 2;
            pidinfos = (pidinfo_t *)realloc(pidinfos, sizeof(pidinfo_t) * info_cap);
        }

        /* pid from filename is the parent_pid (monitor), child_pid is from line 1 */
        pidinfos[info_len++] = (pidinfo_t) {
            .parent_pid = pid,
            .child_pid  = child_pid,
            .cmd        = cmd,
            .workdir    = workdir,
            .logdir     = logdir,
        };

        /* printf("PID: %d, CMD: %s\n", pid, cmd); */
        fclose(file);
    }

    closedir(dir);

    *array_len = info_len;

    qsort(pidinfos, info_len, sizeof(pidinfo_t), sort_pidinfo);
    return pidinfos;
}
