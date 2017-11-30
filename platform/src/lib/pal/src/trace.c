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
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>

#include "pal.h"
#include "pal_impl.h"

static void
pr_trinit(pal_data_t *pd)
{
    const char *file = getenv("PAL_REG_TRACE");

    if (file != NULL) {
        pd->trfp = fopen(file, "w");
        if (pd->trfp != NULL) {
            pd->trace_en = 1;
        }
    }
    pd->trace_init = 1;
}

void
pal_trace(const char *fmt, ...)
{
    pal_data_t *pd = pal_get_data();
    va_list ap;

    if (!pd->trace_init) pr_trinit(pd);
    if (!pd->trace_en) return;

    va_start(ap, fmt);
    vfprintf(pd->trfp, fmt, ap);
    va_end(ap);
}

int
pal_reg_trace_control(const int on)
{
    pal_data_t *pd = pal_get_data();
    int was_on;

    if (!pd->trace_init) pr_trinit(pd);
    was_on = pd->trace_en;
    pd->trace_en = on;
    if (was_on && !pd->trace_en) fflush(pd->trfp);
    return was_on;
}

int
pal_mem_trace_control(const int on)
{
    pal_data_t *pd = pal_get_data();
    int was_on;

    if (!pd->trace_init) pr_trinit(pd);
    was_on = pd->trace_en;
    pd->trace_en = on;
    if (was_on && pd->trace_en) fflush(pd->trfp);
    return was_on;
}
