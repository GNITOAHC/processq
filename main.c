#include "./actions/list/list.h"
#include "./actions/submit/submit.h"
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

    switch (args.action) {
        case ACTION_SUB:
            printf("SUB: %s\n", args.cmd);
            submit((subp_t) {
                .cmd    = args.cmd,
                .outdir = args.out_path,
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
        case ACTION_HELP:
            printf("%s\n", help_message);
            printf("");
            break;
        default: printf("Invalid action\n"); break;
    }

    return 0;
}
