/*
 * Copyright (c) 2018-2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>
#include <linux/pci_regs.h>

#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/include/portcfg.h"
#include "portcfgpd.h"

typedef union {
    u_int32_t d;
    u_int16_t w[2];
    u_int8_t  b[4];
} cfgdata_t;

static u_int32_t
portcfg_readdw(const int port, const u_int16_t addr)
{
    return portcfgpd_readdw(port, addr);
}

static void
portcfg_writedw(const int port, const u_int16_t addr, u_int32_t val)
{
    portcfgpd_writedw(port, addr, val);
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

size_t
portcfg_read(const int port, const u_int16_t addr, void *buf, size_t count)
{
    const size_t requested_count = count;
    u_int16_t off = addr;
    char *cp;
    u_int32_t *wp;

    /* read unaligned head */
    cp = buf;
    while ((off & 0x3) && (count > 0)) {
        *cp = portcfg_readb(port, off);
        cp++;
        off++;
        count--;
    }

    /* read aligned dwords */
    wp = (u_int32_t *)cp;
    while (count >= 4) {
        *wp = portcfg_readd(port, off);
        wp++;
        off += 4;
        count -= 4;
    }

    /* read unaligned tail */
    cp = (char *)wp;
    while (count > 0) {
        *cp = portcfg_readb(port, off);
        cp++;
        off++;
        count--;
    }

    return requested_count;
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

void
portcfg_read_bus(const int port,
                 u_int8_t *pribus, u_int8_t *secbus, u_int8_t *subbus)
{
    cfgdata_t v;

    v.d = portcfg_readdw(port, PCI_PRIMARY_BUS);

    if (pribus) *pribus = v.b[0];
    if (secbus) *secbus = v.b[1];
    if (subbus) *subbus = v.b[2];
}

void
portcfg_read_genwidth(const int port, int *gen, int *width)
{
    /* pcie cap at 0x80, link status at +0x12 */
    const u_int16_t lnksta = portcfg_readw(port, PORTCFG_CAP_PCIE + 0x12);

    if (gen) *gen = lnksta & 0xf;
    if (width) *width = (lnksta >> 4) & 0x1f;
}
