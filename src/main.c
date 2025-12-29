#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VERSION
#define VERSION "dev"
#endif

#include "./cmd/list/list.h"
#include "./cmd/restart/restart.h"
#include "./cmd/stop/stop.h"
#include "./cmd/submit/submit.h"

typedef int (*cmd_handler_t)(int argc, char *argv[], void *config);
typedef struct {
    const char *name;
    const char *description;
    cmd_handler_t handler;
} cmd_t;

static const cmd_t subcommands[] = {
    {  "submit, s",              "Submit a task",  handle_submit_command },
    {    "list, l", "List all running processes",    handle_list_command },
    {    "stop, t",     "Stop a running process",    handle_stop_command },
    { "restart, r",  "Restart a running process", handle_restart_command },
    {         NULL,                         NULL,                   NULL }
};
static const char alias_separator = ',';

void print_usage (const char *program_name) {
    printf("Usage: %s [flags] <command>\n\n", program_name);

    static char global_options_usage[] = "Flags:\n"
                                         "  -h, --help     \tShow this help message\n"
                                         "  -v, --version  \tEnable verbose output\n";

    printf("%s\n", global_options_usage);

    printf("Commands:\n");
    for (const cmd_t *cmd = subcommands; cmd->name != NULL; cmd++) {
        printf("  %-12s    %s\n", cmd->name, cmd->description);
    }
}

int parse_root_options (int argc, char *argv[]) {
    int opt, option_index = 0;

    static struct option long_options[] = {
        {    "help", no_argument, 0, 'h' },
        { "version", no_argument, 0, 'v' },
        {         0,           0, 0,   0 }
    };

    // Reset getopt state
    optind = 1;
    opterr = 0; // Suppress error messages for unknown options

    // Parse global options using POSIX mode (+ stops at first non-option)
    while ((opt = getopt_long(argc, argv, "+hv", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h': print_usage(argv[0]); exit(0);
            case 'v': printf("processq %s\n", VERSION); exit(0);
            case '?': return optind - 1;
            default: return optind - 1;
        }
    }

    return optind;
}

const cmd_t *find_subcommand (const char *name) {
    for (const cmd_t *cmd = subcommands; cmd->name != NULL; cmd++) {
        const char *aliases = cmd->name;
        const char *start   = aliases;

        while (*start) {
            while (*start == ' ') /* skip leading spaces */
                start++;

            /* find next separator or end */
            const char *sep = strchr(start, alias_separator);
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

int main (int argc, char *argv[]) {
    int subcommand_start = parse_root_options(argc, argv);
    if (subcommand_start >= argc) {
        print_usage(argv[0]);
        return 1;
    }

    /* Find the executable subcommand */
    const char *subcommand_name = argv[subcommand_start];
    const cmd_t *subcommand     = find_subcommand(subcommand_name);
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
    return subcommand->handler(sub_argc, sub_argv, NULL);
}
