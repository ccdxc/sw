/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#include "misc.h"

char *
human_readable_buf(char *buf, size_t bufsz, u_int64_t n)
{
    static const char units[][2] = { "", "K", "M", "G", "T" };
    int i;

    for (i = 0; i < sizeof(units) / sizeof(units[0]) - 1; i++) {
        if (n & (1024 - 1)) break;
        n >>= 10;
    }
    snprintf(buf, bufsz, "%"PRIu64"%s", n, units[i]);
    return buf;
}

char *
human_readable(u_int64_t n)
{
#define NBUFS 8
    static char buf[NBUFS][32];
    static int bufi;

    return human_readable_buf(buf[bufi++ % NBUFS], sizeof(buf[0]), n);
}
