/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <sys/types.h>
#include "misc.h"

char *
hex_format(char *buf, size_t buflen,
           const unsigned char *data, const size_t datalen)
{
    size_t blen = buflen;
    char *bp = buf;
    char *sep = "";
    int i, n;

    for (i = 0; blen > 0 && i < datalen; i++) {
        n = snprintf(bp, blen, "%s%02x", sep, data[i]);
        if (n < 0 || n > blen) {
            n = blen;
        }
        blen -= n;
        bp += n;
        sep = " ";
    }
    return buf;
}
