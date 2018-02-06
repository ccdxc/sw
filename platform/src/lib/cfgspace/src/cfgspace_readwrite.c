/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <assert.h>
#include <sys/types.h>

#include "cfgspace.h"
#include "cfgspace_getset.h"

u_int8_t
cfgspace_readb(cfgspace_t *cs, const u_int16_t offset)
{
    return cfgspace_getb(cs, offset);
}

u_int16_t
cfgspace_readw(cfgspace_t *cs, const u_int16_t offset)
{
    return cfgspace_getw(cs, offset);
}

u_int32_t
cfgspace_readd(cfgspace_t *cs, const u_int16_t offset)
{
    return cfgspace_getd(cs, offset);
}

int
cfgspace_read(cfgspace_t *cs,
              const u_int16_t offset,
              const u_int8_t size,
              u_int32_t *valp)
{
    switch (size) {
    case 1: *valp = cfgspace_getb(cs, offset); break;
    case 2: *valp = cfgspace_getw(cs, offset); break;
    case 4: *valp = cfgspace_getd(cs, offset); break;
    default:
        return -1;
    }
    return 0;
}

/*
 * Config space writes.  Normally config space is initialized
 * with the cfgspace_set* functions.  Once initialized, write
 * accesses come through these APIs where we implement the write-mask
 * fields (initialized with cfgspace_set[bwd]m() functions).
 * A bit set in the write-mask indicates that bit is writeable
 * by these operations.  Read-only fields in the current value are
 * merged with writeable fields from the new written value and the
 * current contents of config space are replaced with the result.
 */

void
cfgspace_writeb(cfgspace_t *cs, const u_int16_t offset, const u_int8_t val)
{
    const u_int8_t oval = cfgspace_getb_cur(cs, offset);
    const u_int8_t wmsk = cfgspace_getb_msk(cs, offset);
    const u_int8_t nval = (oval & ~wmsk) | (val & wmsk);
    cfgspace_setb_cur(cs, offset, nval);
}

void
cfgspace_writew(cfgspace_t *cs, const u_int16_t offset, const u_int16_t val)
{
    const u_int16_t oval = cfgspace_getw_cur(cs, offset);
    const u_int16_t wmsk = cfgspace_getw_msk(cs, offset);
    const u_int16_t nval = (oval & ~wmsk) | (val & wmsk);
    cfgspace_setw_cur(cs, offset, nval);
}

void
cfgspace_writed(cfgspace_t *cs, const u_int16_t offset, const u_int32_t val)
{
    const u_int32_t oval = cfgspace_getd_cur(cs, offset);
    const u_int32_t wmsk = cfgspace_getd_msk(cs, offset);
    const u_int32_t nval = (oval & ~wmsk) | (val & wmsk);
    cfgspace_setd_cur(cs, offset, nval);
}

int
cfgspace_write(cfgspace_t *cs,
               const u_int16_t offset,
               const u_int8_t size,
               const u_int32_t val)
{
    switch (size) {
    case 1: cfgspace_writeb(cs, offset, val); break;
    case 2: cfgspace_writew(cs, offset, val); break;
    case 4: cfgspace_writed(cs, offset, val); break;
    default:
        return -1;
    }
    return 0;
}
