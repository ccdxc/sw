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

#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

#define TGT_PORT_BASE   PXB_(CFG_TGT_PORT)
#define TGT_PORT_COUNT  ASIC_(PXB_CSR_CFG_TGT_PORT_ARRAY_COUNT)
#define TGT_PORT_STRIDE (ASIC_(PXB_CSR_CFG_TGT_PORT_ARRAY_ELEMENT_SIZE) * 4)

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
pciehw_tgt_port_single_pnd(const int port, const int on)
{
    const u_int64_t pa = tgt_port_addr(port);
    tgt_portcfg_t cfg;

    cfg.all32 = pal_reg_rd32(pa);
    cfg.single_pnd = on;
    pal_reg_wr32(pa, cfg.all32);
}

void
pciehw_tgt_port_init(void)
{
    const pciemgr_params_t *params = pciehw_get_params();
    int port;

    for (port = 0; port < PCIEHW_NPORTS; port++) {
        pciehw_tgt_port_single_pnd(port, params->single_pnd);
    }
}
