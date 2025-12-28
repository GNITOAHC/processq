#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../../actions/submit/submit.h"
#include "../../../render/colors.h"
#include "../../../render/icons.h"
#include "./submit.h"

static char *outdir = NULL;

static void print_helper (const char *program_name) {
    printf("Usage: %s [OPTIONS] [command]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n"
                                  "  -o, --out DIR  \t\tOutput directory\n";
    printf("%s\n", options_usage);
}

static int parse_options (int argc, char *argv[]) {
    int opt;
    static struct option long_options[] = {
        { "help",       no_argument, 0, 'h' },
        {  "out", required_argument, 0, 'o' },
        {      0,                 0, 0,   0 }
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
            case '?':
            default: return -1;
        }
    }
    return optind;
}

int handle_submit_command (int argc, char *argv[], void *config) {
    int ind = parse_options(argc, argv);
    if (ind < 0) return -1;
    if (ind == 0) return 0;

    if (ind >= argc) {
        fprintf(stderr, "%s%s%s Missing required argument.\n", color_red, icon_x, color_reset);
        print_helper(argv[0]);
        return 1;
    }
    if (ind == 0) { return 0; }

    /* Extract remaining arguments */
    size_t total_len = 0; /* compute length */
    for (int i = optind; i < argc; i++) {
        total_len += strlen(argv[i]) + 1; /* +1 for space or '\0' */
    }

    char *remaining = malloc(total_len);
    if (!remaining) {
        perror("malloc");
        return 1;
    }

    remaining[0] = '\0';
    for (int i = optind; i < argc; i++) {
        strcat(remaining, argv[i]);
        if (i < argc - 1) strcat(remaining, " ");
    }

    printf("%s Executing: %s\n", icon_exe, remaining);
    if (submit((subp_t) {
            .cmd    = remaining,
            .outdir = outdir,
        }) < 1) {
        perror("submit");
        return 1;
    }

    free(remaining);

    return 0;
}
