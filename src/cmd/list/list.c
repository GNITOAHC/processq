#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../../actions/list/list.h"
#include "./list.h"

static long id = -1;

static void print_helper (const char *program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n"
                                  "  --id ID        \t\tInspect a specific process by ID\n";
    printf("%s\n", options_usage);
}

int handle_list_command (int argc, char *argv[], void *config) {
    int opt;

    static struct option long_options[] = {
        { "help",       no_argument, 0, 'h' },
        {   "id", required_argument, 0, 'i' },
        {      0,                 0, 0,   0 }
    };
    optind = 1; /* Reset for subcommand parsing */
    opterr = 1; /* Enable error messages */

    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': print_helper(argv[0]); return 0;
            case 'i':
                {
                    const char *id_str = optarg;
                    char *endptr;
                    id = strtol(id_str, &endptr, 10);
                    if (*endptr != '\0' || id < 0) {
                        fprintf(stderr,
                                "error: Invalid ID '%s'. ID must be zero or a positive integer.\n",
                                id_str);
                        return 1;
                    }
                    printf("Inspecting process with ID: %ld\n", id);
                }
                break;
            case '?':
            default: return 1;
        }
    }

    if (id != -1) {
        if (list_by_id(id) < 0) {
            fprintf(stderr, "error: Failed to list process with ID %ld\n", id);
            return 1;
        }
        exit(EXIT_SUCCESS);
    }

    printf("LIST COMMAND\n");
    if (list() < 0) {
        perror("list commands failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}
