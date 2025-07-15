#include "daemonize.h"
#include "state.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t test (char *cmd, char *path_out, char *path_err) {
    printf("CMD: %s\n", cmd);
    printf("OUT: %s\n", path_out);
    printf("ERR: %s\n", path_err);

    /* return 0; */

    /* Open the file and write to it */
    int fd_out = open(path_out, O_RDWR | O_CREAT);
    int fd_err = open(path_err, O_RDWR | O_CREAT);

    if (fd_out < 0 || fd_err < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* Write to the output file */
    if (write(fd_out, "OUT_TEST\n", 9) != 9) {
        perror("write to path_out");
        exit(EXIT_FAILURE);
    }

    /* Write to the error file */
    if (write(fd_err, "ERR_TEST\n", 9) != 9) {
        perror("write to path_err");
        exit(EXIT_FAILURE);
    }

    /* Close the file descriptors */
    close(fd_out);
    close(fd_err);

    return 0;
}

pid_t daemonize (char *cmd, char *path_out, char *path_err) {
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
    pid = fork();
    if (pid < 0) { exit(EXIT_FAILURE); }

    if (pid > 0) {
        int status = 0;

        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (remove_pidfile(pid) != 0) {
            perror("remove_pidfile error");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    if (write_pidfile(getpid(), cmd) < 0) {
        perror("write_pidfile");
        exit(EXIT_FAILURE);
    }

    /* Redirect file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    const int o = O_RDWR | O_CREAT | O_TRUNC;
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

    open(path_out, o, s);
    open(path_err, o, s);

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
