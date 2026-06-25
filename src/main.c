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
}
