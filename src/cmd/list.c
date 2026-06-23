#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../actions/list/list.h"
#include "./cmd.h"

int handle_list_command (argparse_context_t *ctx) {
    list_cfg_t *cfg = (list_cfg_t *)ctx->userdata;
    if (cfg == NULL) {
        fprintf(stderr, "error: No configuration provided for list command\n");
        return 1;
    }

    int id = cfg->id;
    if (id != -1) {
        if (list_by_id(id) < 0) {
            fprintf(stderr, "error: Failed to list process with ID %d\n", id);
            return 1;
        }
        exit(EXIT_SUCCESS);
    }

    printf("LIST COMMAND\n");
    if (list() < 0) {
        /* Error already reported by list() */
        exit(EXIT_FAILURE);
    }
    return 0;
}
