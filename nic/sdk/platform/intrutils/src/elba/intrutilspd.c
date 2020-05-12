/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/param.h>

#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "intrutilspd.h"

#define INTR_PBA_CFG_OFFSET     ASIC_(INTR_CSR_DHS_INTR_PBA_CFG_BYTE_OFFSET)
#define INTR_PBA_CFG_BASE       (INTR_BASE + INTR_PBA_CFG_OFFSET)
#define INTR_PBA_CFG_STRIDE     0x4

static u_int64_t
intr_pba_cfg_addr(const int lif)
{
    return INTR_PBA_CFG_BASE + (lif * INTR_PBA_CFG_STRIDE);
}

void
intr_pba_cfg(const int lif, const int intrb, const size_t intrc)
{
    u_int64_t pa = intr_pba_cfg_addr(lif);
    union {
        struct {
            u_int32_t start:13;
            u_int32_t count:7;
            u_int32_t ecc:6;
            u_int32_t pad:6;
        } __attribute__((packed));
        u_int32_t w[1];
    } __attribute__((packed)) v = {
        .start = intrb,
        .count = MIN(intrc, 128) - 1,
    };

    pal_reg_wr32(pa, v.w[0]);
}

void
intrpd_hwinit(const u_int32_t clock_freq)
{
    intrpd_coal_init(clock_freq);
}
