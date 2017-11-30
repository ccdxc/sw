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

#include "pal.h"
#include "pciehsys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

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
    pal_reg_wr32(PXC_(CFG_C_MAC_K_GEN, pn), 0x8ce20254); /* setting Gen1x4 */
    pal_reg_wr32(PXC_(CFG_C_MAC_K_PCICONF, pn) + 4, 0x06040000); /*class code*/

    pcieport_set_mac_reset(p, 0); /* mac unreset */
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
