/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>

#include "pciehw.h"
#include "pciemgrutils.h"
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
    return pciehw_cfgrd(0, bdf, offset, size, valp);
}

int
pciehsvc_cfgwr(const u_int16_t bdf,
               const u_int16_t offset, const u_int8_t size, u_int32_t val)
{
    return pciehw_cfgwr(0, bdf, offset, size, val);
}

int
pciehsvc_memrd(const u_int64_t addr, const u_int8_t size, u_int64_t *valp)
{
    return pciehw_memrd(0, addr, size, valp);
}

int
pciehsvc_memwr(const u_int64_t addr, const u_int8_t size, u_int64_t val)
{
    return pciehw_memwr(0, addr, size, val);
}

int
pciehsvc_iord(const u_int32_t addr, const u_int8_t size, u_int32_t *valp)
{
    return pciehw_iord(0, addr, size, valp);
}

int
pciehsvc_iowr(const u_int32_t addr, const u_int8_t size, u_int32_t val)
{
    return pciehw_iowr(0, addr, size, val);
}

unsigned long long
pciehsvc_barsz(const u_int32_t bdf, const int i)
{
    return pciehw_barsz(0, bdf, i);
}
