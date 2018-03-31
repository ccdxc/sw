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

/*
 * Get the marker state for the port.
 */
static int
pcieport_get_tgt_marker_rx(pcieport_t *p)
{
    const u_int64_t tgt_marker_rx =
        (CAP_ADDR_BASE_PXB_PXB_OFFSET +
         CAP_PXB_CSR_STA_TGT_MARKER_RX_BYTE_ADDRESS);
    const u_int32_t portbit = 1 << p->port;

    return (pal_reg_rd32(tgt_marker_rx) & portbit) != 0;
}

/*
 * When the port link goes down the barrier closes and then
 * the port logic inserts a marker into the port transaction
 * pipeline.  When the marker is received at the end of the pipe
 * we know the pipeline has been flushed and all pending transactions
 * have been process.  The hardware logic sets the tgt_marker_rx bit
 * for the port when the marker is received at the end of the pipeline.
 * We wait here for the indication that the tgt marker has been received.
 * If we busy wait and block the cpu (as we currently do) we must be
 * sure that no transactions in the pipeline require the cpu to complete
 * them, such as indirect transactions, or we will deadlock and wait
 * here forever (or until timeout).  We might need to be more
 * sophisticated here eventually.
 */
int
pcieport_tgt_marker_rx_wait(pcieport_t *p)
{
    const int maxpolls = 100; /* 100 ms */
    int polls = 0;
    int marker;

    marker = pcieport_get_tgt_marker_rx(p);
    if (!marker) {
        do {
            usleep(1000);
            marker = pcieport_get_tgt_marker_rx(p);
        } while (!marker && ++polls < maxpolls);
    }

    p->markerpolllast = polls;
    if (polls > p->markerpollmax) {
        p->markerpollmax = polls;
    }

    if (!marker) {
        return -1;
    }
    return 0;
}

/*
 * Get the number of axi pending transaction count for this port.
 */
static int
pcieport_get_tgt_axi_pending(pcieport_t *p)
{
    const u_int64_t tgt_axi_pending =
        (CAP_ADDR_BASE_PXB_PXB_OFFSET +
         CAP_PXB_CSR_STA_TGT_AXI_PENDING_BYTE_ADDRESS);
    const u_int32_t portshift = p->port * 8;
    const int pending = (pal_reg_rd32(tgt_axi_pending) >> portshift) & 0xff;

    return pending;
}

/*
 * As part of draining a port when the link goes down we want
 * to wait for all outstanding pending transactions to drain.
 */
int
pcieport_tgt_axi_pending_wait(pcieport_t *p)
{
    const int maxpolls = 100; /* 100 ms */
    int polls = 0;
    int pending;

    pending = pcieport_get_tgt_axi_pending(p);
    if (pending) {
        do {
            usleep(1000);
            pending = pcieport_get_tgt_axi_pending(p);
        } while (pending && ++polls < maxpolls);
    }

    p->axipendpolllast = polls;
    if (polls > p->axipendpollmax) {
        p->axipendpollmax = polls;
    }

    if (pending) {
        return -1;
    }
    return 0;
}

int
pcieport_gate_open(pcieport_t *p)
{
    const u_int64_t portgate_open = PXC_(CFG_C_PORTGATE_OPEN, p->port);
    u_int32_t is_open;
    const int maxpolls = 2000; /* 2 secs */
    int polls = 0;

    do {
        pal_reg_wr32(portgate_open, 1);
        (void)pal_reg_rd32(portgate_open); /* flush */
        usleep(1000);
        is_open = pal_reg_rd32(portgate_open);
#ifndef __aarch64__
        /* simulate gate open immediately */
        is_open = 1;
#endif
    } while (!is_open && ++polls < maxpolls);

    p->gatepolllast = polls;
    if (polls > p->gatepollmax) {
        p->gatepollmax = polls;
    }

    if (!is_open) {
        return -1;
    }
    return 0;
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

u_int16_t
pcieport_get_phystatus(pcieport_t *p)
{
    return pal_reg_rd32(PXC_(STA_C_PORT_PHYSTATUS, p->port));
}

u_int32_t
pcieport_get_sta_rst(pcieport_t *p)
{
    return pal_reg_rd32(PXC_(STA_C_PORT_RST, p->port));
}

int
pcieport_get_perstn(pcieport_t *p)
{
    const u_int32_t sta_rst = pcieport_get_sta_rst(p);
    return (sta_rst & STA_RSTF_(PERSTN)) != 0;
}

int
pcieport_get_ltssm_st_cnt(pcieport_t *p)
{
    return pal_reg_rd32(PXP_(SAT_P_PORT_CNT_LTSSM_STATE_CHANGED, p->port));
}

void
pcieport_set_ltssm_st_cnt(pcieport_t *p, const int cnt)
{
    pal_reg_wr32(PXP_(SAT_P_PORT_CNT_LTSSM_STATE_CHANGED, p->port), cnt);
}
