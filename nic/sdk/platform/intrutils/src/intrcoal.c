/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "intrutilspd.h"

#define INTR_BASE               ASIC_(ADDR_BASE_INTR_INTR_OFFSET)

#define INTR_COALESCE_OFFSET    ASIC_(INTR_CSR_CFG_INTR_COALESCE_BYTE_OFFSET)
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

/*
 * The driver wants to convert from microseconds units of interrupt
 * coalescing into hw "tick" units for the INTR.coal_init register.
 *
 * The INTR.coal_init register of an interrupt resource counts "ticks"
 * before the next interrupt.  A "tick" duration is configured by the
 * the CFG_INTR_COALESCE register which specifies the number of core
 * clock cycles for each "tick".  The core clock is (usually) 833 MHz
 * on Capri.
 *
 * We need to determine the ratio of "ticks" to "usecs" and then select
 * a multiplier and divisor to express that ratio.
 *
 * ================
 * CFG_INTR_COALESCE
 * ----------------
 *
 * Often we work backwards, first selecting a tick resolution in usecs
 * leading to a CFG_INTR_COALESCE value to set.  For example, say we want
 * a desired resolution is 3us per tick so INTR.coal_init=1 is 3us,
 * INTR.coal_init=2 is 6us, etc.
 *
 *     Cpu clock rate is 833333333 MHz.
 *     One cpu clock cycle is 1/833333333s, or 1/833.333333us.
 *     For resolution R,   ticks = R * (1/833.333333).
 *     For resolution 3us, ticks = 3 * (1/833.333333) = 2500.
 *
 * ================
 * mul, div
 * ----------------
 *
 * For the driver to convert from a coalescing time in microseconds
 * to ticks in resolution R microseconds, the ratio of microseconds
 * to ticks is 1:R, or 1/R.  For R=3us, ratio is 1/3 so we want to
 * select mul=1 and div=3 to express the ratio 1/3.
 *
 * We want mul and div to be integers to allow for integer math
 * (some OS's don't support floating point math in kernel mode).
 * For R=1.5us, the ratio is 1/1.5, or 2/3 to allow for integer math.
 */
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
