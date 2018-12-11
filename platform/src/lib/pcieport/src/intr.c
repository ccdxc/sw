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
#include <sys/time.h>

#include "platform/src/lib/pal/include/pal.h"
#include "platform/src/lib/pciemgrutils/include/pciesys.h"
#include "pcieport.h"
#include "portcfg.h"
#include "pcieport_impl.h"

/*
 * 3 conditions for link up
 *     1) PCIE clock (PERSTxN_DN2UP)
 *     2) CFG_PP_SW_RESET unreset
 *     3) CFG_C_PORT_MAC = 0x8 to clear bit 0
 */
static int
pcieport_intr(pcieport_t *p)
{
    const int pn = p->port;
    u_int32_t int_mac, sta_rst;

    int_mac = pal_reg_rd32(PXC_(INT_C_MAC_INTREG, pn));
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

    /*
     * Snapshot current status here *before* ack'ing int_mac intrs.
     */
    sta_rst = pcieport_get_sta_rst(p);
#ifdef __aarch64__
    pal_reg_wr32(PXC_(INT_C_MAC_INTREG, pn), int_mac);
    pciesys_loginfo("pcieport_intr: int_mac 0x%x sta_rst 0x%x\n",
                    int_mac, sta_rst);
#endif

    if (int_mac & MAC_INTREGF_(LINK_UP2DN)) {
        pcieport_fsm(p, PCIEPORTEV_LINKDN);
    }
    if (int_mac & MAC_INTREGF_(RST_DN2UP)) {
        pcieport_fsm(p, PCIEPORTEV_MACDN);
    }

    if (sta_rst & STA_RSTF_(PERSTN)) {
        if (int_mac & MAC_INTREGF_(RST_UP2DN)) {
            pcieport_fsm(p, PCIEPORTEV_MACUP);
        }
        if (int_mac & MAC_INTREGF_(LINK_DN2UP)) {
            pcieport_fsm(p, PCIEPORTEV_LINKUP);
        }
        if (int_mac & MAC_INTREGF_(SEC_BUSNUM_CHANGED)) {
            pcieport_fsm(p, PCIEPORTEV_BUSCHG);
        }
    }
    return 0;
}

void
pcieport_intr_init(pcieport_t *p)
{
    u_int32_t int_mac, sta_rst;
    pcieportst_t initst = PCIEPORTST_OFF;
    u_int8_t ltssm_st, secbus;

    int_mac = pal_reg_rd32(PXC_(INT_C_MAC_INTREG, p->port));
    sta_rst = pcieport_get_sta_rst(p);
#ifdef __aarch64__
    pal_reg_wr32(PXC_(INT_C_MAC_INTREG, p->port), int_mac);
#endif

    ltssm_st = sta_rst & 0x1f;
    if (ltssm_st == 0x10) {
        portcfg_read_bus(p->port, NULL, &secbus, NULL);
    } else {
        secbus = 0;
    }

    pciesys_loginfo("pcieport_intr_init: "
                    "int_mac 0x%x sta_rst 0x%x secbus 0x%02x\n",
                    int_mac, sta_rst, secbus);

    if (sta_rst & STA_RSTF_(PERSTN)) {
        if (sta_rst & STA_RSTF_(MAC_DL_UP)) {
            initst = secbus ? PCIEPORTST_UP : PCIEPORTST_LINKUP;
        } else {
            initst = PCIEPORTST_MACUP;
        }
    }

    pcieport_fsm_init(p, initst);
}

static void
pcieport_poweron(pcieport_t *p)
{
    int r;

#ifdef __aarch64__
    pciesys_loginfo("port%d: poweron\n", p->port);
#endif
    pcieport_set_ltssm_st_cnt(p, 0);
    r = pcieport_config(p);
    if (r) pciesys_logwarn("%d: poweron config failed: %d\n", p->port, r);
}

/*
 * PERSTxN indicates PCIe clock is good.
 * This happens on HAPS at poweron once host power is stable
 * and PCIe clock is provided to our PCIe slot.
 * (ASIC will have local refclk so will always have good PERSTxN.  Maybe?)
 *
 * This is our indication to configure the port and
 * bring the port out of reset, assert LTSSM_EN, start the
 * process of link bringup.
 *
 * New algorithm on PERSTxN_DN2UP:
 *     1) unreset PP.*SW_RESET for that port
 *     2) toggle serdes reset, pcs reset: write 0 then 1
 *     3) K_GEN, PCICONF config
 *     4) open PORTGATE
 *     5) crs=0 (if tables programmed)
 */
int
pp_intr(void)
{
    pcieport_info_t *pi = pcieport_info_get();
    u_int32_t int_pp;
    int port;

    int_pp = pal_reg_rd32(PP_(INT_PP_INTREG));
#ifndef __aarch64__
    {
        static int sim_int_pp = INTREG_PERSTN(0);
        if (sim_int_pp) {
            /* sim perst, sta_rst */
            int_pp = sim_int_pp;
            pal_reg_wr32(PXC_(STA_C_PORT_RST, 0), STA_RSTF_(PERSTN));
        }
        sim_int_pp = 0; /* cancel after first time */
    }
#endif
    if (int_pp == 0) return -1;

#ifdef __aarch64__
    pal_reg_wr32(PP_(INT_PP_INTREG), int_pp);
#endif

    for (port = 0; port < PCIEPORT_NPORTS; port++) {
        pcieport_t *p = &pi->pcieport[port];
        if (p->open &&
            (int_pp & INTREG_PERSTN(port)) &&
            pcieport_get_perstn(p)) {

            pcieport_poweron(p);
        }
    }

    return 0;
}

int
pcieport_poll(const int port)
{
    pcieport_info_t *pi = pcieport_info_get();
    pcieport_t *p;

    if (port < 0 || port >= PCIEPORT_NPORTS) {
        return -EBADF;
    }

    pp_intr();
    p = &pi->pcieport[port];
    pcieport_intr(p);
    return 0;
}
