/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdlib.h>

#include "misc.h"

/*
 * Like strtoul, but interpret optional extension:
 * k, m, g for kilobytes, megabytes, gigabytes respectively.
 */

unsigned long long
strtoull_ext(const char *s)
{
    char *ep;
    unsigned long long n;

    ep = NULL;
    n = strtoull(s, &ep, 0);
    if (ep != NULL) {
        switch (*ep) {
        case 'k': n *= 1024; break;
        case 'm': n *= 1024 * 1024; break;
        case 'g': n *= 1024 * 1024 * 1024; break;
        }
    }
    return n;
}
