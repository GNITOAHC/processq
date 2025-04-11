#include "./actions/list/list.h"
#include "./actions/submit/submit.h"
#include "./config.h"
#include "argp.h"

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
    args_t args = argp_parse(argc, argv);

    if (!args.valid) { exit(EXIT_FAILURE); }
    if (args.action == ACTION_NONE) {
        printf("No action specified\n");
        exit(EXIT_FAILURE);
    }

    config_t config;
    if (args.config_path != NULL) config = conf_read(args.config_path);
    else config = conf_read_default();

    if (config.read_status == -1) {
        printf("Config file read error");
        exit(EXIT_FAILURE);
    }

    switch (args.action) {
        case ACTION_SUB:
            printf("SUB: %s\n", args.cmd);
            submit((subp_t) {
                .cmd    = args.cmd,
                .outdir = args.out_path != NULL ? args.out_path : config.default_out_dir,
            });
            printf("");
            break;
        case ACTION_LIST:
            printf("LIST COMMAND\n");
            if (list() < 0) {
                perror("list commands failed");
                exit(EXIT_FAILURE);
            }
            break;
        default: printf("Invalid action\n"); break;
    }

    /* Free allocated memory from config */
    if (config.state_file_path != NULL) free(config.state_file_path);
    if (config.default_out_dir != NULL) free(config.default_out_dir);
    if (config.outfile_pattern != NULL) free(config.outfile_pattern);
    if (config.errfile_pattern != NULL) free(config.errfile_pattern);

    return 0;
}
