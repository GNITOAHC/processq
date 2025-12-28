#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../../actions/stop/stop.h"
#include "../../../render/colors.h"
#include "../../../render/icons.h"
#include "./stop.h"

static void print_helper (const char *program_name) {
    printf("Usage: %s [OPTIONS] [command]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n";
    printf("%s\n", options_usage);
}

static int parse_options (int argc, char *argv[]) {
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
            default: return -1;
        }
    }
    return optind;
}

int handle_stop_command (int argc, char *argv[], void *config) {
    int ind = parse_options(argc, argv);
    if (ind < 0) return -1;
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

    /* printf("Parsed integer: %ld\n", value); */

    printf("STOP: %ld\n", value);
    if (stop((int)value) < 0) {
        perror("stop command failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}
