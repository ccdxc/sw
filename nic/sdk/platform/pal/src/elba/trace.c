/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "pal.h"
#include "internal.h"

static void
pr_trinit(pal_data_t *pd)
{
    const char *file = getenv("PAL_TRACE");

    if (file != NULL) {
        pd->trfp = fopen(file, "w");
        if (pd->trfp != NULL) {
            /* initial setting from $PAL_TRACE_ON else on */
            const char *trace_on = getenv("PAL_TRACE_ON");
            const int on = trace_on ? atoi(trace_on) : 1;
            pd->reg_trace_en = on;
            pd->mem_trace_en = on;
        }
    }
    pd->trace_init = 1;
}

void
pal_reg_trace(const char *fmt, ...)
{
    pal_data_t *pd = pal_get_data();
    va_list ap;

    if (!pd->trace_init) pr_trinit(pd);
    if (!pd->reg_trace_en) return;

    va_start(ap, fmt);
    vfprintf(pd->trfp, fmt, ap);
    va_end(ap);
    fflush(pd->trfp);
}

void
pal_mem_trace(const char *fmt, ...)
{
    pal_data_t *pd = pal_get_data();
    va_list ap;

    if (!pd->trace_init) pr_trinit(pd);
    if (!pd->mem_trace_en) return;

    va_start(ap, fmt);
    vfprintf(pd->trfp, fmt, ap);
    va_end(ap);
    fflush(pd->trfp);
}

int
pal_reg_trace_control(const int on)
{
    pal_data_t *pd = pal_get_data();
    int was_on;

    if (!pd->trace_init) pr_trinit(pd);
    was_on = pd->reg_trace_en;
    if (pd->trfp) {
        pd->reg_trace_en = on;
        if (was_on) fflush(pd->trfp);
    }
    return was_on;
}

int
pal_mem_trace_control(const int on)
{
    pal_data_t *pd = pal_get_data();
    int was_on;

    if (!pd->trace_init) pr_trinit(pd);
    was_on = pd->mem_trace_en;
    if (pd->trfp) {
        pd->mem_trace_en = on;
        if (was_on) fflush(pd->trfp);
    }
    return was_on;
}

