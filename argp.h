#pragma once

#include <getopt.h>

typedef enum {
    ACTION_NONE,
    ACTION_SUB,
    ACTION_LIST,
    ACTION_HELP,
    ACTION_VERSION,
} action_t;

typedef struct {
    /* 1 = valid argument, 0 = invalid argument */
    int valid;
    /* The path to the configuration file and log file, if specified. */
    char *config_path, *out_path;
    /* The command to run */
    char *cmd;
    /* Action to take */
    action_t action;
} args_t;

static struct option long_options[] = {
    {    "help",       no_argument, 0, 'h' },
    { "version",       no_argument, 0, 'v' },
    {  "config", required_argument, 0, 'c' },
    {     "out", required_argument, 0, 'o' },
    {     "cmd", required_argument, 0, 'm' },
    {    "list",       no_argument, 0, 'l' },
    {         0,                 0, 0,   0 }
};

static char version[] = "0.1.1";

static char short_options[] = "hvc:o:m:l";
static char help_message[]  = "Usage: queue [OPTIONS]\n"
                              "Options:\n"
                              "  -h, --help          \t\tDisplay this help message\n"
                              "  -v, --version       \t\tDisplay version information\n"
                              "  -c, --config [FILE] \t\tSpecify a configuration file\n"
                              "  -o, --out [DIR]     \t\tSpecify a output directory\n"
                              "  -m, --cmd [COMMAND] \t\tSpecify a command to run\n"
                              "  -l, --list          \t\tList all running processes\n";

args_t argp_parse(int argc, char *argv[]);
