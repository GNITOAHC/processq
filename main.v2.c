#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./actions/list/list.h"
#include "./actions/stop/stop.h"
#include "./actions/submit/submit.h"

#include "render/colors.h"
#include "render/icons.h"

/*
 * Global configuration structure
 */
typedef struct {
    char *config_path;
} g_config_t;

/*
 * Subcommand function pointer type
 */
typedef int (*subcommand_handler_t)(int argc, char *argv[], g_config_t *global);
typedef struct {
    const char *name;
    const char *description;
    subcommand_handler_t handler;
} subcommand_t;

/*
 * Subcommand definitions
 */
int handle_submit_command(int argc, char *argv[], g_config_t *global);
int handle_list_command(int argc, char *argv[], g_config_t *global);
int handle_stop_command(int argc, char *argv[], g_config_t *global);
int handle_restart_command(int argc, char *argv[], g_config_t *global);

static const subcommand_t subcommands[] = {
    {  "submit, s",              "Submit a task",  handle_submit_command },
    {    "list, l", "List all running processes",    handle_list_command },
    {    "stop, t",     "Stop a running process",    handle_stop_command },
    { "restart, r",  "Restart a running process", handle_restart_command },
    {         NULL,                         NULL,                   NULL }
};
static const char subcommand_separator = ',';

const subcommand_t *find_subcommand (const char *name) {
    for (const subcommand_t *cmd = subcommands; cmd->name != NULL; cmd++) {
        const char *aliases = cmd->name;
        const char *start   = aliases;

        while (*start) {
            while (*start == ' ') /* skip leading spaces */
                start++;

            /* find next separator or end */
            const char *sep = strchr(start, subcommand_separator);
            const char *end = sep ? sep : start + strlen(start);

            /* trim trailing spaces before separator */
            const char *trim_end = end - 1;
            while (trim_end >= start && *trim_end == ' ')
                trim_end--;

            size_t len = (size_t)(trim_end - start + 1);

            if (strlen(name) == len && strncmp(name, start, len) == 0) return cmd;

            if (!sep) break;
            start = sep + 1;
        }
    }
    return NULL;
}

/* Print usage information */
void print_usage (const char *program_name) {
    printf("Usage: %s [GLOBAL_OPTIONS] <subcommand> [SUBCOMMAND_OPTIONS] [arguments]\n\n",
           program_name);

    static char global_options_usage[] = "Global Options:\n"
                                         "  -h, --help        \t\tShow this help message\n"
                                         "  -v, --version     \t\tEnable verbose output\n"
                                         "  -c, --config FILE \t\tSpecify configuration file\n";

    printf("%s\n", global_options_usage);

    printf("Subcommands:\n");
    for (const subcommand_t *cmd = subcommands; cmd->name != NULL; cmd++) {
        printf("  %-12s    %s\n", cmd->name, cmd->description);
    }

    /* printf("\nIMPORTANT - Option Conflicts:\n"); */
    /* printf("  Global -c: Color flag (no argument) - enables/disables colors\n"); */
    /* printf("  build -c:  Compiler path (requires argument) - path to compiler\n"); */
    /* printf("  deploy -c: Config file (requires argument) - deployment config\n"); */
    /* printf("  test -c:   Coverage file (requires argument) - coverage output file\n"); */

    /* printf("\nExamples:\n"); */
    /* printf("  %s -c build -c /usr/bin/gcc main.c\n", program_name); */
    /* printf("  %s --color deploy -c production.conf\n", program_name); */
    /* printf("  %s -v test -c coverage.xml\n", program_name); */
}

/*
 * Parse the global options
 */
int parse_global_options (int argc, char *argv[], g_config_t *global) {
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        {    "help",       no_argument, 0, 'h' },
        { "version",       no_argument, 0, 'v' },
        {  "config", required_argument, 0, 'c' },
        {         0,                 0, 0,   0 }
    };

    // Initialize global options
    memset(global, 0, sizeof(g_config_t));

    // Reset getopt state
    optind = 1;
    opterr = 0; // Suppress error messages for unknown options

    // Parse global options using POSIX mode (+ stops at first non-option)
    // NOTE: "c" has NO COLON - it's a flag, not requiring an argument
    while ((opt = getopt_long(argc, argv, "+hvc:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h': print_usage(argv[0]); exit(0);
            case 'v': printf("query v2.0\n"); exit(0);
            case 'c': global->config_path = optarg; break;
            case '?': return optind - 1;
            default: return optind - 1;
        }
    }

    return optind;
}

