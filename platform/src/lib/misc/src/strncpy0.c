/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <string.h>
#include "misc.h"

char *
strncpy0(char *d, const char *s, const size_t n)
{
    strncpy(d, s, n);
    if (d && n && d[n - 1] != '\0') {
        d[n - 1] = '\0';
    }
    return d;
}
