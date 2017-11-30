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
#include "pcieport.h"
#include "pcieport_impl.h"

#define MAC_INTREGF_(REG) \
    (CAP_PXC_CSR_INT_C_MAC_INTREG_ ##REG## _INTERRUPT_FIELD_MASK)
#define STA_RSTF_(REG) \
    (CAP_PXC_CSR_STA_C_PORT_RST_ ##REG## _FIELD_MASK)

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
    extern int pcieport_link_bringup(pcieport_t *p);

    int_mac = pal_reg_rd32(PXC_(INT_C_MAC_INTREG, pn));
    if (int_mac == 0) return -1;

    sta_rst = pal_reg_rd32(PXC_(STA_C_PORT_RST, pn));
    pal_reg_wr32(PXC_(INT_C_MAC_INTREG, pn), int_mac);

    if (int_mac & MAC_INTREGF_(LINK_UP2DN)) {
        printf("link down\n");
        pcieport_fsm(p, PCIEPORTEV_LINKDN);
    }
    if (int_mac & MAC_INTREGF_(RST_DN2UP)) {
        printf("mac down\n");
        pcieport_fsm(p, PCIEPORTEV_MACDN);
    }

    if (sta_rst & STA_RSTF_(PERSTN)) {
        if (int_mac & MAC_INTREGF_(RST_UP2DN)) {
            printf("mac up\n");
            pcieport_fsm(p, PCIEPORTEV_MACUP);
            pcieport_gate_open(p);
            pcieport_set_crs(p, 0);
        }
        if (int_mac & MAC_INTREGF_(LINK_DN2UP)) {
            printf("link up\n");
            pcieport_fsm(p, PCIEPORTEV_LINKUP);
        }
        if (int_mac & MAC_INTREGF_(SEC_BUSNUM_CHANGED)) {
            printf("sec_busnum\n");
            pcieport_fsm(p, PCIEPORTEV_BUSCHG);
        }
    }
    return 0;
}

#define INTREG_PERST0N \
    CAP_PP_CSR_INT_PP_INTREG_PERST0N_DN2UP_INTERRUPT_FIELD_MASK
#define INTREG_PERSTN(port) \
    (INTREG_PERST0N >> (port))

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
    pcieport_info_t *pi = &pcieport_info;
    u_int32_t int_pp;

    int_pp = pal_reg_rd32(PP_(INT_PP_INTREG));
    if (int_pp == 0) return -1;

    pal_reg_wr32(PP_(INT_PP_INTREG), int_pp);

    if (int_pp & INTREG_PERSTN(0)) {
        pcieport_t *p = &pi->pcieport[0];
        printf("power on\n");
        pcieport_config(p);
    }
    return 0;
}

int
pcieport_poll(pcieport_t *p)
{
    pp_intr();
    pcieport_intr(p);
    return 0;
}
