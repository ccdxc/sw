/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"
#include "platform/pcieport/include/pcieport.h"
#include "portcfgpd.h"
#include "pcieportpd.h"

static u_int64_t
portcfgpd_base(const int port)
{
    return PXC_(DHS_C_MAC_APB_ENTRY, port);
}

u_int32_t
portcfgpd_readdw(const int port, const u_int16_t addr)
{
    assert(addr < 4096);
    return pal_reg_rd32(portcfgpd_base(port) + addr);
}

void
portcfgpd_writedw(const int port, const u_int16_t addr, u_int32_t val)
{
    assert(addr < 4096);
    pal_reg_wr32(portcfgpd_base(port) + addr, val);
}
