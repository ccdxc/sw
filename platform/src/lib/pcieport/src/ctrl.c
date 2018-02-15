/*
 * Copyright (c) 2017, Pensando Systems Inc.
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

    gen = pal_reg_rd64(PXC_(CFG_C_MAC_K_GEN, pn));
    gen &= 0xffffffff00000000ULL;
    gen |= 0x80e20254;

    switch (p->width) {
    case 1: gen |= (0xf << 24); break;
    case 2: gen |= (0xe << 24); break;
    case 4: gen |= (0xc << 24); break;
    case 8: gen |= (0x8 << 24); break;
    case 16: /* 16 is default */ break;
    }

    pal_reg_wr64(PXC_(CFG_C_MAC_K_GEN, pn), gen);
}

static void
pcieport_hostconfig(pcieport_t *p)
{
    const int pn = p->port;

    /* toggle these resets */
    pcieport_set_serdes_reset(p, 1);
    pcieport_set_pcs_reset(p, 1);
    pcieport_set_serdes_reset(p, 0);
    pcieport_set_pcs_reset(p, 0);

    pal_reg_wr32(PP_(CFG_PP_LINKWIDTH), 0x2222); /* 4 port x4 linkwidth mode */
    pal_reg_wr32(PP_(CFG_PP_SW_RESET), 0xfffc);

    pcieport_mac_k_gen(p);

    pal_reg_wr32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 4, 0x06040000); /*class code*/

    pcieport_set_mac_reset(p, 0); /* mac unreset */
    /* XXX !is_asic only XXX */
    pcieport_set_clock_freq(p, 8);
    pcieport_set_ltssm_en(p, 1);  /* ready for ltssm */
}

static void
pcieport_rcconfig(pcieport_t *p)
{
    /* XXX */
    assert(0);
}

void
pcieport_config(pcieport_t *p)
{
    if (!p->config) return;

    if (p->host) {
        pcieport_hostconfig(p);
    } else {
        pcieport_rcconfig(p);
    }
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
        p->subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;
    }

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
