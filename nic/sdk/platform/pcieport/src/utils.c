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
#include "platform/pcieport/include/pcieport.h"
#include "pcieport_impl.h"

/*
 * Get the marker state for the port.
 */
static int
pcieport_get_tgt_marker_rx(pcieport_t *p)
{
    const u_int64_t tgt_marker_rx = PXB_(STA_TGT_MARKER_RX);
    const u_int32_t portbit = 1 << p->port;

    return (pal_reg_rd32(tgt_marker_rx) & portbit) != 0;
}

/*
 * When the port link goes down the barrier closes and then
 * the port logic inserts a marker into the port transaction
 * pipeline.  When the marker is received at the end of the pipe
 * we know the pipeline has been flushed and all pending transactions
 * have been processed.  The hardware logic sets the tgt_marker_rx bit
 * for the port when the marker is received at the end of the pipeline.
 * We wait here for the indication that the tgt marker has been received.
 *
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

    p->stats.markerpolllast = polls;
    if (polls > p->stats.markerpollmax) {
        p->stats.markerpollmax = polls;
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
    const u_int64_t tgt_axi_pending = PXB_(STA_TGT_AXI_PENDING);
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

    p->stats.axipendpolllast = polls;
    if (polls > p->stats.axipendpollmax) {
        p->stats.axipendpollmax = polls;
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
    } while (!is_open && ++polls < maxpolls);

    p->stats.gatepolllast = polls;
    if (polls > p->stats.gatepollmax) {
        p->stats.gatepollmax = polls;
    }

    if (!is_open) {
        return -1;
    }
    return 0;
}

void
pcieport_set_crs(pcieport_t *p, const int on)
{
    const u_int64_t cfg_mac_reg = PXC_(CFG_C_PORT_MAC, p->port);
    u_int32_t cfg_mac = pal_reg_rd32(cfg_mac_reg);

    if (on) {
        cfg_mac |= CFG_MACF_(0_2_CFG_RETRY_EN);
    } else {
        cfg_mac &= ~CFG_MACF_(0_2_CFG_RETRY_EN);
    }
    pal_reg_wr32(cfg_mac_reg, cfg_mac);
}

/*
 * Note this function sets reset on/off, but the register
 * is inverted logic for _RESET_N.  If on=1 then put the serdes
 * in reset by clearing reset_n bits;
 */
void
pcieport_set_serdes_reset(pcieport_t *p, const int on)
{
    u_int32_t v = pal_reg_rd32(PP_(CFG_PP_SD_ASYNC_RESET_N, p->port));
    if (on) {
        v &= ~p->lanemask;
    } else {
        v |= p->lanemask;
    }
    pal_reg_wr32(PP_(CFG_PP_SD_ASYNC_RESET_N, p->port), v);
}

/*
 * Note this function sets reset on/off, but the register
 * is inverted logic for _RESET_N.  If on=1 then put the serdes
 * in reset by clearing reset_n bits;
 */
void
pcieport_set_pcs_reset(pcieport_t *p, const int on)
{
    u_int32_t v = pal_reg_rd32(PP_(CFG_PP_PCS_RESET_N, p->port));
    if (on) {
        v &= ~p->lanemask;
    } else {
        v |= p->lanemask;
    }
    pal_reg_wr32(PP_(CFG_PP_PCS_RESET_N, p->port), v);
}

void
pcieport_set_mac_reset(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));

    if (on) {
        reg |= CFG_C_PORT_MAC_F_MAC_RESET;
    } else {
        reg &= ~CFG_C_PORT_MAC_F_MAC_RESET;
    }
    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}

int
pcieport_get_ltssm_en(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));

    return (reg & CFG_C_PORT_MAC_F_LTSSM_EN) != 0;
}

void
pcieport_set_ltssm_en(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));

    if (on) {
        reg |= CFG_C_PORT_MAC_F_LTSSM_EN;
    } else {
        reg &= ~CFG_C_PORT_MAC_F_LTSSM_EN;
    }
    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}

void
pcieport_set_aer_common_en(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg[2];

    pal_reg_rd32w(PXC_(CFG_C_PORT_MAC, pn), reg, 2);
    if (on) {
        reg[1] |= CFG_C_PORT_MAC_F_AER_COMMON_EN;
    } else {
        reg[1] &= ~CFG_C_PORT_MAC_F_AER_COMMON_EN;
    }
    pal_reg_wr32w(PXC_(CFG_C_PORT_MAC, pn), reg, 2);
}

void
pcieport_set_clock_freq(pcieport_t *p, const u_int32_t freq)
{
    const int pn = p->port;
    u_int32_t reg = pal_reg_rd32(PXC_(CFG_C_PORT_MAC, pn));

    reg &= ~CFG_C_PORT_MAC_F_CLOCK_FREQ_MASK;
    reg |= freq << CFG_C_PORT_MAC_F_CLOCK_FREQ_SHIFT;

    pal_reg_wr32(PXC_(CFG_C_PORT_MAC, pn), reg);
}

