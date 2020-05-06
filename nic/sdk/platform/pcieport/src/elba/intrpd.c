/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>

#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pcieport/include/pcieport.h"
#include "platform/pcieport/include/portcfg.h"
#include "platform/pcieport/src/pcieport_impl.h"
#include "pcieportpd.h"

#define INT_C_MAC_INTERRUPT \
    ASIC_(PXC_CSR_INT_GROUPS_INTREG_INT_C_MAC_INTERRUPT_FIELD_MASK)
#define INT_PP_INTERRUPT \
    ASIC_(PP_CSR_INT_GROUPS_INTREG_INT_PP_INTERRUPT_FIELD_MASK)

/*
 * This is the pcie application leaf register
 * that contains all the "mission-mode" interrupt
 * sources we will monitor.
 */
static void
pcieport_set_c_int_c_mac_intreg(const int port, const u_int32_t mask)
{
    u_int64_t reg = PXC_(INT_C_MAC_INT_ENABLE_SET, port);
    pal_reg_wr32(reg, mask);
}

static void
pcieport_clear_c_int_c_mac_intreg(const int port, const u_int32_t mask)
{
    u_int64_t reg = PXC_(INT_C_MAC_INT_ENABLE_CLEAR, port);
    pal_reg_wr32(reg, mask);
}

static void
pcieport_set_c_int_groups_intreg(const int port, const int on)
{
    const u_int64_t reg = PXC_(INT_GROUPS_INT_ENABLE_RW_REG, port);
    u_int32_t v = pal_reg_rd32(reg);

    if (on) {
        v |= INT_C_MAC_INTERRUPT;
    } else {
        v &= ~INT_C_MAC_INTERRUPT;
    }
    pal_reg_wr32(reg, v);
}

static void
pcieport_set_c_intr(const int port, const int on)
{
    const u_int64_t reg = PXC_(CSR_INTR, port);
    u_int32_t v = pal_reg_rd32(reg);

#define DOWSTREAM_ENABLE \
    ASIC_(PXP_CSR_CSR_INTR_DOWSTREAM_ENABLE_FIELD_MASK)

    if (on) {
        v |= DOWSTREAM_ENABLE;
    } else {
        v &= ~DOWSTREAM_ENABLE;
    }
    pal_reg_wr32(reg, v);
}

/*
 * Manage the intrs in PP_INTREG.  Note that there are
 * multiple per-port interrupts in this register.  We use
 *     portX_c_int_interrupt for mac interrupts, and
 *     perstXn_dn2up_interrupt for poweron notification.
 */
static void
pcieport_set_int_pp_intreg(const int port, const int on)
{
    u_int64_t reg;
    u_int32_t v;

    if (on) {
        reg = PP_(INT_PP_INT_ENABLE_SET, port);
    } else {
        reg = PP_(INT_PP_INT_ENABLE_CLEAR, port);
    }
    v = 0;
#if 0
    /* XXX ELBA-TODO find these intr on elba? */

    /* set port_c_int_interrupt so leaf intrs propagate up */
    v |= PP_INTREG_PORT_C_INT_INTERRUPT(port);
    /* set intreg_perstn so perstn intrs arrive */
    v |= PP_INTREG_PERSTN(port);
#endif
    pal_reg_wr32(reg, v);
}

static void
pcieport_set_int_groups_intreg(const int port, const int on)
{
    const u_int64_t reg = PP_(INT_GROUPS_INT_ENABLE_RW_REG, port);
    u_int32_t v = pal_reg_rd32(reg);

    if (on) {
        v |= INT_PP_INTERRUPT;
    } else {
        v &= ~INT_PP_INTERRUPT;
    }
    pal_reg_wr32(reg, v);
}

/*
 * Set enable registers for the hierarchy to get
 * the pcie mac interrupt delivered to the GIC.
 * Note we set registers from leaf-to-GIC.
 */
static void
pcieport_set_mac_intr_hierarchy(const int port, const int on)
{
    pcieport_set_c_int_groups_intreg(port, on);
    pcieport_set_c_intr(port, on);
    pcieport_set_int_pp_intreg(port, on);
    pcieport_set_int_groups_intreg(port, on);
    /*
     * Technically we'd like to manage pp_intr too,
     * but the UIO interrupt controller owns this register
     * as the top-most register that has a mask register
     * available for masking the intr while processing.
     *
     * pcieport_set_pp_intr(port, on);
     */
}

static void
pcieport_ack_c_int_groups_intreg(const int port)
{
    const u_int64_t reg = PXC_(INT_GROUPS_INTREG, port);
    const u_int32_t v = INT_C_MAC_INTERRUPT;

    /* write-1-to-clear */
    pal_reg_wr32(reg, v);
}

static void
pcieport_ack_int_pp_intreg(const int port)
{

    const u_int64_t reg = PP_(INT_PP_INTREG, port);
    /* XXX ELBA-TODO */
    const u_int32_t v = 0; /* PP_INTREG_PORT_C_INT_INTERRUPT(port);*/

    /* write-1-to-clear */
    pal_reg_wr32(reg, v);
}

