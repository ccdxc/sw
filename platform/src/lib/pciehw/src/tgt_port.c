/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "pal.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "pciehsys.h"

#define TGT_PORT_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_TGT_PORT_BYTE_ADDRESS)
#define TGT_PORT_COUNT \
    CAP_PXB_CSR_CFG_TGT_PORT_ARRAY_COUNT
#define TGT_PORT_STRIDE \
    (CAP_PXB_CSR_CFG_TGT_PORT_ARRAY_ELEMENT_SIZE * 4)

typedef union {
    struct {
        u_int32_t id_limit:8;
        u_int32_t bus_adjust:1;
        u_int32_t halt:1;
        u_int32_t single_pnd:1;
        u_int32_t td_mis:1;
        u_int32_t depth_thres:4;
        u_int32_t skip_notify_if_qfull:1;
        u_int32_t fbe_holes_allow:1;
        u_int32_t fence_dis:1;
    } __attribute__((packed));
    u_int32_t all32;
} tgt_portcfg_t;

static u_int64_t
tgt_port_addr(const int port)
{
    return TGT_PORT_BASE + (port * TGT_PORT_STRIDE);
}

void
pciehw_tgt_port_skip_notify(const int port, const int on)
{
    const u_int64_t pa = tgt_port_addr(port);
    tgt_portcfg_t cfg;

    cfg.all32 = pal_reg_rd32(pa);
    cfg.skip_notify_if_qfull = on;
    pal_reg_wr32(pa, cfg.all32);
}

void
pciehw_tgt_port_init(pciehw_t *phw)
{
}
