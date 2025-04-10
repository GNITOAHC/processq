#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *util_complete_path (char *path) {
    if (path == NULL) return NULL;

    char path_copy[1024];
    strncpy(path_copy, path, sizeof(path_copy) - 1);

    /* char *realpath = (char *)malloc(sizeof(char) * 1024); */

    if (strncmp("~", path, strlen("~")) == 0) {
        char *home_dir = getenv("HOME");
        if (home_dir != NULL) {
            sprintf(path, "%s/%s", home_dir, path_copy + 2);
            return path;
        } else {
            // Handle the case where HOME environment variable is not set
            return path;
        }
    } else {
        return path;
    }
}
