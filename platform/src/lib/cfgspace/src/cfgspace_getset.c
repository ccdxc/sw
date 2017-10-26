/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <sys/types.h>

#include "cfgspace.h"
#include "cfgspace_getset.h"

/*
 * Low-level config space initialization operations.
 */

u_int8_t
cfgspace_getb(cfgspace_t *cs, u_int16_t offset)
{
    if (offset < cfgspace_size(cs)) {
        return cfgspace_getb_cur(cs, offset);
    }
    /*
     * Any read between end of implementation and
     * end of PCIe Spec size returns 0's.
     */
    if (offset < 4096) {
        return 0;
    }
    /*
     * Shouldn't get any access beyond end of PCIe Spec size,
     * but if so, return all 0xff's.
     */
    return 0xff;
}

u_int16_t
cfgspace_getw(cfgspace_t *cs, u_int16_t offset)
{
    if (offset + 1 < cfgspace_size(cs)) {
        return cfgspace_getw_cur(cs, offset);
    }
    /*
     * Any read between end of implementation and
     * end of PCIe Spec size returns 0's.
     */
    if (offset + 1 < 4096) {
        return 0;
    }
    /*
     * Shouldn't get any access beyond end of PCIe Spec size,
     * but if so, return all 0xff's.
     */
    return 0xffff;
}

u_int32_t
cfgspace_getd(cfgspace_t *cs, u_int16_t offset)
{
    if (offset + 3 < cfgspace_size(cs)) {
        return cfgspace_getd_cur(cs, offset);
    }
    /*
     * Any read between end of implementation and
     * end of PCIe Spec size returns 0's.
     */
    if (offset + 3 < 4096) {
        return 0;
    }
    /*
     * Shouldn't get any access beyond end of PCIe Spec size,
     * but if so, return all 0xff's.
     */
    return 0xffffffff;
}

void
cfgspace_setb(cfgspace_t *cs, u_int16_t offset, u_int8_t val)
{
    if (offset < cfgspace_size(cs)) {
        cfgspace_setb_cur(cs, offset, val);
    }
}

void
cfgspace_setw(cfgspace_t *cs, u_int16_t offset, u_int16_t val)
{
    if (offset + 1 < cfgspace_size(cs)) {
        cfgspace_setw_cur(cs, offset, val);
    }
}

void
cfgspace_setd(cfgspace_t *cs, u_int16_t offset, u_int32_t val)
{
    if (offset + 3 < cfgspace_size(cs)) {
        cfgspace_setd_cur(cs, offset, val);
    }
}

void
cfgspace_setbm(cfgspace_t *cs, u_int16_t offset, u_int8_t val, u_int8_t msk)
{
    if (offset < cfgspace_size(cs)) {
        cfgspace_setb_cur(cs, offset, val);
        cfgspace_setb_msk(cs, offset, msk);
    }
}

void
cfgspace_setwm(cfgspace_t *cs, u_int16_t offset, u_int16_t val, u_int16_t msk)
{
    if (offset + 1 < cfgspace_size(cs)) {
        cfgspace_setw_cur(cs, offset, val);
        cfgspace_setw_msk(cs, offset, msk);
    }
}

void
cfgspace_setdm(cfgspace_t *cs, u_int16_t offset, u_int32_t val, u_int32_t msk)
{
    if (offset + 3 < cfgspace_size(cs)) {
        cfgspace_setd_cur(cs, offset, val);
        cfgspace_setd_msk(cs, offset, msk);
    }
}
