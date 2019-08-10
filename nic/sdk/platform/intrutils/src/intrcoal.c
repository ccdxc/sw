/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"

#include "cap_top_csr_defines.h"
#include "cap_intr_c_hdr.h"

#include "intrutils.h"

#define INTR_BASE               CAP_ADDR_BASE_INTR_INTR_OFFSET

#define INTR_COALESCE_OFFSET    CAP_INTR_CSR_CFG_INTR_COALESCE_BYTE_OFFSET
#define INTR_COALESCE_BASE      (INTR_BASE + INTR_COALESCE_OFFSET)

void
intr_coal_set_resolution(const int ticks)
{
    const u_int64_t pa = INTR_COALESCE_BASE;

    pal_reg_wr32(pa, ticks);
}

int
intr_coal_get_resolution(void)
{
    const u_int64_t pa = INTR_COALESCE_BASE;

    return pal_reg_rd32(pa);
}

int
intr_coal_get_params(int *mul, int *div)
{
    const int res = intr_coal_get_resolution();
    int r = 0;

    switch (res) {
    case 83: /* 10us on fpga */
        if (!pal_is_asic()) {
            /* fpga is 1/100 speed */
            *mul = 1;
            *div = 10;
        } else {
            *mul = 0;
            *div = 1;
            r = -1;
        }
        break;
    case 1250: /* 1.5us */
        *mul = 2;
        *div = 3;
        break;
    case 2500: /* 3us */
        *mul = 1;
        *div = 3;
        break;
    case 8300: /* 10us */
        *mul = 1;
        *div = 10;
        break;
    default:
        /*
         * mul=0 effectively disables intr coalescing for this unknown config.
         * div=1 avoids divide-by-0.
         */
        *mul = 0;
        *div = 1;
        r = -1;
        break;
    }
    return r;
}
