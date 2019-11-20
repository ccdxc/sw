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
    u_int32_t lnkcap;
    u_int16_t pcie_cap, lnksta;
    u_int8_t cap_gen, cap_width;
    u_int8_t cur_gen, cur_width;

    pcie_cap = cfgspace_findcap(cs, 0x10);
    if (pcie_cap == 0) return;

    /*
     * Read the Link Status register, clear old gen/width,
     * then update with current gen/width.
     */

    lnkcap = cfgspace_getd(cs, pcie_cap + 0xc);
    cap_gen = lnkcap & 0xf;
    cap_width = (lnkcap >> 4) & 0x3f;

    /* clamp params to advertised capability */
    cur_gen = MIN(gen, cap_gen);
    cur_width = MIN(width, cap_width);

    lnksta = cfgspace_getw(cs, pcie_cap + 0x12);
    lnksta &= ~0x3ff;
    lnksta |= ((cur_width & 0x3f) << 4) | (cur_gen & 0xf);
    cfgspace_setw(cs, pcie_cap + 0x12, lnksta);
}

void
cfgspace_update(cfgspace_t *cs, const u_int8_t gen, const u_int8_t width)
{
    updatecap_pcie(cs, gen, width);
}
