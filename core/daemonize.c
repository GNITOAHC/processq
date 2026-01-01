#include "daemonize.h"
#include "state.h"

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Signal handler for graceful shutdown */
static volatile sig_atomic_t stop_requested = 0;

static void handle_stop_signal (int sig) {
    (void)sig;
    stop_requested = 1;
}

static void log_exit_status (const char *path_err, int status) {
    time_t now         = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    int fd = open(path_err, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) { return; }

    char log_buf[256];
    int len;
    // clang-format off
    if (WIFEXITED(status)) {
        len = snprintf(log_buf, sizeof(log_buf), "[queue] [exit] exit_code=%d timestamp=%s\n", WEXITSTATUS(status), time_buf);
    } else if (WIFSIGNALED(status)) {
        len = snprintf(log_buf, sizeof(log_buf), "[queue] [exit] killed_by_signal=%d timestamp=%s\n", WTERMSIG(status), time_buf);
    } else {
        len = snprintf(log_buf, sizeof(log_buf), "[queue] [exit] unknown_exit timestamp=%s\n", time_buf);
    }
    // clang-format on

    write(fd, log_buf, len);
    close(fd);
}

enum log_level { LOG_INFO, LOG_ERROR };
static void log (const char *path_err, enum log_level l, const char *message) {
    int fd = open(path_err, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) { return; }
    time_t now         = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    char log_buf[256];

    if (l == LOG_INFO) {
        snprintf(log_buf, sizeof(log_buf), "[queue] [info] %s timestamp=%s\n", message, time_buf);
    } else if (l == LOG_ERROR) {
        snprintf(log_buf, sizeof(log_buf), "[queue] [error] %s timestamp=%s\n", message, time_buf);
    }

    write(fd, log_buf, strlen(log_buf));
    close(fd);
}

pid_t daemonize (char *cmd, char *path_out, char *path_err, bool restart) {
    pid_t pid;

    /* First fork, create a background process */
    pid = fork();
    if (pid < 0) { exit(EXIT_FAILURE); } /* Exit if fork() fails */
    if (pid > 0) { exit(EXIT_SUCCESS); } /* Exit for the parent process */

    /* Create a new session */
    if (setsid() < 0) { exit(EXIT_FAILURE); }

    /* Second fork, to prevent the process from acquiring a terminal */
    pid = fork();
    if (pid < 0) { exit(EXIT_FAILURE); } /* Exit if fork() fails */
    if (pid > 0) { exit(EXIT_SUCCESS); } /* Exit for the parent process */

    umask(0); /* Set file permissions (umask) */

    /*
     * Third fork, create a monitoring process and the actual submitted process.
     * parent: monitoring process
     * child: submitted process
     */

    signal(SIGTERM, handle_stop_signal);
    signal(SIGINT, handle_stop_signal);
    /* Store monitor PID for child to record */
    pid_t monitor_pid = getpid();

    /* Monitor loop for restarting the child process */
    while (1) {
        pid = fork();
        if (pid < 0) { exit(EXIT_FAILURE); }

        if (pid > 0) {
            int status = 0;

            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            log_exit_status(path_err, status);

            if (remove_pidfile(pid) != 0) {
                perror("remove_pidfile error");
                exit(EXIT_FAILURE);
            }

            if (restart && !stop_requested) {
                log(path_err, LOG_INFO, "Restarting process");
                continue;
            }
            log(path_err, LOG_INFO, "Monitor stopped");
            exit(EXIT_SUCCESS);
        }
        break; /* Child process breaks out of the loop */
    }

    /* In daemonize, monitor is the parent after third fork */
    if (write_pidfile(getpid(), monitor_pid, cmd) < 0) {
        perror("write_pidfile");
        exit(EXIT_FAILURE);
    }

    /* Redirect file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    const int o = O_RDWR | O_CREAT | O_APPEND;
    const int s = S_IRUSR | S_IWUSR;

    const int fd_out = open(path_out, o, s);
    if (fd_out < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (dup2(fd_out, STDOUT_FILENO) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }

    const int fd_err = open(path_err, o, s);
    if (fd_err < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (dup2(fd_err, STDERR_FILENO) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }

    execl("/bin/bash", "bash", "-c", cmd, (char *)NULL);

    /*
     * Failed to execute if execl() returns:
     *
     * Once execl() is called, it'll replace the current process with a new one. If it fails, the
     * process will return -1, then the following perror() will be called.
     */
    perror("execl");
    exit(EXIT_FAILURE);
}
