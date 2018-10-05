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
#include "pal_mm.h"
#include "pal_impl.h"

static pal_data_t pal_data;

void pal_init(char *application_name) {
   pal_mm_init(application_name);
}

pal_data_t *
pal_get_data(void)
{
    pal_data_t *pd = &pal_data;
    if (!pd->memopen) {
#ifdef __aarch64__
        /* Making all accesses to /dev/mem UNCACHED for now */
        pd->memfd = open("/dev/mem", O_RDWR | O_SYNC);
        assert(pd->memfd >= 0);
#endif
	pd->regions = NULL;
        pd->memopen = 1;
    }
    return pd;
}

int
pal_get_env(void)
{
#ifdef __aarch64__
#define MS_STA_VER \
    (CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_STA_VER_BYTE_ADDRESS)

    union {
        struct {
            u_int32_t chip_type:4;
            u_int32_t chip_version:12;
            u_int32_t chip_build:16;
        } __attribute__((packed));
        u_int32_t w;
    } reg;

    reg.w = pal_reg_rd32(MS_STA_VER);
    return reg.chip_type;
#else
    const char *env = getenv("PAL_ENV");
    if (env) {
        return strtoul(env, NULL, 0);
    }
    return PAL_ENV_HAPS; /* for now x86_64 emulates HAPS */
#endif
}

int
pal_is_asic(void)
{
    return pal_get_env() == PAL_ENV_ASIC;
}
