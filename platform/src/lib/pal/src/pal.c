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

static pal_data_t pal_data;

pal_data_t *
pal_get_data(void)
{
    pal_data_t *pd = &pal_data;
    if (!pd->memopen) {
#ifdef __aarch64__
        pd->memfd = open("/dev/mem", O_RDWR);
        assert(pd->memfd >= 0);
#endif
        pd->memopen = 1;
    }
    return pd;
}