void
pcieport_set_margining_ready(pcieport_t *p, const int on)
{
    const int pn = p->port;
    u_int32_t reg[2];
#define MARGINING_READY \
    ASIC_(PXC_CSR_CFG_C_PORT_MAC_CFG_C_PORT_MAC_1_2_MARGINING_READY_FIELD_MASK)

    pal_reg_rd32w(PXC_(CFG_C_PORT_MAC, pn), reg, 2);
    if (on) {
        reg[1] |= MARGINING_READY;
    } else {
        reg[1] &= ~MARGINING_READY;
    }
    pal_reg_wr32w(PXC_(CFG_C_PORT_MAC, pn), reg, 2);
}

/*
 * The interface to this register group has special considerations.
 * When the "update" field is set the hw sets its internal buffer pointers
 * to be within [base,base+limit).  This update happens when the update
 * field transitions 0->1.  We want to do a read-modify-write to update
 * port parameters here, but we want to have update=1 only for the port
 * with new parameters.  We'll follow the protocol that we read current
 * settings, set update=0 for all ports, set update=1 along with the new
 * port params, then set update=0 again to leave the register group sane.
 *
 * Also, even when update=0 the base/limit params are used by hw to
 * verify the port buffer limits during operation so base/limit must
 * remain set correctly even when update=0.
 */
void
pcieport_rx_credit_bfr(const int port, const int base, const int limit)
{
#define RX_CREDIT_BFR_ADDR PXB_(CFG_TGT_RX_CREDIT_BFR)

    union {
        struct {
#define FIELDS(P) \
            u_int32_t adr_base##P:10; \
            u_int32_t adr_limit##P:10; \
            u_int32_t update##P:1; \
            u_int32_t rst_rxfifo##P:1
            FIELDS(0); FIELDS(1); FIELDS(2); FIELDS(3);
            FIELDS(4); FIELDS(5); FIELDS(6); FIELDS(7);
#undef FIELDS
        } __attribute__((packed));
        u_int32_t w[6];
    } r;

    pal_reg_rd32w(RX_CREDIT_BFR_ADDR, r.w, 6);

    /* clear all updateX fields, in case someone left one set */
    r.update0 = 0; r.update1 = 0; r.update2 = 0; r.update3 = 0;
    r.update4 = 0; r.update5 = 0; r.update6 = 0; r.update7 = 0;

#define UPD_FIELDS(P, base, limit, upd) \
    do { \
        r.adr_base##P = base; \
        r.adr_limit##P = limit; \
        r.update##P = upd; \
        r.rst_rxfifo##P = 0; \
    } while (0)

    /* set correct fields and update bit */
    switch (port) {
    case 0: UPD_FIELDS(0, base, limit, 1); break;
    case 1: UPD_FIELDS(1, base, limit, 1); break;
    case 2: UPD_FIELDS(2, base, limit, 1); break;
    case 3: UPD_FIELDS(3, base, limit, 1); break;
    case 4: UPD_FIELDS(4, base, limit, 1); break;
    case 5: UPD_FIELDS(5, base, limit, 1); break;
    case 6: UPD_FIELDS(6, base, limit, 1); break;
    case 7: UPD_FIELDS(7, base, limit, 1); break;
    default: break;
    }

    /* write new config */
    pal_reg_wr32w(RX_CREDIT_BFR_ADDR, r.w, 6);
    /* read back to flush */
    pal_reg_rd32w(RX_CREDIT_BFR_ADDR, r.w, 6);

    /* set correct fields and clear update bit */
    switch (port) {
    case 0: UPD_FIELDS(0, base, limit, 0); break;
    case 1: UPD_FIELDS(1, base, limit, 0); break;
    case 2: UPD_FIELDS(2, base, limit, 0); break;
    case 3: UPD_FIELDS(3, base, limit, 0); break;
    case 4: UPD_FIELDS(4, base, limit, 0); break;
    case 5: UPD_FIELDS(5, base, limit, 0); break;
    case 6: UPD_FIELDS(6, base, limit, 0); break;
    case 7: UPD_FIELDS(7, base, limit, 0); break;
    default: break;
    }

    /* write config with update=0 */
    pal_reg_wr32w(RX_CREDIT_BFR_ADDR, r.w, 6);
}

