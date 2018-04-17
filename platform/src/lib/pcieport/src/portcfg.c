/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

#include "cap_top_csr_defines.h"
#include "cap_pp_c_hdr.h"

#include "pal.h"
#include "portcfg.h"

#ifndef PXC_
#define PXC_(REG, pn) \
    (CAP_ADDR_BASE_PP_PP_OFFSET + \
     ((pn) * CAP_PXC_CSR_BYTE_SIZE) + \
     CAP_PP_CSR_PORT_C_ ##REG## _BYTE_ADDRESS)
#endif

typedef union {
    u_int32_t d;
    u_int16_t w[2];
    u_int8_t  b[4];
} cfgdata_t;

static u_int32_t
portcfg_readdw(const int port, const u_int16_t addr)
{
    assert(addr < 4096);
    return pal_reg_rd32(PXC_(DHS_C_MAC_APB_ENTRY, port) + addr);
}

static void
portcfg_writedw(const int port, const u_int16_t addr, u_int32_t val)
{
    assert(addr < 4096);
    pal_reg_wr32(PXC_(DHS_C_MAC_APB_ENTRY, port) + addr, val);
}

u_int8_t
portcfg_readb(const int port, const u_int16_t addr)
{
    const u_int16_t addrdw = addr & ~0x3;
    const u_int8_t byteidx = addr & 0x3;
    cfgdata_t v;

    v.d = portcfg_readdw(port, addrdw);
    return v.b[byteidx];
}

u_int16_t
portcfg_readw(const int port, const u_int16_t addr)
{
    const u_int16_t addrdw = addr & ~0x3;
    const u_int8_t wordidx = (addr & 0x3) >> 1;
    cfgdata_t v;

    assert((addr & 0x1) == 0);
    v.d = portcfg_readdw(port, addrdw);
    return v.w[wordidx];
}

u_int32_t
portcfg_readd(const int port, const u_int16_t addr)
{
    assert((addr & 0x3) == 0);
    return portcfg_readdw(port, addr);
}

void
portcfg_writeb(const int port, const u_int16_t addr, const u_int8_t val)
{
    const u_int16_t addrdw = addr & ~0x3;
    const u_int8_t byteidx = addr & 0x3;
    cfgdata_t v;

    v.d = portcfg_readdw(port, addrdw);
    v.b[byteidx] = val;
    portcfg_writedw(port, addrdw, v.d);
}

void
portcfg_writew(const int port, const u_int16_t addr, const u_int16_t val)
{
    const u_int16_t addrdw = addr & ~0x3;
    const u_int8_t wordidx = (addr & 0x3) >> 1;
    cfgdata_t v;

    assert((addr & 0x1) == 0);
    v.d = portcfg_readdw(port, addrdw);
    v.w[wordidx] = val;
    portcfg_writedw(port, addrdw, v.d);
}

void
portcfg_writed(const int port, const u_int16_t addr, const u_int32_t val)
{
    assert((addr & 0x3) == 0);
    portcfg_writedw(port, addr, val);
}
