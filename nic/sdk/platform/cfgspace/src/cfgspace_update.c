/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/cfgspace/include/cfgspace.h"

/*
 * The pcie capability contains a "link status" section
 * that reflects the current negotiated link speed and width.
 * Update those fields here with the current link gen/width.
 */
static void
updatecap_pcie(cfgspace_t *cs, const u_int8_t gen, const u_int8_t width)
{
    u_int16_t pcie_cap, lnksta;

    pcie_cap = cfgspace_findcap(cs, 0x10);
    if (pcie_cap == 0) return;

    /*
     * Read the Link Status register, clear old gen/width,
     * then update with current gen/width.
     */
    lnksta = cfgspace_getw(cs, pcie_cap + 0x12);
    lnksta &= ~0x3ff;
    lnksta |= ((width & 0x3f) << 4) | (gen & 0xf);
    cfgspace_setw(cs, pcie_cap + 0x12, lnksta);
}

void
cfgspace_update(cfgspace_t *cs, const u_int8_t gen, const u_int8_t width)
{
    updatecap_pcie(cs, gen, width);
}
