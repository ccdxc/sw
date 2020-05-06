/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "intrutilspd.h"

#define NWORDS(a)               (sizeof(a) / sizeof(u_int32_t))

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

static void
intr_cfg_coalesce_resolution(const int res)
{
    const uint64_t pa =
        (INTR_BASE + CAP_INTR_CSR_CFG_INTR_COALESCE_BYTE_ADDRESS);

    pal_reg_wr32(pa, res);
}

void
intrpd_hwinit(void)
{
    intr_cfg_legacy_intx();
    if (!pal_is_asic()) {
        intr_cfg_coalesce_resolution(83);
    } else {
        /* set 3.0us resolution */
        intr_coal_set_resolution(2500);
    }
}
