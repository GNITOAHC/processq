#pragma once

/* Replace the leading ~ to actual home directory, function will modify the given `char *path` directly */
char *util_complete_path(char *path);
