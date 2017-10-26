/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "pciehsys.h"

void *
pciehsys_zalloc(const size_t size)
{
    void *p = calloc(1, size);
    assert(p != NULL);
    return p;
}

void
pciehsys_free(void *p)
{
    free(p);
}

void *
pciehsys_realloc(void *p, const size_t size)
{
    p = realloc(p, size);
    assert(p != NULL);
    return p;
}

void
pciehsys_log(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void pciehsys_error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
