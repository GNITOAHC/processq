#include "config.h"
#include "util.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE      1024
#define REGEX_PATTERN "([A-Za-z0-9_]+)=\"([^\"]+)\""

config_t conf_read (const char *conf_file_path) {
    if (access(conf_file_path, F_OK) == -1) {
        fprintf(stderr, "Config file not found: %s\n", conf_file_path);
        return (config_t) { .read_status = 0 };
    }

    FILE *file = fopen(conf_file_path, "r");
    if (!file) return (config_t) { .read_status = -1 };

    regex_t regex;
    regmatch_t matches[3];
    char line[MAX_LINE];

    config_t cfg = (config_t) {
        .state_file_path = NULL,
        .default_out_dir = NULL,
        .outfile_pattern = NULL,
        .errfile_pattern = NULL,

        .read_status = 1,
    };

    if (regcomp(&regex, REGEX_PATTERN, REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        fclose(file);
        return (config_t) { .read_status = -1 };
    }

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue; /* Skip comments and empty lines */

        /* if (regexec(&regex, line, 3, matches, 0) != 0) continue; */

        if (regexec(&regex, line, 3, matches, 0) == 0) {
            char key[MAX_LINE] = { 0 };
            char *value        = (char *)malloc(MAX_LINE * sizeof(char));

            strncpy(key, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
            strncpy(value, line + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);

            /* printf("Key: %s, Value: %s\n", key, value); */
            if (strcmp(key, "STATE_FILE_PATH") == 0 && cfg.state_file_path == NULL)
                cfg.state_file_path = value;
            if (strcmp(key, "DEFAULT_OUT_DIR") == 0 && cfg.default_out_dir == NULL)
                cfg.default_out_dir = value;
            if (strcmp(key, "OUTFILE_PATTERN") == 0 && cfg.outfile_pattern == NULL)
                cfg.outfile_pattern = value;
            if (strcmp(key, "ERRFILE_PATTERN") == 0 && cfg.errfile_pattern == NULL)
                cfg.errfile_pattern = value;

            /* If value is not used, free it */
            if (strcmp(key, "STATE_FILE_PATH") != 0 && strcmp(key, "DEFAULT_OUT_DIR") != 0 &&
                strcmp(key, "OUTFILE_PATTERN") != 0 && strcmp(key, "ERRFILE_PATTERN") != 0) {
                printf("Freeing value: %s\n", value);
                /* free(value); */
            }
        }
    }

    cfg.state_file_path = util_complete_path(cfg.state_file_path);
    cfg.default_out_dir = util_complete_path(cfg.default_out_dir);
    cfg.outfile_pattern = util_complete_path(cfg.outfile_pattern);
    cfg.errfile_pattern = util_complete_path(cfg.errfile_pattern);

    /* if (cfg.state_file_path != NULL) printf("STATE_FILE_PATH: %s\n", cfg.state_file_path); */
    /* if (cfg.default_out_dir != NULL) printf("DEFAULT_OUT_DIR: %s\n", cfg.default_out_dir); */
    /* if (cfg.outfile_pattern != NULL) printf("OUTFILE_PATTERN: %s\n", cfg.outfile_pattern); */
    /* if (cfg.errfile_pattern != NULL) printf("ERRFILE_PATTERN: %s\n", cfg.errfile_pattern); */

    regfree(&regex);
    fclose(file);

    return cfg;
}

config_t conf_read_default () {
    char default_config_path[MAX_LINE] = { 0 };
    sprintf(default_config_path, "%s/.config/processq.conf", getenv("HOME"));

    /* Check if the default config file exists */
    if (access(default_config_path, F_OK) == -1) {
        /* fprintf(stderr, "Default config file not found: %s\n", default_config_path); */
        return (config_t) { .read_status = 0 };
    }

    return conf_read(default_config_path);
}
