#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <argparse.h>

#include "../../actions/submit/submit.h"
#include "../../render/colors.h"
#include "../../render/icons.h"
#include "./cmd.h"

int handle_submit_command (argparse_context_t *ctx) {
    submit_cfg_t *cfg = (submit_cfg_t *)ctx->userdata;
    if (cfg == NULL) {
        fprintf(stderr, "error: No configuration provided for submit command\n");
        return 1;
    }

    char *outdir = (char *)cfg->out;
    int restart  = cfg->restart;

    if (ctx->argc <= 0) {
        fprintf(stderr, "%s%s%s Missing required argument.\n", color_red, icon_x, color_reset);
        argparse_print_help(ctx->command, stdout);
        return 1;
    }

    /* Extract remaining arguments */
    size_t total_len = 0; /* compute length */
    for (int i = 0; i < ctx->argc; i++) {
        total_len += strlen(ctx->argv[i]) + 1; /* +1 for space or '\0' */
    }

    char *remaining = malloc(total_len);
    if (!remaining) {
        perror("malloc");
        return 1;
    }

    remaining[0] = '\0';
    for (int i = 0; i < ctx->argc; i++) {
        strcat(remaining, ctx->argv[i]);
        if (i < ctx->argc - 1) strcat(remaining, " ");
    }

    printf("%s Executing: %s\n", icon_exe, remaining);
    if (submit((subp_t) { .cmd = remaining, .logdir = outdir, .restart = restart }) < 1) {
        perror("submit");
        free(remaining);
        return 1;
    }

    free(remaining);

    return 0;
}
