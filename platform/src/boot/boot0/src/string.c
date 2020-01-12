
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#pragma GCC optimize("Os")

#include "dtls.h"

int
strcmp(const char *a, const char *b)
{
    int r;

    while (1) {
	if ((r = *a - *b++) != 0 || !*a++) {
	    break;
        }
    }
    return r;
}

size_t
strlen(const char *s)
{
    const char *e;
    for (e = s; *e; e++) {
        ;
    }
    return (size_t)(e - s);
}

char *
strchr(const char *s, int c)
{
    while (*s && *s != c)
        ++s;
    return *s ? (char *)s : NULL;
}

void *
memcpy(void *__restrict dst, const void *__restrict src, size_t len)
{
    char *dp = dst;
    const char *sp = src;
    while (len--) {
        *dp++ = *sp++;
    }
    return dst;
}
