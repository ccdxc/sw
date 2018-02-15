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

#define PORT_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_TGT_PORT_BYTE_ADDRESS)
#define PORT_COUNT \
    CAP_PXB_CSR_CFG_TGT_PORT_ARRAY_COUNT
#define PORT_STRIDE \
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
    u_int32_t w;
} portcfg_t;


static u_int64_t
port_addr(const int port)
{
    return PORT_BASE + (port * PORT_STRIDE);
}

void
pciehw_port_skip_notify(const int port, const int on)
{
    portcfg_t cfg;

    cfg.w = pal_reg_rd32(port_addr(port));
    cfg.skip_notify_if_qfull = on;
    pal_reg_wr32(port_addr(port), cfg.w);
}

void
pciehw_port_init(pciehw_t *phw)
{
}
