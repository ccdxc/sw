/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#include "misc.h"

int
main(int argc, char *argv[])
{
    u_int64_t n;
    int i;
    extern int humanize_number(char *, size_t, int, char *, int, int);

    for (i = 1; i < argc; i++) {
        n = strtoull(argv[i], NULL, 0);
        printf("n 0x%"PRIx64": %s\n", n, human_readable(n));
    }
    exit(0);
}
