/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "pci_ids.h"
#include "pal.h"
#include "pciehsys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

static void
pcieport_mac_k_gen(pcieport_t *p)
{
    const int pn = p->port;
    u_int64_t gen;

    gen = pal_reg_rd64_safe(PXC_(CFG_C_MAC_K_GEN, pn));
    gen &= 0xffffffff00000000ULL;
    gen |= 0x80e20254; /* XXX replace hard-coded value? */

    switch (p->width) {
    case 1: gen |= (0xf << 24); break;
    case 2: gen |= (0xe << 24); break;
    case 4: gen |= (0xc << 24); break;
    case 8: gen |= (0x8 << 24); break;
    case 16: /* 16 is default */ break;
    }

    pal_reg_wr64_safe(PXC_(CFG_C_MAC_K_GEN, pn), gen);
}

static void
pcieport_mac_k_pciconf(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t val;
    extern int vga_support;

    /* class code */
    val = 0x06040000;
    pal_reg_wr32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 4, 0x06040000);

    /* vga supported */
    val = pal_reg_rd32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 0xc);
    val &= ~(1 << 4);
    val |= (vga_support << 4);
    pal_reg_wr32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 0xc, val);
}

static void
pcieport_mac_k_rx_cred(pcieport_t *p)
{
    u_int32_t val = 0x00200080;
    pal_reg_wr32(PXC_(CFG_C_MAC_K_RX_CRED, p->port), val);
}

static void
pcieport_mac_set_ids(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t val;

    /* set subvendor/deviceid */
    val = (p->subdeviceid << 16) | p->subvendorid;
    pal_reg_wr32(PXC_(CFG_C_MAC_SSVID_CAP, pn), val);
}

static void
pcieport_unreset(pcieport_t *p)
{
    u_int32_t val = pal_reg_rd32(PP_(CFG_PP_SW_RESET));
    val &= ~(0x3 << (p->port << 1));
    pal_reg_wr32(PP_(CFG_PP_SW_RESET), val);
}

static int
pcieport_mac_unreset(pcieport_t *p)
{
    u_int16_t phystatus;
    int perstn;
    const int maxpolls = 2000; /* 2 seconds */
    int polls = 0;

    do {
        usleep(1000);
        phystatus = pcieport_get_phystatus(p);
        perstn = pcieport_get_perstn(p);
    } while (phystatus && perstn && ++polls < maxpolls);

    p->phypolllast = polls;
    if (polls > p->phypollmax) {
        p->phypollmax = polls;
    }

    if (!perstn) {
        /*
         * perstn went away - we went back into reset
         */
        p->phypollperstn++;
        return -1;
    }

    if (phystatus != 0) {
        /*
         * PHY didn't come out of reset as expected?
         */
        p->phypollfail++;
        return -1;
    }

    pcieport_set_mac_reset(p, 0); /* mac unreset */
    return 0;
}

static int
pcieport_hostconfig(pcieport_t *p)
{
    /* toggle these resets */
    pcieport_set_serdes_reset(p, 1);
    pcieport_set_pcs_reset(p, 1);
    pcieport_set_serdes_reset(p, 0);
    pcieport_set_pcs_reset(p, 0);

    pcieport_unreset(p);

    pcieport_mac_k_gen(p);
    pcieport_mac_k_rx_cred(p);
    pcieport_mac_k_pciconf(p);
    pcieport_mac_set_ids(p);

    /* now ready to unreset mac */
    if (pcieport_mac_unreset(p) < 0) {
        return -1;
    }

    if (!pal_is_asic()) {
        /* reduce clock frequency for fpga */
        pcieport_set_clock_freq(p, 8);
    }

    pcieport_set_ltssm_en(p, 1);  /* ready for ltssm */
    return 0;
}

static int
pcieport_rcconfig(pcieport_t *p)
{
    /* XXX */
    assert(0);
    return 0;
}

int
pcieport_config(pcieport_t *p)
{
    int r;

    if (!p->config) return -1;

    if (p->host) {
        r = pcieport_hostconfig(p);
    } else {
        r = pcieport_rcconfig(p);
    }
    return r;
}

static int
pcieport_cmd_hostconfig(pcieport_t *p, void *arg)
{
    pcieport_hostconfig_t *pcfg = arg;

    if (pcfg) {
        p->gen = pcfg->gen;
        p->width = pcfg->width;
        p->subvendorid = pcfg->subvendorid;
        p->subdeviceid = pcfg->subdeviceid;
    }

    if (p->gen == 0) {
        p->gen = 4;
    }
    if (p->width == 0) {
        p->width = 4;
    }
    if (p->subvendorid == 0) {
        p->subvendorid = PCI_VENDOR_ID_PENSANDO;
    }
    if (p->subdeviceid == 0) {
        p->subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;
    }

    /* fpga config x4 */
    p->lanemask = 0xf << (p->port << 1);

    p->host = 1;
    p->config = 1;

    return 0;
}

static int
pcieport_cmd_crs(pcieport_t *p, void *arg)
{
    int on = *(int *)arg;

    if (!p->config) return -EBADF;
    if (!p->host)   return -EINVAL;
    p->crs = on;
    pcieport_set_crs(p, p->crs);
    return 0;
}

int
pcieport_ctrl(pcieport_t *p, const pcieport_cmd_t cmd, void *arg)
{
    int r = 0;

    if (!p->open) {
        return -EBADF;
    }

    switch (cmd) {
    case PCIEPORT_CMD_HOSTCONFIG:
        r = pcieport_cmd_hostconfig(p, arg);
        break;
    case PCIEPORT_CMD_CRS:
        r = pcieport_cmd_crs(p, arg);
        break;
    default:
        pciehsys_error("pcieport_ctrl: unknown cmd %d\n", cmd);
        r = -EINVAL;
        break;
    }
    return r;
}
