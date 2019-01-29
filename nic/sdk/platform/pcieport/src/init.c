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

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
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

static int
pcieport_already_init(void)
{
    pcieport_info_t *pi = pcieport_info_get();
    pcieport_t *p = &pi->pcieport[0];

    /*
     * Check port 0 for ltssm_en=1 indicating someone
     * initialized the port already.
     */
    if (pcieport_get_ltssm_en(p)) {
        return 1;
    }
    return 0;
}

static u_int64_t
getenv_override_ull(const char *label, const char *name, const u_int64_t def)
{
    const char *env = getenv(name);
    if (env) {
        u_int64_t val = strtoull(env, NULL, 0);
        pciesys_loginfo("%s: $%s override %" PRIu64 " (0x%" PRIx64 ")\n",
                        label, name, val, val);
        return val;
    }
    return def;
}

static u_int64_t
pcieport_param_ull(const char *name, const u_int64_t def)
{
    return getenv_override_ull("pcieport", name, def);
}

int
pcieport_onetime_init(void)
{
    pcieport_info_t *pi = pcieport_info_get();

    if (pi->init) {
        /* already initialized */
        return 0;
    }

    pi->serdes_init_always = pcieport_param_ull("PCIE_SERDES_INIT_ALWAYS", 0);

    if (pcieport_already_init()) {
        pciesys_loginfo("pcieport: inherited init\n");
        pi->init = 1;
        pi->serdes_init = 1;
        pi->already_init = 1;
        return 0;
    }
#ifdef __aarch64__
    pciesys_loginfo("pcieport: full init\n");
#endif
    pcieport_link_init();
    pi->init = 1;
    return 0;
}

int
pcieport_onetime_portinit(pcieport_t *p)
{
    pcieport_info_t *pi = pcieport_info_get();

    if (p->init) {
        /* port already been through here */
        return 0;
    }

    p->sris           = pcieport_param_ull("PCIE_SRIS", p->sris);
    p->crs            = pcieport_param_ull("PCIE_STRICT_CRS", p->crs);
    p->compliance     = pcieport_param_ull("PCIE_COMPLIANCE", p->compliance);
    p->aer_common     = pcieport_param_ull("PCIE_AER_COMMON", 0);
    p->vga_support    = pcieport_param_ull("PCIE_VGA_SUPPORT", 0);
    p->req_gen        = pcieport_param_ull("PCIE_REQ_GEN", 0);
    p->req_width      = pcieport_param_ull("PCIE_REQ_WIDTH", 0);
    p->reduce_rx_cred = pcieport_param_ull("PCIE_REDUCE_RX_CRED", 0);

    /*
     * We have inherited a system already initialized by
     * uboot or an earlier instance of pcieport.
     * This is a sign we should acquire the current state
     * of this port to initialize the state machine.
     */
    if (pi->already_init) {
        pcieport_intr_init(p);
    }
    p->init = 1;
    return 0;
}
