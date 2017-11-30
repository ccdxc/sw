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
#include "pciehsys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

void
pcieport_gate_open(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t reg;
    unsigned int tries = 20; /* 2 secs */

#define PORTGATE_OPEN CAP_PXC_CSR_STA_C_PORT_MAC_PORTGATE_OPEN_FIELD_MASK
    do {
        pal_reg_wr32(PXC_(CFG_C_PORTGATE_OPEN, pn), 1);
        usleep(100000);
        reg = pal_reg_rd32(PXC_(STA_C_PORT_MAC, pn));
#ifndef __aarch64__
        /* simulate gate open immediately */
        reg |= PORTGATE_OPEN;
#endif
    } while ((reg & PORTGATE_OPEN) == 0 && --tries > 0);
    if ((reg & PORTGATE_OPEN) == 0) {
        pciehsys_error("port%d gate_open: PORTGATE_OPEN not set in mac\n",
                       p->port);
        //XXX pcieport_fault(p, "portgate open failed");
        return;
    }
}

void
pcieport_set_crs(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));
#define CFG_RETRY_EN \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_CFG_RETRY_EN_FIELD_MASK

    if (on) {
        reg |= CFG_RETRY_EN;
    } else {
        reg &= ~CFG_RETRY_EN;
    }
    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}

void
pcieport_set_serdes_reset(pcieport_t *p, const int on)
{
    if (on) {
        pal_reg_wr32(PP_(CFG_PP_SD_ASYNC_RESET_N), 0);
    } else {
        pal_reg_wr32(PP_(CFG_PP_SD_ASYNC_RESET_N), 0xffff);
    }
}

void
pcieport_set_pcs_reset(pcieport_t *p, const int on)
{
    if (on) {
        pal_reg_wr32(PP_(CFG_PP_PCS_RESET_N), 0);
    } else {
        pal_reg_wr32(PP_(CFG_PP_PCS_RESET_N), 0xffff);
    }
}

void
pcieport_set_mac_reset(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));
#define MAC_RESET \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_RESET_FIELD_MASK

    if (on) {
        reg |= MAC_RESET;
    } else {
        reg &= ~MAC_RESET;
    }
    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}

void
pcieport_set_ltssm_en(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));
#define LTSSM_EN \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_LTSSM_EN_FIELD_MASK

    if (on) {
        reg |= LTSSM_EN;
    } else {
        reg &= ~LTSSM_EN;
    }
    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}
