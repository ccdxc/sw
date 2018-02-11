/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#include "pciehost.h"
#include "pciehw.h"
#include "pciehsvc.h"
#include "pciehsvc_impl.h"

static pciehsvc_t pciehsvc;

static pciehsvc_t *
pciehsvc_get(void)
{
    return &pciehsvc;
}

int
pciehsvc_open(pciehsvc_params_t *svcparams)
{
    pciehsvc_t *psvc = pciehsvc_get();

    if (svcparams) {
        psvc->svcparams = *svcparams;
    }
    return pciehw_open(NULL);
}

void
pciehsvc_close(void)
{
    pciehw_close();
}

int
pciehsvc_cfgrd(const u_int16_t bdf,
               const u_int16_t offset, const u_int8_t size, u_int32_t *valp)
{
    return pciehw_cfgrd(bdf, offset, size, valp);
}

int
pciehsvc_cfgwr(const u_int16_t bdf,
               const u_int16_t offset, const u_int8_t size, u_int32_t val)
{
    return pciehw_cfgwr(bdf, offset, size, val);
}

int
pciehsvc_memrd(const u_int64_t addr, const u_int8_t size, u_int64_t *valp)
{
    return pciehw_memrd(addr, size, valp);
}

int
pciehsvc_memwr(const u_int64_t addr, const u_int8_t size, u_int64_t val)
{
    return pciehw_memwr(addr, size, val);
}

int
pciehsvc_iord(const u_int32_t addr, const u_int8_t size, u_int32_t *valp)
{
    return pciehw_iord(addr, size, valp);
}

int
pciehsvc_iowr(const u_int32_t addr, const u_int8_t size, u_int32_t val)
{
    return pciehw_iowr(addr, size, val);
}

int
pciehsvc_barsz(const u_int32_t bdf, const int i)
{
    return pciehw_barsz(bdf, i);
}

pciehsvc_params_t *
pciehsvc_get_params(void)
{
    pciehsvc_t *psvc = pciehsvc_get();
    return &psvc->svcparams;
}
