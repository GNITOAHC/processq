#pragma once

#include <argparse.h>

// list [-h] [--id <id>]
typedef struct {
    int id;
} list_cfg_t;
int handle_list_command(argparse_context_t *ctx);

// restart [-h] [--out <dir>] [--restart] <id>
typedef struct {
    int id;
    const char *out;
    bool restart;
} restart_cfg_t;
int handle_restart_command(argparse_context_t *ctx);

// stop [-h] <id>
typedef struct {
    int id;
} stop_cfg_t;
int handle_stop_command(argparse_context_t *ctx);

// submit [-h] [--out <dir>] [--restart] <command>
// command will be collected from the leftover arguments
typedef struct {
    const char *out;
    bool restart;
} submit_cfg_t;
int handle_submit_command(argparse_context_t *ctx);
