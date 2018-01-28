/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <sys/types.h>
#include "misc.h"

char *
hex_format(char *buf, size_t buflen,
           const void *data, const size_t datalen)
{
    const unsigned char *dp = data;
    size_t blen = buflen;
    char *bp = buf;
    char *sep = "";
    int i, n;

    for (i = 0; blen > 0 && i < datalen; i++) {
        n = snprintf(bp, blen, "%s%02x", sep, dp[i]);
        if (n < 0 || n > blen) {
            n = blen;
        }
        blen -= n;
        bp += n;
        sep = " ";
    }
    return buf;
}