int main (int argc, char *argv[]) {
    g_config_t global_config;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    /* Parse global options first */
    int subcommand_start = parse_global_options(argc, argv, &global_config);
    if (subcommand_start >= argc) {
        fprintf(stderr, "error: No subcommand specified\n\n");
        print_usage(argv[0]);
        return 1;
    }

    /* Find the executable subcommand */
    const char *subcommand_name    = argv[subcommand_start];
    const subcommand_t *subcommand = find_subcommand(subcommand_name);
    if (subcommand == NULL) {
        fprintf(stderr, "error: Unknown subcommand '%s'\n\n", subcommand_name);
        print_usage(argv[0]);
        return 1;
    }
    /* printf("Found subcommand: %s\n\n", subcommand->name); */

    /* Prepare arguments for subcommand */
    int sub_argc    = argc - subcommand_start;
    char **sub_argv = &argv[subcommand_start];

    /* Execute subcommand */
    return subcommand->handler(sub_argc, sub_argv, &global_config);
}

/*
 * Subcommand handlers
 */
int print_submit_helper (char *program_name) {
    printf("Usage: %s [OPTIONS] [arguments]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n"
                                  "  -o, --out DIR  \t\tSpecify output directory\n";
    printf("%s\n", options_usage);
    return 0;
}
int handle_submit_command (int argc, char *argv[], g_config_t *global) {
    char *outdir = NULL;
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
            case 'h': print_submit_helper(argv[0]); return 0;
            case 'o':
                outdir = optarg;
                printf("%s Output directory: %s\n", icon_dir, optarg);
                break;
            case '?': return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "%s%s%s Missing required argument.\n", color_red, icon_x, color_reset);
        print_submit_helper(argv[0]);
        return 1;
    }

    /* ---- Extract remaining args ---- */
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

int print_list_helper (char *program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n";
    printf("%s\n", options_usage);
    return 0;
}
int handle_list_command (int argc, char *argv[], g_config_t *global) {
    int opt;

    static struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        {      0,           0, 0,   0 }
    };
    optind = 1; /* Reset for subcommand parsing */
    opterr = 1; /* Enable error messages */

    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': print_list_helper(argv[0]); return 0;
            case '?':
            default: return 1;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "%s%s%s Missing required argument.\n", color_red, icon_x, color_reset);
        return 1;
    }
    printf("LIST COMMAND\n");
    if (list() < 0) {
        perror("list commands failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int print_stop_helper (char *program_name) {
    printf("Usage: %s [OPTIONS] [arguments]\n\n", program_name);
    static char options_usage[] = "Options:\n"
                                  "  -h, --help     \t\tShow this help message\n";
    printf("%s\n", options_usage);
    return 0;
}
int handle_stop_command (int argc, char *argv[], g_config_t *global) {
    int opt;

    static struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        {      0,           0, 0,   0 }
    };
    optind = 1; /* Reset for subcommand parsing */
    opterr = 1; /* Enable error messages */

    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h': print_stop_helper(argv[0]); return 0;
            case '?':
            default: return 1;
        }
    }
    /* ---- Validate and extract single integer ---- */
    if (optind >= argc) {
        fprintf(stderr, "%s%s%s Missing required integer argument.\n", color_red, icon_x,
                color_reset);
        return 1;
    }
    if (optind < argc - 1) {
        fprintf(stderr, "%s%s%s Too many arguments. Only one integer is allowed.\n", color_red,
                icon_x, color_reset);
        return 1;
    }

    char *endptr;
    long value = strtol(argv[optind], &endptr, 10);

    if (*endptr != '\0') { // not a pure integer
        fprintf(stderr, "%s%s%s Argument must be an integer, got: %s\n", color_red, icon_x,
                color_reset, argv[optind]);
        return 1;
    }

    printf("  🔢 Parsed integer: %ld\n", value);

    printf("STOP: %ld\n", value);
    if (stop((int)value) < 0) {
        perror("stop command failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int print_restart_helper (char *program_name) {
    return 0;
}
int handle_restart_command (int argc, char *argv[], g_config_t *global) {
    printf("\uf489");
    return 0;
}
