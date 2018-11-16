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

#include "platform/src/lib/pal/include/pal.h"
#include "platform/src/lib/pciemgrutils/include/pciesys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

static void
pcieport_rx_credit_init(const int nports)
{
    int base, ncredits, i;

    if (pal_is_asic()) {
        assert(nports == 1); /* XXX tailored for 1 active port */
        /* port 0 gets all credits for now */
        pcieport_rx_credit_bfr(0, 0, 1023);
    } else {
        assert(nports == 4); /* XXX tailored for 4 active ports */
        ncredits = 1024 / nports;
        for (base = 0, i = 0; i < 8; i += 2, base += ncredits) {
            const int limit = base + ncredits - 1;

            pcieport_rx_credit_bfr(i, base, limit);
            pcieport_rx_credit_bfr(i + 1, 0, 0);
        }
    }
}

static void
pcieport_macfifo_thres(const int thres)
{
    union {
        struct {
            u_int32_t macfifo_thres:5;
            u_int32_t rd_sgl_pnd:1;
            u_int32_t tag_avl_guardband:3;
            u_int32_t cnxt_avl_guardband:3;
        } __attribute__((packed));
        u_int32_t w;
    } v;
    const u_int64_t itr_tx_req =
        (CAP_ADDR_BASE_PXB_PXB_OFFSET +
         CAP_PXB_CSR_CFG_ITR_TX_REQ_BYTE_ADDRESS);

    v.w = pal_reg_rd32(itr_tx_req);
    v.macfifo_thres = thres;
    pal_reg_wr32(itr_tx_req, v.w);
}

static void
pcieport_link_init_asic(void)
{
    pal_reg_wr32(PP_(CFG_PP_LINKWIDTH), 0x0); /* 1 port x16 linkwidth mode */
    pcieport_rx_credit_init(1);
    pcieport_macfifo_thres(5); /* match late-stage ECO */
}

static void
pcieport_link_init_haps(void)
{
    pal_reg_wr32(PP_(CFG_PP_LINKWIDTH), 0x2222); /* 4 port x4 linkwidth mode */
    pcieport_rx_credit_init(4);
    pcieport_macfifo_thres(5); /* match late-stage ECO */
}

static void
pcieport_link_init(void)
{
    if (pal_is_asic()) {
        pcieport_link_init_asic();
    } else {
        pcieport_link_init_haps();
    }
}

int
pcieport_onetime_init(void)
{
    pcieport_info_t *pi = &pcieport_info;

    if (pi->init) {
        /* already initialized */
        return 0;
    }
    pcieport_link_init();
    pi->init = 1;
    return 0;
}