/*
 * Enable the requested interrupt sources, and enable
 * the interrupt hierarchy required to get the interrupt
 * to the GIC.
 */
static void
pcieport_mac_intr_enable(const int port, const u_int32_t mask)
{
    pcieport_set_c_int_c_mac_intreg(port, mask);
    pcieport_set_mac_intr_hierarchy(port, 1);
}

/*
 * Disable the mac interrupt sources (and the hierarchy).
 */
static void
pcieport_mac_intr_disable(const int port, const u_int32_t mask)
{
    pcieport_clear_c_int_c_mac_intreg(port, mask);
    pcieport_set_mac_intr_hierarchy(port, 0);
}

/*
 * Acknowledge the intreg hierarchy that latches mac intrs.
 * These need to be cleared before the source is re-enabled.
 */
static void
pcieport_mac_intr_ack(const int port)
{
    pcieport_ack_c_int_groups_intreg(port);
    pcieport_ack_int_pp_intreg(port);
}

/*
 * Update int_mac stats to count each instance of these interrupts.
 */
static void
pcieport_int_mac_stats(u_int32_t int_mac, u_int64_t *stats)
{
    while (int_mac) {
        u_int32_t bit = ffs(int_mac) - 1;
        int_mac &= ~(1 << bit);
        stats[bit]++;
    }
}

/*
 * 3 conditions for link up
 *     1) PCIE clock (PERSTxN_DN2UP)
 *     2) CFG_PP_SW_RESET unreset
 *     3) CFG_C_PORT_MAC = 0x8 to clear bit 0
 */
static int
pcieport_handle_mac_intr(pcieport_t *p)
{
    const int port = p->port;
    u_int32_t int_mac, sta_rst;

    int_mac = pal_reg_rd32(PXC_(INT_C_MAC_INTREG, port));
#ifndef __aarch64__
    {
        static int sim_int_mac = (MAC_INTREGF_(RST_UP2DN) |
                                  MAC_INTREGF_(LINK_DN2UP) |
                                  MAC_INTREGF_(SEC_BUSNUM_CHANGED));
        if (sim_int_mac) {
            int_mac = sim_int_mac;
            pal_reg_wr32(PXC_(STA_C_PORT_RST, 0), STA_RSTF_(PERSTN));
        }
        sim_int_mac = 0; /* cancel after first time */
    }
#endif
    if (int_mac == 0) return -1;

    pcieport_int_mac_stats(int_mac, &p->stats.int_mac_stats);

    /*
     * Snapshot current status here *before* ack'ing int_mac intrs.
     */
    sta_rst = pcieport_get_sta_rst(p);
#ifdef __aarch64__
    pal_reg_wr32(PXC_(INT_C_MAC_INTREG, port), int_mac);
    pcieport_mac_intr_ack(port);

    /*
     * Don't log LTSSM_ST_CHANGED, we sometimes see a burst of
     * these as the link settles depending on how long the BIOS
     * takes to bring up the link after reset.
     * We'll count LTSSM_ST_CHANGED below for stats.
     */
    if (int_mac & ~MAC_INTREGF_(LTSSM_ST_CHANGED)) {
        pciesys_loginfo("pcieport_intr: int_mac 0x%x sta_rst 0x%x\n",
                        int_mac, sta_rst);
    }
#else
    if (0) pcieport_mac_intr_ack(port);
#endif

    if (int_mac & MAC_INTREGF_(LTSSM_ST_CHANGED)) {
        /* count these, might indicate link quality issues */
        if (p->state < PCIEPORTST_LINKUP) {
            p->stats.intr_ltssmst_early++;
        } else {
            p->stats.intr_ltssmst++;
        }
    }
    if (int_mac & MAC_INTREGF_(LINK_UP2DN)) {
        p->stats.intr_linkup2dn++;
        pcieport_fsm(p, PCIEPORTEV_LINKDN);
    }
    if (int_mac & MAC_INTREGF_(RST_DN2UP)) {
        p->stats.intr_rstdn2up++;
        pcieport_fsm(p, PCIEPORTEV_MACDN);
    }

    if (sta_rst & STA_RSTF_(PERSTN)) {
        if (int_mac & MAC_INTREGF_(RST_UP2DN)) {
            p->stats.intr_rstup2dn++;
            pcieport_fsm(p, PCIEPORTEV_MACUP);
        }
        if (int_mac & MAC_INTREGF_(LINK_DN2UP)) {
            p->stats.intr_linkdn2up++;
            pcieport_fsm(p, PCIEPORTEV_LINKUP);
        }
        if (int_mac & MAC_INTREGF_(SEC_BUSNUM_CHANGED)) {
            p->stats.intr_secbus++;
            pcieport_fsm(p, PCIEPORTEV_BUSCHG);
        }
    }
    return 0;
}

static void
pcieport_poweron(pcieport_t *p)
{
    int r;

#ifdef __aarch64__
    pciesys_loginfo("port%d: poweron\n", p->port);
#endif
    r = pcieport_config_powerup(p);
    if (r) pciesys_logwarn("%d: poweron config failed: %d\n", p->port, r);
}

