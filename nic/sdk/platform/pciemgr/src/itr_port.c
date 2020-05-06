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

#define ITR_PORT_BASE   PXB_(CFG_ITR_PORT)
#define ITR_PORT_COUNT  ASIC_(PXB_CSR_CFG_ITR_PORT_ARRAY_COUNT)
#define ITR_PORT_STRIDE (ASIC_(PXB_CSR_CFG_ITR_PORT_ARRAY_ELEMENT_SIZE) * 4)

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

static void
itr_port_init(const int port,
              const u_int32_t limit,
              const u_int32_t tick)
{
    const u_int64_t cfg_itr_port = itr_port_addr(port);
    itr_portcfg_t cfg;

    cfg.all64 = pal_reg_rd64(cfg_itr_port);
    cfg.rdreq_limit = limit;
    cfg.timer_tick = tick;
    pal_reg_wr64(cfg_itr_port, cfg.all64);
}

void
pciehw_itr_port_init(void)
{
    int port;

    for (port = 0; port < PCIEHW_NPORTS; port++) {
        if (!pal_is_asic()) {
            /* reduce rdreq on haps */
            const u_int32_t limit = 0x10;
            /* timer_tick to 1/100 default on haps */
            const u_int32_t tick = 0x19e;

            itr_port_init(port, limit, tick);
        }
    }
}
