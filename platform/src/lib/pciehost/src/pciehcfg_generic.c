/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <sys/types.h>

#include "pciehcfg.h"
#include "cfgspace.h"
#include "cfgspace_getset.h"

/*
 * Config space reads.  These are just wrappers around the
 * accessor functions to read the current value of config space.
 */

static u_int8_t
pciehcfg_generic_readb(pciehcfg_t *pcfg, const u_int16_t offset)
{
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    return cfgspace_getb(&cs, offset);
}

static u_int16_t
pciehcfg_generic_readw(pciehcfg_t *pcfg, const u_int16_t offset)
{
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    return cfgspace_getw(&cs, offset);
}

static u_int32_t
pciehcfg_generic_readd(pciehcfg_t *pcfg, const u_int16_t offset)
{
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    return cfgspace_getd(&cs, offset);
}

/*
 * Config space writes.  Normally config space is initialized
 * with the pciehcfg_set* functions.  Once initialized, write
 * accesses come through these APIs where we implement the write-mask
 * fields (initialized with pciehcfg_set[bwd]m() functions).
 * A bit set in the write-mask indicates that bit is writeable
 * by these operations.  Read-only fields in the current value are
 * merged with writeable fields from the new written value and the
 * current contents of config space are replaced with the result.
 */

static void
pciehcfg_generic_writeb(pciehcfg_t *pcfg, u_int16_t offset, u_int8_t val)
{
    u_int8_t oval, nval, wmsk;
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    oval = cfgspace_getb_cur(&cs, offset);
    wmsk = cfgspace_getb_msk(&cs, offset);
    nval = (oval & ~wmsk) | (val & wmsk);
    cfgspace_setb_cur(&cs, offset, nval);
}

static void
pciehcfg_generic_writew(pciehcfg_t *pcfg, u_int16_t offset, u_int16_t val)
{
    u_int16_t oval, nval, wmsk;
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    oval = cfgspace_getw_cur(&cs, offset);
    wmsk = cfgspace_getw_msk(&cs, offset);
    nval = (oval & ~wmsk) | (val & wmsk);
    cfgspace_setw_cur(&cs, offset, nval);
}

static void
pciehcfg_generic_writed(pciehcfg_t *pcfg, u_int16_t offset, u_int32_t val)
{
    u_int32_t oval, nval, wmsk;
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    oval = cfgspace_getd_cur(&cs, offset);
    wmsk = cfgspace_getd_msk(&cs, offset);
    nval = (oval & ~wmsk) | (val & wmsk);
    cfgspace_setd_cur(&cs, offset, nval);
}

int
pciehcfg_generic_read(pciehcfg_t *pcfg,
                      const u_int16_t offset,
                      const u_int8_t size,
                      u_int32_t *valp)
{
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    return cfgspace_read(&cs, offset, size, valp);
}

int
pciehcfg_generic_write(pciehcfg_t *pcfg,
                      const u_int16_t offset,
                      const u_int8_t size,
                      const u_int32_t val)
{
    cfgspace_t cs;

    pciehcfg_get_cfgspace(pcfg, &cs);
    return cfgspace_write(&cs, offset, size, valp);
}
