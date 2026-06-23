#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <argparse.h>

#ifndef VERSION
#define VERSION "dev"
#endif

#include "./cmd/cmd.h"

/* #include "./cmd/list/list.h" */
/* #include "./cmd/restart/restart.h" */
/* #include "./cmd/stop/stop.h" */
/* #include "./cmd/submit/submit.h" */

typedef int (*cmd_handler_t)(int argc, char *argv[], void *config);
typedef struct {
    const char *name;
    const char *description;
    cmd_handler_t handler;
} cmd_t;

static const cmd_t subcommands[] = {
    /* { "submit, sub, s",              "Submit a task",  handle_submit_command }, */
    /* {    "list, l, ls", "List all running processes",    handle_list_command }, */
    /* {     "stop, t, x",     "Stop a running process",    handle_stop_command }, */
    /* {     "restart, r",  "Restart a running process", handle_restart_command }, */
    { NULL, NULL, NULL }
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
        printf("  %-15s    %s\n", cmd->name, cmd->description);
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

static bool show_version = false;
int root_handler (argparse_context_t *ctx) {
    if (show_version) {
        printf("processq %s\n", VERSION);
    } else {
        argparse_print_help(ctx->command, stdout);
    }
    return 0;
}

int cli (int argc, char *argv[]) {
    argparse_t *parser       = argparse_new("queue", "A simple task daemonizer");
    argparse_command_t *root = argparse_root(parser);
    argparse_command_t *submit, *list, *stop, *restart;
    argparse_pos_t *pos;

    if (parser == NULL) { return 1; }

    /* root command flags */
    argparse_flag_bool(root, 'v', "version", &show_version, "Show version information");
    argparse_command_set_handler(root, root_handler);

    /* queue submit */
    submit_cfg_t submit_cfg = { .out = NULL, .restart = false };

    submit = argparse_command_add(root, "submit", "Submit a task");
    argparse_command_add_alias(submit, "sub");
    argparse_command_add_alias(submit, "s");
    argparse_flag_string(submit, 'o', "out", &submit_cfg.out, "Output directory");
    argparse_flag_bool(submit, 'r', "restart", &submit_cfg.restart, "Restart on exit");
    argparse_command_set_userdata(submit, &submit_cfg);
    argparse_command_set_handler(submit, handle_submit_command);

    /* queue list */
    list_cfg_t list_cfg = { .id = -1 };

    list = argparse_command_add(root, "list", "List all running processes");
    argparse_command_add_alias(list, "ls");
    argparse_command_add_alias(list, "l");
    argparse_flag_int(list, 'i', "id", &list_cfg.id, "Process ID");
    argparse_command_set_userdata(list, &list_cfg);
    argparse_command_set_handler(list, handle_list_command);

    /* queue stop */
    stop_cfg_t stop_cfg = { .id = -1 };

    stop = argparse_command_add(root, "stop", "Stop a running process");
    argparse_command_add_alias(stop, "t");
    argparse_command_add_alias(stop, "x");
    pos = argparse_positional_int(stop, "id", &stop_cfg.id, "Process ID");
    argparse_positional_required(pos, true);
    argparse_command_set_userdata(stop, &stop_cfg);
    argparse_command_set_handler(stop, handle_stop_command);

    /* queue restart */
    restart_cfg_t restart_cfg = { .id = -1, .out = NULL, .restart = false };

    restart = argparse_command_add(root, "restart", "Restart a running process");
    argparse_command_add_alias(restart, "r");
    argparse_flag_string(restart, 'o', "out", &restart_cfg.out, "Output directory");
    argparse_flag_bool(restart, 'r', "restart", &restart_cfg.restart, "Restart on exit");
    pos = argparse_positional_int(restart, "id", &restart_cfg.id, "Process ID");
    argparse_positional_required(pos, true);
    argparse_command_set_userdata(restart, &restart_cfg);
    argparse_command_set_handler(restart, handle_restart_command);

    const int rc = argparse_run(parser, argc, argv);
    argparse_free(parser);
    return (rc < 0) ? 1 : rc;
}

int main (int argc, char *argv[]) {
    return cli(argc, argv);

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
