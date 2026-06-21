#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../actions/stop/stop.h"
#include "../../render/colors.h"
#include "../../render/icons.h"
#include "./cmd.h"

int handle_stop_command (argparse_context_t *ctx) {
    stop_cfg_t *cfg = (stop_cfg_t *)ctx->userdata;
    if (cfg == NULL) {
        fprintf(stderr, "error: No configuration provided for stop command\n");
        return 1;
    }
    if (ctx->argc > 0) {
        fprintf(stderr, "%s%s%s Too many arguments. Only one integer is allowed.\n", color_red,
                icon_x, color_reset);
        return 1;
    }

    printf("STOP: %d\n", cfg->id);
    if (stop(cfg->id) < 0) {
        perror("stop command failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}
