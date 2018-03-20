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

/*
 * Note this function sets reset on/off, but the register
 * is inverted logic for _RESET_N.  If on=1 then put the serdes
 * in reset by clearing reset_n bits;
 */
void
pcieport_set_serdes_reset(pcieport_t *p, const int on)
{
    u_int32_t v = pal_reg_rd32(PP_(CFG_PP_SD_ASYNC_RESET_N));
    if (on) {
        v &= ~p->lanemask;
    } else {
        v |= p->lanemask;
    }
    pal_reg_wr32(PP_(CFG_PP_SD_ASYNC_RESET_N), v);
}

/*
 * Note this function sets reset on/off, but the register
 * is inverted logic for _RESET_N.  If on=1 then put the serdes
 * in reset by clearing reset_n bits;
 */
void
pcieport_set_pcs_reset(pcieport_t *p, const int on)
{
    u_int32_t v = pal_reg_rd32(PP_(CFG_PP_PCS_RESET_N));
    if (on) {
        v &= ~p->lanemask;
    } else {
        v |= p->lanemask;
    }
    pal_reg_wr32(PP_(CFG_PP_PCS_RESET_N), v);
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

void
pcieport_set_clock_freq(pcieport_t *p, const u_int32_t freq)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));
#define CLOCK_FREQ_MASK \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_TL_CLOCK_FREQ_FIELD_MASK
#define CLOCK_FREQ_SHIFT \
    CAP_PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_0_2_TL_CLOCK_FREQ_LSB

    reg &= ~CLOCK_FREQ_MASK;
    reg |= freq << CLOCK_FREQ_SHIFT;

    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}

void
pcieport_rx_credit_bfr(const int port, const int base, const int limit)
{
#define RX_CREDIT_BFR_ADDR \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + \
     CAP_PXB_CSR_CFG_TGT_RX_CREDIT_BFR_BYTE_ADDRESS)

    union {
        struct {
#define FIELDS(P) \
            u_int32_t adr_base##P:10; \
            u_int32_t adr_limit##P:10; \
            u_int32_t update##P:1; \
            u_int32_t rst_rxfifo##P:1
            FIELDS(0); FIELDS(1); FIELDS(2); FIELDS(3);
            FIELDS(4); FIELDS(5); FIELDS(6); FIELDS(7);
        } __attribute__((packed));
        u_int32_t w[6];
    } r;

    pal_reg_rd32w(RX_CREDIT_BFR_ADDR, r.w, 6);

#define UPD_FIELDS(P, base, limit) \
    do { \
        r.adr_base##P = base; \
        r.adr_limit##P = limit; \
        r.update##P = 1; \
        r.rst_rxfifo##P = 0; \
    } while (0)

    switch (port) {
    case 0: UPD_FIELDS(0, base, limit); break;
    case 1: UPD_FIELDS(1, base, limit); break;
    case 2: UPD_FIELDS(2, base, limit); break;
    case 3: UPD_FIELDS(3, base, limit); break;
    case 4: UPD_FIELDS(4, base, limit); break;
    case 5: UPD_FIELDS(5, base, limit); break;
    case 6: UPD_FIELDS(6, base, limit); break;
    case 7: UPD_FIELDS(7, base, limit); break;
    default: break;
    }

    pal_reg_wr32w(RX_CREDIT_BFR_ADDR, r.w, 6);
}
