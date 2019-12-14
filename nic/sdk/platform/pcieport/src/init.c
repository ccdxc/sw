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
#include "platform/pciemgr/include/pciemgr.h"
#include "pcieport.h"
#include "portmap.h"
#include "pcieport_impl.h"

typedef struct pp_linkwidth_s {
    int port;
    int gen;
    int width;
    uint32_t usedlanes;
    uint32_t pp_linkwidth;
} pp_linkwidth_t;

/*
 * Distribute the 1024 pcie rx credits among the ports selected
 * in "portmask". For now, assume all ports get equal credits.
 * (We could give more rx credits to wider (x8 vs x4) or
 * faster ports (gen4 vs gen3) but for now assume homogeneous links.)
 */
static void
pcieport_rx_credit_init(const int portmask)
{
    int port, portbit, nports, ncredits_per_port, base;

    /*
     * Figure out how many ports are going to be configured in portmask
     * and then divide the credits evenly among the ports.
     */
    nports = 0;
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        portbit = 1 << port;
        if (portmask & portbit) {
            nports++;
        }
    }
    ncredits_per_port = 1024 / nports;

    base = 0;
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        portbit = 1 << port;
        if (portmask & portbit) {
            const int limit = base + ncredits_per_port - 1;
            pcieport_rx_credit_bfr(port, base, limit);
            base += ncredits_per_port;
        } else {
            /* "No soup for you!" No credits for this unused port. */
            pcieport_rx_credit_bfr(port, 0, 0);
        }
    }
}

static int
pp_linkwidth(pp_linkwidth_t *pplw, const int maxwidth)
{
    uint32_t linkwidth, portlanes;

    if (pplw->width > maxwidth) {
        pciesys_logerror("pp_linkwidth port%d max width %d > %d\n",
                         pplw->port, pplw->width, maxwidth);
        return -1;
    }

    /*
     * 1-2 lanes: set pp_linkwidth 3
     * 4   lanes: set 2
     * 8   lanes: set 1
     * 16  lanes: set 0
     */
    switch (pplw->width) {
    case  1:
    case  2: linkwidth = 3; portlanes = 0x3    << (pplw->port * 2); break;
    case  4: linkwidth = 2; portlanes = 0xf    << (pplw->port * 2); break;
    case  8: linkwidth = 1; portlanes = 0xff   << (pplw->port * 2); break;
    case 16: linkwidth = 0; portlanes = 0xffff << (pplw->port * 2); break;
    default:
        pciesys_logerror("pp_linkwidth port%d bad width %d\n",
                         pplw->port, pplw->width);
        return -1;
    }

    /* check if someone is already using these lanes */
    if (pplw->usedlanes & portlanes) {
        pciesys_logerror("pp_linkwidth port%d gen%dx%d lane overlap\n",
                         pplw->port, pplw->gen, pplw->width);
        return -1;
    }

    /* we are using these lanes */
    pplw->usedlanes |= portlanes;
    pplw->pp_linkwidth |= linkwidth << (pplw->port * 2);
    return 0;
}

/*
 * We have 16 pcie lanes to configure across 8 ports.
 * Based on the pcie link port config for this platform
 * configure the global PP_LINKWIDTH lane configuration
 * to match the port mapping.
 *
 * Enforce these constraints on the maximum number of lanes
 * for ports, and watch for overlapping lane commitments,
 * e.g. if port0 is configured for 8 lanes then port3 cannot use 2.
 *
 *     Port0 can use 16, 8, 4, 2 lanes.
 *     Port1 can use           2 lanes.
 *     Port2 can use        4, 2 lanes.
 *     Port3 can use           2 lanes.
 *     Port4 can use     8, 4, 2 lanes.
 *     Port5 can use           2 lanes.
 *     Port6 can use        4, 2 lanes.
 *     Port7 can use           2 lanes.
 */
static int
pcieport_pp_linkwidth(void)
{
    const uint32_t portmask = portmap_portmask();
    pp_linkwidth_t pplw;
    int port, r;

    memset(&pplw, 0, sizeof(pplw));

    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        const int portbit = 1 << port;
        if (portmask & portbit) {
            pcieport_spec_t ps;

            if (portmap_getspec(port, &ps) < 0) {
                pciesys_logerror("portmap_getspec port %d failed\n", port);
                return -1;
            }
            pplw.port = port;
            pplw.gen = ps.gen;
            pplw.width = ps.width;

            switch (port) {
            case 0: r = pp_linkwidth(&pplw, 16); break;
            case 1: r = pp_linkwidth(&pplw,  2); break;
            case 2: r = pp_linkwidth(&pplw,  4); break;
            case 3: r = pp_linkwidth(&pplw,  2); break;
            case 4: r = pp_linkwidth(&pplw,  8); break;
            case 5: r = pp_linkwidth(&pplw,  2); break;
            case 6: r = pp_linkwidth(&pplw,  4); break;
            case 7: r = pp_linkwidth(&pplw,  2); break;
            default:
                pciesys_logerror("pp_linkwidth: unknown port %d\n", port);
                return -1;
            }
            if (r < 0) {
                return r;
            }
        }
    }
    pal_reg_wr32(PP_(CFG_PP_LINKWIDTH), pplw.pp_linkwidth);
    return 0;
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
pcieport_link_init(void)
{
    const uint32_t portmask = portmap_portmask();

    pcieport_pp_linkwidth();
    pcieport_rx_credit_init(portmask);
    pcieport_macfifo_thres(5); /* match late-stage ECO */
}

static int
pcieport_already_init(void)
{
#ifdef __aarch64__
    const uint32_t portmask = portmap_portmask();
    int port;

    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        const int portbit = 1 << port;
        /* check first enabled port */
        if (portmask & portbit) {
            pcieport_info_t *pi = pcieport_info_get();
            pcieport_t *p = &pi->pcieport[port];

            /*
             * Check first port for ltssm_en=1 indicating someone
             * initialized the port already.
             */
            if (pcieport_get_ltssm_en(p)) {
                return 1;
            }
            break;
        }
    }
#endif
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
pcieport_onetime_init(pcieport_info_t *pi, pciemgr_initmode_t initmode)
{
    int port;

    /* init port field */
    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        pcieport_t *p = &pi->pcieport[port];
        p->port = port;
    }

    /* first check that version matches what we expect */
    if (pi->version && pi->version != PCIEPORT_VERSION) {
        /* inherit-only, but we can't inherit this version */
        if (initmode == INHERIT_ONLY) {
            pciesys_logerror("pcieport: version mismatch %d (want %d)\n",
                             pi->version, PCIEPORT_VERSION);
            return -1;
        }
        /* reset so we re-init everything */
        pciesys_logwarn("pcieport: version mismatch %d (want %d), resetting\n",
                         pi->version, PCIEPORT_VERSION);
        memset(pi, 0, sizeof(*pi));
    }

    /* if already initialized, no more to do */
    if (pi->init) {
        return 0;
    }

    pi->version = PCIEPORT_VERSION;
    pi->serdes_init_always = pcieport_param_ull("PCIE_SERDES_INIT_ALWAYS", 0);

    if (pcieport_already_init()) {
        pciesys_loginfo("pcieport: inherited init\n");
        pi->init = 1;
        pi->serdes_init = 1;
        pi->inherited_init = 1;
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
    if (pi->inherited_init) {
        pcieport_intr_inherit(p);
    }
    p->init = 1;
    return 0;
}