void
pcieport_pcsd_control_sris(const int sris_en)
{
    const int sris_en_grp = sris_en ? 0x3 : 0;

    union {
        struct {
#define FIELDS(P) \
            u_int32_t fts_align_grp_##P:2; \
            u_int32_t sris_en_grp_##P:2; \
            u_int32_t rx8b10b_realign_grp_##P:2
            FIELDS(0); FIELDS(1); FIELDS(2); FIELDS(3);
            FIELDS(4); FIELDS(5); FIELDS(6); FIELDS(7);
#undef FIELDS
        } __attribute__((packed));
        u_int32_t w[2];
    } r;

    pal_reg_rd32w(PP_(CFG_PP_PCSD_CONTROL, 0), r.w, 2);

    r.sris_en_grp_0 = sris_en_grp;
    r.sris_en_grp_1 = sris_en_grp;
    r.sris_en_grp_2 = sris_en_grp;
    r.sris_en_grp_3 = sris_en_grp;
    r.sris_en_grp_4 = sris_en_grp;
    r.sris_en_grp_5 = sris_en_grp;
    r.sris_en_grp_6 = sris_en_grp;
    r.sris_en_grp_7 = sris_en_grp;

    pal_reg_wr32w(PP_(CFG_PP_PCSD_CONTROL, 0), r.w, 2);
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

u_int32_t
pcieport_get_sta_c_port_mac(pcieport_t *p)
{
    return pal_reg_rd32(PXC_(STA_C_PORT_MAC, p->port));
}

u_int32_t
pcieport_get_sta_p_port_mac(pcieport_t *p)
{
    return pal_reg_rd32(PXP_(STA_P_PORT_MAC, p->port));
}

int
pcieport_get_mac_lanes_reversed(pcieport_t *p)
{
    const u_int32_t sta_mac = pcieport_get_sta_p_port_mac(p);
    return (sta_mac & STA_P_PORT_MACF_(LANES_REVERSED)) != 0;
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

static void
pcieport_clear_early_sat_ind_reason(pcieport_t *p)
{
    union {
        struct {
            u_int32_t pmr_force:8;
            u_int32_t prt_force:8;
            u_int32_t msg:8;
            u_int32_t atomic:8;
            u_int32_t poisoned:8;
            u_int32_t unsupp:8;
            u_int32_t pmv:8;
            u_int32_t db_pmv:8;
            u_int32_t pmt_miss:8;
            u_int32_t rc_vfid_miss:8;
            u_int32_t pmr_prt_miss:8;
            u_int32_t prt_oor:8;
            u_int32_t bdf_wcard_oor:8;
            u_int32_t vfid_oor:8;
        } __attribute__((packed));
        u_int32_t w[4];
    } v;
    const u_int64_t sat_tgt_ind_reason = PXB_(SAT_TGT_IND_REASON);

    pal_reg_rd32w(sat_tgt_ind_reason, v.w, 4);
    /*
     * We get many pmt_miss events during BIOS bus scan
     * as the BIOS probes for devices that don't exist.
     * This counter ends up saturated at startup always.
     * We'll clear the counter here so it can count
     * the pmt_miss events we get *after* the BIOS scan.
     *
     * (We could add a catchall entry in the PMT to catch
     * these and return UR?)
     */
    v.pmt_miss = 0;
    pal_reg_wr32w(sat_tgt_ind_reason, v.w, 4);
}

/*
 * We get some pipe_decode_disparity_err counts during link
 * establishment.  We clear these when the link comes up so any
 * counts that show up later are from after the link has settled.
 */
static void
pcieport_clear_pipe_decode_err(pcieport_t *p)
{
    const u_int64_t base = PP_(SAT_PP_PIPE_DECODE_DISPARITY_ERR_0, p->port);
    const int stride = 0x14;
    int i;

    for (i = 0; i < 16; i++) {
        const int lanebit = 1 << i;

        if (p->lanemask & lanebit) {
            pal_reg_wr32(base + (i * stride), 0);
        }
    }
}

void
pcieport_clear_early_link_counts(pcieport_t *p)
{
    pcieport_set_ltssm_st_cnt(p, 0);
    pcieport_clear_early_sat_ind_reason(p);
    pcieport_clear_pipe_decode_err(p);
}

void
pcieport_get_tx_fc_credits(const int port,
                           int *posted_hdr, int *posted_data,
                           int *nonposted_hdr, int *nonposted_data)
{
    u_int32_t reg[2];
#define HDR_CREDITS(r)          (((r) >> 16) & 0x7ff)
#define HDR_INFINITE(r)         ((r) & (1 << 27))
#define DATA_CREDITS(r)         ((r) & 0x7fff)
#define DATA_INFINITE(r)        ((r) & (1 << 15))

    pal_reg_rd32w(PXC_(STA_C_TX_FC_CREDITS, port), reg, 2);

    *posted_hdr     =  HDR_INFINITE(reg[0]) ? -1 :  HDR_CREDITS(reg[0]);
    *posted_data    = DATA_INFINITE(reg[0]) ? -1 : DATA_CREDITS(reg[0]);
    *nonposted_hdr  =  HDR_INFINITE(reg[1]) ? -1 :  HDR_CREDITS(reg[1]);
    *nonposted_data = DATA_INFINITE(reg[1]) ? -1 : DATA_CREDITS(reg[1]);
}
