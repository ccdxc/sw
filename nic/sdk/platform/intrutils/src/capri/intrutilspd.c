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

#define NWORDS(a)               (sizeof(a) / sizeof(u_int32_t))

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
            u_int32_t start:12;
            u_int32_t count:6;
            u_int32_t ecc:6;
            u_int32_t pad:8;
        } __attribute__((packed));
        u_int32_t w[1];
    } __attribute__((packed)) v = {
        .start = intrb,
        .count = MIN(intrc, 64) - 1,
    };

    pal_reg_wr32(pa, v.w[0]);
}

/*
 * Default hw config for INTX message needs adjustment
 * for correct operation.
 */
static void
intr_cfg_legacy_intx(void)
{
    const uint64_t pa =
        (INTR_BASE + CAP_INTR_CSR_CFG_LEGACY_INTX_PCIE_MSG_HDR_BYTE_ADDRESS);
    uint32_t w[4];

    pal_reg_rd32w(pa, w, NWORDS(w));
    w[0] = 0x34;
    pal_reg_wr32w(pa, w, NWORDS(w));
}

void
intrpd_hwinit(const u_int32_t clock_freq)
{
    intr_cfg_legacy_intx();
    intrpd_coal_init(clock_freq);
}
