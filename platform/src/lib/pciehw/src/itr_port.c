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

#define ITR_PORT_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_ITR_PORT_BYTE_ADDRESS)
#define ITR_PORT_COUNT \
    CAP_PXB_CSR_CFG_ITR_PORT_ARRAY_COUNT
#define ITR_PORT_STRIDE \
    (CAP_PXB_CSR_CFG_ITR_PORT_ARRAY_ELEMENT_SIZE * 4)

typedef union {
    struct {
        u_int64_t rdreq_limit:8;
        u_int64_t timer_tick:16;
        u_int64_t timer_limit:10;
        u_int64_t bus_adjust:1;
        u_int64_t arb_cr_hdr:5;
        u_int64_t arb_cr_data:6;
        u_int64_t arb_cr_dis_wr:1;
    } __attribute__((packed));
    u_int64_t all64;
} itr_portcfg_t;

static u_int64_t
itr_port_addr(const int port)
{
    return ITR_PORT_BASE + (port * ITR_PORT_STRIDE);
}

static u_int32_t
pciehw_itr_port_set_rdreq_limit(const int port, const u_int32_t limit)
{
    const u_int64_t pa = itr_port_addr(port);
    itr_portcfg_t cfg;
    u_int32_t olimit;

    cfg.all64 = pal_reg_rd64(pa);
    olimit = cfg.rdreq_limit;
    cfg.rdreq_limit = limit;
    pal_reg_wr64(pa, cfg.all64);
    return olimit;
}

void
pciehw_itr_port_init(pciehw_t *phw)
{
    int port;

    for (port = 0; port < phw->nports; port++) {
        if (pciehw_port_is_enabled(port)) {
            pciehw_itr_port_set_rdreq_limit(port, 0x10);
        }
    }
}
