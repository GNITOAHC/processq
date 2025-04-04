#include "argp.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static char *get_path () {
    char cwd[2048];
    return getcwd(cwd, sizeof(cwd));
}
static char *get_time () {
    time_t now                     = time(NULL);
    char *time_str                 = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

args_t argp_parse (int argc, char *argv[]) {
    int opt = 0, opt_index = 0;
    int valid = 1;
    char *cmd = NULL, *config_path = NULL, *out_path = NULL;
    char *invalid_message = NULL;

    action_t action = ACTION_NONE;

    while ((opt = getopt_long(argc, argv, short_options, long_options, &opt_index)) != -1) {
        switch (opt) {
            case 'h': action = ACTION_HELP; break;
            case 'v': printf("Version %.1f\n", version); break;
            case 'c': printf("Config file: %s\n", optarg); break;
            case 'o':
                /* printf("Output directory: %s\n", optarg); */
                out_path = optarg;
                break;
            case 'm':
                cmd    = optarg;
                action = ACTION_SUB;
                break;
            case 'l': action = ACTION_LIST; break;
            case '?': valid = 0; break;
            default: printf("opt = %d\n", opt); break;
        }
    }

    if (!valid) return (args_t) { .valid = 0 };

    /* printf("PATH: %s\n", get_path()); */
    /* printf("TIME: %s\n", get_time()); */

    return (args_t) {
        .valid       = 1,
        .config_path = config_path,
        .out_path    = out_path,
        .cmd         = cmd,
        .action      = action,
    };
}
