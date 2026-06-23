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

#include "../../actions/submit/submit.h"
#include "../../core/state.h"
#include "../../render/colors.h"
#include "../../render/icons.h"
#include "./cmd.h"

static char *restart_logdir = NULL;
static int restart_flag     = 0;

static int restart (const int restart_idx, const pid_t pid, const char *cmd, const char *workdir,
                    const char *state_logdir) {
    (void)restart_idx;

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

    /* Use user-specified logdir if provided, otherwise use logdir from state file */
    char *effective_logdir = restart_logdir;
    if (effective_logdir == NULL) { effective_logdir = (char *)state_logdir; }

    if (submit((subp_t) {
            .cmd     = cmd_copy,
            .logdir  = effective_logdir,
            .workdir = (char *)workdir,
            .restart = restart_flag,
        }) < 1) {
        perror("submit");
        return 1;
    }

    /* Free the duplicated command string */
    free(cmd_copy);

    return 0;
}

int handle_restart_command (argparse_context_t *ctx) {
    restart_cfg_t *cfg = (restart_cfg_t *)ctx->userdata;
    if (cfg == NULL) {
        fprintf(stderr, "error: No configuration provided for restart command\n");
        return 1;
    }
    if (ctx->argc > 0) {
        fprintf(stderr, "%s%s%s Too many arguments. Only one integer is allowed.\n", color_red,
                icon_x, color_reset);
        return 1;
    }

    /* ---- Restart process ---- */
    int restart_idx = cfg->id;
    int count       = 0;
    restart_logdir  = (char *)cfg->out;
    restart_flag    = cfg->restart;

    pidinfo_t *pidinfos = read_pidfiles(&count);
    if (pidinfos == NULL) {
        perror("read_pidfiles");
        return -1;
    }

    if (restart_idx < 0 || restart_idx >= count) {
        fprintf(stderr, "%s%s%s Restart index out of range: %d\n", color_red, icon_x, color_reset,
                restart_idx);
        for (int i = 0; i < count; i++) {
            free(pidinfos[i].cmd);
            free(pidinfos[i].workdir);
            free(pidinfos[i].logdir);
        }
        free(pidinfos);
        return 1;
    }

    const pidinfo_t *restart_pidinfo = &pidinfos[restart_idx];

    printf("RESTARTING: [%d] Parent: %d, Child: %d, CMD: %s", restart_idx,
           restart_pidinfo->parent_pid, restart_pidinfo->child_pid, restart_pidinfo->cmd);
    printf("Are you sure you want to restart this process? (y/N) ");

    int c = getchar();
    if (c == 'y' || c == 'Y') {
        printf("Restarting process...\n");
        int restart_result = restart(restart_idx, restart_pidinfo->child_pid, restart_pidinfo->cmd,
                                     restart_pidinfo->workdir, restart_pidinfo->logdir);

        /* Free pidinfos and its dynamically allocated fields before returning */
        for (int i = 0; i < count; i++) {
            free(pidinfos[i].cmd);
            free(pidinfos[i].workdir);
            free(pidinfos[i].logdir);
        }
        free(pidinfos);

        return restart_result;
    } else {
        printf("Process not restarted\n");
        for (int i = 0; i < count; i++) {
            free(pidinfos[i].cmd);
            free(pidinfos[i].workdir);
            free(pidinfos[i].logdir);
        }
        free(pidinfos);
        return 1;
    }
}
