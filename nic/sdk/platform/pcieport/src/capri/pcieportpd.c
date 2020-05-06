/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/src/pcieport_impl.h"
#include "pcieportpd.h"

int
pcieportpd_validate_hostconfig(pcieport_t *p)
{
    switch (p->cap_gen) {
    case 1:
    case 2:
    case 3:
    case 4:
        /* all good */
        break;
    default:
        pciesys_logerror("port %d unsupported gen%d\n", p->port, p->cap_gen);
        return -EFAULT;
    }

    switch (p->cap_width) {
    case 1: /* x1 uses 2 lanes */
    case 2:
        /* XXX verify peer isn't also configured to use our lanes */
        break;
    case 4:
        /* odd ports don't support x4 */
        if (p->port & 0x1) {
            goto bad_width;
        }
        /* XXX verify peer isn't also configured to use our lanes */
        break;
    case 8:
        /* only ports 0,4 can support x8 */
        if (p->port != 0 && p->port != 4) {
            goto bad_width;
        }
        break;
    case 16:
        /* only port 0 can use all 16 lanes */
        if (p->port != 0) {
            goto bad_width;
        }
        break;
    default:
        pciesys_logerror("port %d unsupported x%d\n", p->port, p->cap_width);
        return -ERANGE;
    }
    return 0;

 bad_width:
    pciesys_logerror("port %d doesn't support x%d\n", p->port, p->cap_width);
    return -EINVAL;
}

int
pcieportpd_hostconfig(pcieport_t *p, const pciemgr_params_t *params)
{
    switch (p->cap_width) {
    case  1: /* x1 uses 2 lanes */
    case  2: p->lanemask = 0x0003 << (p->port << 0); break;
    case  4: p->lanemask = 0x000f << (p->port << 1); break;
    case  8: p->lanemask = 0x00ff << (p->port << 2); break;
    case 16: p->lanemask = 0xffff << (p->port << 4); break;
    }

    return 0;
}

/*
 * We don't get with pcieport_get(port) here.
 * This is used by some callers who are not
 * going to pcieport_open() but want to read
 * registers directly (e.g. pcieutil);
 */
int
pcieportpd_is_accessible(const int port)
{
    const u_int32_t sta_rst = pal_reg_rd32(PXC_(STA_C_PORT_RST, port));
    const u_int32_t sta_mac = pal_reg_rd32(PXC_(STA_C_PORT_MAC, port));
    const u_int8_t ltssm_st = sta_mac & 0x1f;

    /*
     * port is accessible if out of reset so pcie refclk is good and
     * ltssm_st >= 0x9 (config.complete).
     *
     * Note: this is a bit conservative, we don't need to be all the
     * way through config to have a stable refclk.  But the link might
     * still be settling and refclk could go away during the settling
     * time so we check ltssm_st to be sure we've settled.
     */
    return (sta_rst & STA_RSTF_(PERSTN)) != 0 && ltssm_st >= 0x9;
}
