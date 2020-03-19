/*
 * Copyright (c) 2017,2020, Pensando Systems Inc.
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
updatecap_pcie(cfgspace_t *cs,
               const u_int8_t gen, const u_int8_t width,
               const u_int16_t lnksta2)
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

    /*
     * Link Status 2
     * If link up with speed >= Gen3, indicate equalization complete.
     */
    if (cur_gen == 3) {
        const u_int16_t eqflags =
            ((1 << 1) | /* Equalization 8.0 GT/s complete */
             (1 << 2) | /* Equalization 8.0 GT/s Phase 1 successful */
             (1 << 3) | /* Equalization 8.0 GT/s Phase 2 successful */
             (1 << 4)); /* Equalization 8.0 GT/s Phase 3 successful */
        u_int16_t llnksta2 = cfgspace_getw(cs, pcie_cap + 0x32);
        llnksta2 &= ~eqflags;
        llnksta2 |= lnksta2 & eqflags;
        cfgspace_setw(cs, pcie_cap + 0x32, llnksta2);
    }
}

/*
 * Indicate equalization completed for gen4 16.0 GT/s links.
 */
static void
updatecap_physlayer(cfgspace_t *cs,
                    const u_int8_t gen, const u_int8_t width,
                    const u_int32_t phylsta)
{
    const u_int32_t eqflags =
        ((1 << 0) | /* Equalization 16.0 GT/s complete */
         (1 << 1) | /* Equalization 16.0 GT/s Phase 1 successful */
         (1 << 2) | /* Equalization 16.0 GT/s Phase 2 successful */
         (1 << 3)); /* Equalization 16.0 GT/s Phase 3 successful */
    u_int16_t phys_cap;
    u_int32_t v;

    /* if physical link != gen4 then nothing to do */
    if (gen != 4) return;

    phys_cap = cfgspace_findcap(cs, 0x26);
    if (phys_cap == 0) return;

    v = cfgspace_getd(cs, phys_cap + 0xc);
    v &= ~eqflags;
    v |= phylsta & eqflags;
    cfgspace_setd(cs, phys_cap + 0xc, v);
}

void
cfgspace_update(cfgspace_t *cs,
                const u_int8_t gen, const u_int8_t width,
                const u_int16_t lnksta2)
{
    updatecap_pcie(cs, gen, width, lnksta2);
    updatecap_physlayer(cs, gen, width, 0/* XXX TODO GEN4 */);
}
