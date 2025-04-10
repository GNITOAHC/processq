#pragma once

typedef struct {
    char *state_file_path; // STATE_FILE_PATH TODO
    char *default_out_dir; // DEFAULT_OUT_DIR
    char *outfile_pattern; // OUTFILE_PATTERN TODO
    char *errfile_pattern; // ERRFILE_PATTERN TODO

    short int read_status; // Status code for config file read operation
} config_t;

/* read_status: error (-1); file not exists(0); success (1)
 * returned char* should be freed by the caller
 */
config_t conf_read(const char *conf_file_path);
config_t conf_read_default();