/*
 * PERSTxN indicates PCIe clock is good.
 * This happens on HAPS at poweron once host power is stable
 * and PCIe clock is provided to our PCIe slot.
 *
 * ASIC with local refclk would always have good PERSTxN, but local
 * refclk doesn't allow us to support host spread-spectrum clocks.
 *
 * This is our indication to configure the port and
 * bring the port out of reset, assert LTSSM_EN, start the
 * process of link bringup.
 */
static int
pcieport_handle_pp_intr(pcieport_t *p)
{
    u_int32_t int_pp;

    int_pp = pal_reg_rd32(PP_(INT_PP_INTREG, p->port));
#ifndef __aarch64__
    {
        /*
         * In our x86_64 simulation environment we want this
         * to happen once to kick off our state machines and get
         * everything in the proper state.
         */
        static int sim_int_pp = PP_INTREG_PERSTN(0);
        if (sim_int_pp) {
            /* sim perst, sta_rst */
            int_pp = sim_int_pp;
            pal_reg_wr32(PXC_(STA_C_PORT_RST, 0), STA_RSTF_(PERSTN));
        }
        sim_int_pp = 0; /* cancel after first time */
    }
#endif

    if (int_pp & PP_INTREG_PERSTN(p->port)) {
#ifdef __aarch64__
        /* write-1-to-clear acknowledge the interrupt */
        pal_reg_wr32(PP_(INT_PP_INTREG, p->port), PP_INTREG_PERSTN(p->port));
        pciesys_loginfo("pp_intr%d: int_pp 0x%x\n", p->port, int_pp);
#endif
        p->stats.intr_perstn++;
        /*
         * Perhaps PERSTN intr fired but then PERSTN went
         * away before we got a chance to process it.
         * Check that we currently have PERSTN asserted now
         * and if still asserted process as true poweron event.
         */
        if (pcieport_get_perstn(p)) {
            pcieport_poweron(p);
        }
        return 0;       /* is our intr */
    }
    return -1;          /* not our intr */
}

int
pcieportpd_check_for_intr(const int port)
{
    pcieport_t *p = pcieport_get(port);
    int r = -1;

    if (p != NULL) {
        /* check poweron intr first - rare, but more important */
        if (r < 0) {
            r = pcieport_handle_pp_intr(p);
        }
        /* check port mac intr */
        if (r < 0) {
            r = pcieport_handle_mac_intr(p);
        }
        if (r >= 0) p->stats.intr_total++;
    }
    return r;
}

void
pcieportpd_intr_inherit(pcieport_t *p)
{
    u_int32_t int_mac, sta_rst, ltssm_cnt;
    pcieportst_t initst = PCIEPORTST_OFF;
    u_int8_t secbus;

    int_mac = pal_reg_rd32(PXC_(INT_C_MAC_INTREG, p->port));
    sta_rst = pcieport_get_sta_rst(p);
#ifdef __aarch64__
    pal_reg_wr32(PXC_(INT_C_MAC_INTREG, p->port), int_mac);
#endif

    if (pcieport_is_accessible(p->port)) {
        u_int64_t pa = PXP_(SAT_P_PORT_CNT_LTSSM_STATE_CHANGED, p->port);
        ltssm_cnt = pal_reg_rd32(pa);
        portcfg_read_bus(p->port, NULL, &secbus, NULL);
    } else {
        secbus = 0;
        ltssm_cnt = 0;
    }

    pciesys_loginfo("pcieport_intr_inherit: "
                    "int_mac 0x%x sta_rst 0x%x secbus 0x%02x ltssm_cnt %d\n",
                    int_mac, sta_rst, secbus, ltssm_cnt);

    if (sta_rst & STA_RSTF_(PERSTN)) {
        if (sta_rst & STA_RSTF_(MAC_DL_UP)) {
            initst = secbus ? PCIEPORTST_UP : PCIEPORTST_LINKUP;
        } else {
            initst = PCIEPORTST_MACUP;
        }
    }

    pcieport_fsm_init(p, initst);
}

#define MAC_INTRS       (MAC_INTREGF_(RST_UP2DN) | \
                         MAC_INTREGF_(LINK_DN2UP) | \
                         MAC_INTREGF_(SEC_BUSNUM_CHANGED) | \
                         MAC_INTREGF_(LTSSM_ST_CHANGED))

void
pcieportpd_intr_enable(const int port)
{
    const u_int32_t mask = MAC_INTRS;

    pcieport_mac_intr_enable(port, mask);
}

void
pcieportpd_intr_disable(const int port)
{
    const u_int32_t mask = MAC_INTRS;

    pcieport_mac_intr_disable(port, mask);
}

int
pcieportpd_intr_init(const int port)
{
    pcieport_intr_enable(port);
    return 0;
}

int
pcieportpd_poll_init(const int port)
{
    pcieport_intr_disable(port);
    return 0;
}
