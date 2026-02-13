#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "../../../actions/submit/submit.h"
#include "../../../core/state.h"
#include "../../../render/colors.h"
#include "../../../render/icons.h"
#include "./restart.h"

static char *outdir     = NULL;
static int restart_flag = 0;

static void print_helper (const char *program_name) {
    printf("Usage: %s [OPTIONS] [RESTART_INDEX]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n"
                                  "  -o, --out DIR  \t\tOutput directory\n"
                                  "  -r, --restart  \t\tRestart on program exit\n";
    printf("%s\n", options_usage);
}

static int parse_options (int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
        {    "help",       no_argument, 0, 'h' },
        {     "out", required_argument, 0, 'o' },
        { "restart",       no_argument, 0, 'r' },
        {         0,                 0, 0,   0 }
    };
    optind = 1; /* Reset for subcommand parsing */
    opterr = 1; /* Enable error messages */
    while ((opt = getopt_long(argc, argv, "ho:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': print_helper(argv[0]); return 0;
            case 'o':
                outdir = optarg;
                printf("Output directory: %s\n", outdir);
                break;
            case 'r':
                printf("Restart on exit enabled.\n");
                restart_flag = 1;
                break;
            case '?':
            default: return -1;
        }
    }
    return optind;
}

static int sort_pidinfo (const void *a, const void *b) {
    return ((pidinfo_t *)a)->parent_pid - ((pidinfo_t *)b)->parent_pid;
}

static int restart (const int restart_idx, const pid_t pid, const char *cmd) {
    /* Terminate the existing process */
    if (kill(pid, SIGTERM) < 0) {
        perror("kill");
        return -1;
    }
    printf("Process with PID %d stopped.\n", pid);

    /* Restart the process */
    size_t cmd_len = strlen(cmd);
    char *cmd_copy = (char *)malloc(cmd_len + 1);
    if (cmd_copy == NULL) {
        perror("malloc");
        return -1;
    }
    memcpy(cmd_copy, cmd, cmd_len);
    cmd_copy[cmd_len] = '\0';
    printf("%s Executing: %s\n", icon_exe, cmd_copy);
    if (submit((subp_t) {
            .cmd     = cmd_copy,
            .outdir  = outdir,
            .restart = restart_flag,
        }) < 1) {
        perror("submit");
        return 1;
    }

    return 0;
}

int handle_restart_command (int argc, char *argv[], void *config) {
    int ind = parse_options(argc, argv);
    if (ind < 0) return 1;
    if (ind == 0) return 0;

    /* ---- Validate and extract single integer ---- */
    if (ind >= argc) {
        fprintf(stderr, "%s%s%s Missing required integer argument.\n", color_red, icon_x,
                color_reset);
        return 1;
    }
    if (ind < argc - 1) {
        fprintf(stderr, "%s%s%s Too many arguments. Only one integer is allowed.\n", color_red,
                icon_x, color_reset);
        return 1;
    }

    char *endptr;
    long value = strtol(argv[ind], &endptr, 10);

    if (*endptr != '\0') { // not a pure integer
        fprintf(stderr, "%s%s%s Argument must be an integer, got: %s\n", color_red, icon_x,
                color_reset, argv[ind]);
        return 1;
    }

    /* ---- Restart process ---- */
    int restart_idx = (int)value;
    int count       = 0;

    pidinfo_t *pidinfo = read_pidfiles(&count);
    if (pidinfo == NULL) {
        perror("read_pidfiles");
        return -1;
    }

    qsort(pidinfo, count, sizeof(pidinfo_t), sort_pidinfo);
    const pidinfo_t *restart_pidinfo = &pidinfo[restart_idx];

    printf("RESTARTING: [%d] Parent: %d, Child: %d, CMD: %s", restart_idx, restart_pidinfo->parent_pid,
           restart_pidinfo->child_pid, restart_pidinfo->cmd);
    printf("Are you sure you want to restart this process? (y/N) ");

    int c = getchar();
    if (c == 'y' || c == 'Y') {
        printf("Restarting process...\n");
        return restart(restart_idx, restart_pidinfo->child_pid, restart_pidinfo->cmd);
    } else {
        printf("Process not restarted\n");
        exit(EXIT_FAILURE);
    }
}
