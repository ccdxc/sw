#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/evutils/include/evutils.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "platform/pcieport/include/pcieport.h"

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"

#define INT_C_MAC_INTERRUPT \
    CAP_PXC_CSR_INT_GROUPS_INTREG_INT_C_MAC_INTERRUPT_FIELD_MASK
#define INT_PP_INTERRUPT \
    CAP_PP_CSR_INT_GROUPS_INTREG_INT_PP_INTERRUPT_FIELD_MASK

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
    CAP_PXP_CSR_CSR_INTR_DOWSTREAM_ENABLE_FIELD_MASK

    if (on) {
        v |= DOWSTREAM_ENABLE;
    } else {
        v &= ~DOWSTREAM_ENABLE;
    }
    pal_reg_wr32(reg, v);
}

static void
pcieport_set_int_pp_intreg(const int port, const int on)
{
    u_int64_t reg;
    u_int32_t v;

    if (on) {
        reg = PP_(INT_PP_INT_ENABLE_SET);
    } else {
        reg = PP_(INT_PP_INT_ENABLE_CLEAR);
    }
    v = 0;
    /* set port_c_int_interrupt so leaf intrs propagate up */
    v |= PP_INTREG_PORT_C_INT_INTERRUPT(port);
    /* set intreg_perstn so perstn intrs arrive */
    v |= PP_INTREG_PERSTN(port);
    pal_reg_wr32(reg, v);
}

static void
pcieport_set_int_groups_intreg(const int port, const int on)
{
    const u_int64_t reg = PP_(INT_GROUPS_INT_ENABLE_RW_REG);
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

    const u_int64_t reg = PP_(INT_PP_INTREG);
    const u_int32_t v = PP_INTREG_PORT_C_INT_INTERRUPT(port);

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

#define MAC_INTRS       (MAC_INTREGF_(RST_UP2DN) | \
                         MAC_INTREGF_(LINK_DN2UP) | \
                         MAC_INTREGF_(SEC_BUSNUM_CHANGED) | \
                         MAC_INTREGF_(LTSSM_ST_CHANGED))

int
pcieport_intr_enable(const int port)
{
    const u_int32_t mask = MAC_INTRS;

    pcieport_mac_intr_enable(port, mask);
    return 0;
}

int
pcieport_intr_disable(const int port)
{
    const u_int32_t mask = MAC_INTRS;

    pcieport_mac_intr_disable(port, mask);
    return 0;
}

static int
pcieport_handle_pp_intr(const int port)
{
    u_int32_t int_pp;

    int_pp = pal_reg_rd32(PP_(INT_PP_INTREG));
    if (int_pp & PP_INTREG_PERSTN(port)) {
        pal_reg_wr32(PP_(INT_PP_INTREG), PP_INTREG_PERSTN(port));
        return 0;
    }
    return -1;
}

static int
pcieport_handle_mac_intr(const int port)
{
    u_int32_t int_mac;

    int_mac = pal_reg_rd32(PXC_(INT_C_MAC_INTREG, port));
    if (int_mac) {
        pal_reg_wr32(PXC_(INT_C_MAC_INTREG, port), int_mac);
        pcieport_mac_intr_ack(0);
        printf("hande_mac_intr: int_mac 0x%x\n", int_mac);
    }
    return int_mac ? 0 : -1;
}

static void
my_isr(void *arg)
{
    int r = -1;

    printf("my_isr:\n");
    if (r < 0) {
        r = pcieport_handle_pp_intr(0);
    }
    if (r < 0) {
        r = pcieport_handle_mac_intr(0);
    }
    if (r < 0) {
        printf("my_isr: spurious? r %d\n", r);
    }
}

int
main(int argc, char *argv[])
{
    struct pal_int pal_int;
    int opt, r;

    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
        case '?':
        default:
            exit(1);
        }
    }

    if ((r = pal_int_open(&pal_int, "pciemac")) < 0) {
        fprintf(stderr, "pal_int_open failed %d\n", r);
        exit(1);
    }
    evutil_add_pal_int(EV_DEFAULT_ &pal_int, my_isr, NULL);

    printf("enabling pcie mac intr\n");
    pcieport_intr_enable(0);

    printf("waiting for pcie mac intr\n");
    evutil_run(EV_DEFAULT);

    evutil_remove_pal_int(EV_DEFAULT_ &pal_int);
    pcieport_intr_disable(0);
    pal_int_close(&pal_int);
    exit(0);
}
