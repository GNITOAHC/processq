#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../../actions/list/list.h"
#include "./list.h"
/* #include "../../../render/colors.h" */
/* #include "../../../render/icons.h" */

static void print_helper (const char *program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n";
    printf("%s\n", options_usage);
}

int handle_list_command (int argc, char *argv[], void *config) {
    int opt;

    static struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        {      0,           0, 0,   0 }
    };
    optind = 1; /* Reset for subcommand parsing */
    opterr = 1; /* Enable error messages */

    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': print_helper(argv[0]); return 0;
            case '?':
            default: return 1;
        }
    }

    printf("LIST COMMAND\n");
    if (list() < 0) {
        /* Error already reported by list() */
        exit(EXIT_FAILURE);
    }
    return 0;
}
