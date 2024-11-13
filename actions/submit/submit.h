#pragma once

/* subp_t: submit() parameters */
typedef struct {
    char *cmd, *outdir;
} subp_t;

/* 1: success, 0: failed */
int submit(subp_t p);
