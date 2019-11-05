/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "platform/pal/include/pal.h"

#include "cap_top_csr_defines.h"
#include "cap_intr_c_hdr.h"

#include "intrutils.h"

#define NWORDS(a)       (sizeof(a) / sizeof(u_int32_t))

#define INTR_BASE               CAP_ADDR_BASE_INTR_INTR_OFFSET

#define INTR_MSIXCFG_OFFSET     CAP_INTR_CSR_DHS_INTR_MSIXCFG_BYTE_OFFSET
#define INTR_MSIXCFG_BASE       (INTR_BASE + INTR_MSIXCFG_OFFSET)
#define INTR_MSIXCFG_STRIDE     0x10

#define INTR_FWCFG_OFFSET       CAP_INTR_CSR_DHS_INTR_FWCFG_BYTE_OFFSET
#define INTR_FWCFG_BASE         (INTR_BASE + INTR_FWCFG_OFFSET)
#define INTR_FWCFG_STRIDE       0x8

#define INTR_DRVCFG_OFFSET      CAP_INTR_CSR_DHS_INTR_DRVCFG_BYTE_OFFSET
#define INTR_DRVCFG_BASE        (INTR_BASE + INTR_DRVCFG_OFFSET)
#define INTR_DRVCFG_STRIDE      0x20

#define INTR_PBA_CFG_OFFSET     CAP_INTR_CSR_DHS_INTR_PBA_CFG_BYTE_OFFSET
#define INTR_PBA_CFG_BASE       (INTR_BASE + INTR_PBA_CFG_OFFSET)
#define INTR_PBA_CFG_STRIDE     0x4

#define INTR_PBA_OFFSET         CAP_INTR_CSR_DHS_INTR_PBA_ARRAY_BYTE_OFFSET
#define INTR_PBA_BASE           (INTR_BASE + INTR_PBA_OFFSET)
#define INTR_PBA_STRIDE         0x8

#define INTR_ASSERT_OFFSET      CAP_INTR_CSR_DHS_INTR_ASSERT_BYTE_OFFSET
#define INTR_ASSERT_BASE        (INTR_BASE + INTR_ASSERT_OFFSET)
#define INTR_ASSERT_STRIDE      0x4
#define INTR_ASSERT_DATA        0x00000001 /* in little-endian */

#define INTR_STATE_OFFSET       CAP_INTR_CSR_DHS_INTR_STATE_BYTE_OFFSET
#define INTR_STATE_BASE         (INTR_BASE + INTR_STATE_OFFSET)
#define INTR_STATE_STRIDE       0x10

u_int64_t
intr_msixcfg_addr(const int intrb)
{
    return INTR_MSIXCFG_BASE + (intrb * INTR_MSIXCFG_STRIDE);
}

u_int32_t
intr_msixcfg_size(const int intrc)
{
    return intrc * INTR_MSIXCFG_STRIDE;
}

u_int64_t
intr_fwcfg_addr(const int intrb)
{
    return INTR_FWCFG_BASE + (intrb * INTR_FWCFG_STRIDE);
}

u_int64_t
intr_drvcfg_addr(const int intrb)
{
    return INTR_DRVCFG_BASE + (intrb * INTR_DRVCFG_STRIDE);
}

u_int32_t
intr_drvcfg_size(const int intrc)
{
    return intrc * INTR_DRVCFG_STRIDE;
}

u_int64_t
intr_assert_addr(const int intr)
{
    return INTR_ASSERT_BASE + (intr * INTR_ASSERT_STRIDE);
}

u_int32_t
intr_assert_stride(void)
{
    return INTR_ASSERT_STRIDE;
}

u_int32_t
intr_assert_data(void)
{
    return INTR_ASSERT_DATA;
}

u_int64_t
intr_pba_addr(const int lif)
{
    return INTR_PBA_BASE + (lif * INTR_PBA_STRIDE);
}

u_int32_t
intr_pba_size(const int intrc)
{
    return intrc > 0 ? INTR_PBA_STRIDE : 0;
}

u_int64_t
intr_pba_cfg_addr(const int lif)
{
    return INTR_PBA_CFG_BASE + (lif * INTR_PBA_CFG_STRIDE);
}

u_int64_t
intr_state_addr(const int intr)
{
    return INTR_STATE_BASE + (intr * INTR_STATE_STRIDE);
}

void
intr_pba_cfg(const int lif, const int intrb, const size_t intrc)
{
    u_int64_t pa = intr_pba_cfg_addr(lif);
    union {
        struct {
            u_int32_t start:12;
            u_int32_t count:6;
            u_int32_t ecc:6;
            u_int32_t pad:8;
        } __attribute__((packed));
        u_int32_t w[1];
    } __attribute__((packed)) v = {
        .start = intrb,
        .count = MIN(intrc, 64) - 1,
    };

    pal_reg_wr32(pa, v.w[0]);
}

void
intr_drvcfg(const int intr,
            const int mask, const int coal_init, const int mask_on_assert)
{
    u_int64_t pa = intr_drvcfg_addr(intr);

    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask), 1);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, coal_init), coal_init);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask_on_assert), mask_on_assert);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, coal_curr), 0);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask), mask);
}

/*
 * Set the drvcfg_mask for this interrupt resource.
 * Return the previous value of the mask so caller can
 * restore to previous value if desired.
 */
int
intr_drvcfg_mask(const int intr, const int on)
{
    const u_int64_t pa = intr_drvcfg_addr(intr);
    const int omask = pal_reg_rd32(pa + offsetof(intr_drvcfg_t, mask));

    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask), on);
    return omask;
}

void
intr_pba_clear(const int intr)
{
    const u_int64_t pa = intr_drvcfg_addr(intr);
    u_int32_t omask, credits;

    omask = intr_fwcfg_function_mask(intr, 1);
    {
        credits = pal_reg_rd32(pa + offsetof(intr_drvcfg_t, int_credits));
        pal_reg_wr32(pa + offsetof(intr_drvcfg_t, int_credits), credits);
    }
    intr_fwcfg_function_mask(intr, omask);
}

void
intr_msixcfg(const int intr,
             const u_int64_t msgaddr, const u_int32_t msgdata, const int vctrl)
{
    const u_int64_t pa = intr_msixcfg_addr(intr);

    pal_reg_wr64(pa + offsetof(intr_msixcfg_t, msgaddr), msgaddr);
    pal_reg_wr32(pa + offsetof(intr_msixcfg_t, msgdata), msgdata);
    pal_reg_wr32(pa + offsetof(intr_msixcfg_t, vector_ctrl), vctrl);
}

/*
 * Configure the fwcfg register group.  This register group is
 * under fw control (hence the name) and not visible to the host.
 *
 * Note:  We are careful to make config changes to fwcfg only with
 * the function_mask set.  Masking the interrupt will deassert the
 * interrupt if asserted in legacy mode, then we change any config,
 * then re-enable with the new config.  Subsequent interrupts
 * will re-assert with the new config.
 */
void
intr_fwcfg(const int intr,
           const int lif,
           const int port,
           const int legacy,
           const int intpin,
           const int fmask)
{
    const u_int64_t pa = intr_fwcfg_addr(intr);
    intr_fwcfg_t v = {
        .function_mask = 1, /* masked while making updates, then set */
        .lif = lif,
        .port_id = port,
        .local_int = 0,
        .legacy = legacy,
        .int_pin = intpin,
    };

    /* mask via function_mask while making changes */
    intr_fwcfg_function_mask(intr, 1);
    {
        pal_reg_wr32w(pa, v.w, NWORDS(v.w));
    }
    intr_fwcfg_function_mask(intr, fmask);
}

/*
 * Short-cut for configuring an interrupt resource in MSI mode.
 */
void
intr_fwcfg_msi(const int intr, const int lif, const int port)
{
    const int legacy = 0;
    const int intpin = 0;
    const int fmask = 0;

    intr_fwcfg(intr, lif, port, legacy, intpin, fmask);
}

/*
 * Short-cut for configuring an interrupt resource in legacy mode.
 */
void
intr_fwcfg_legacy(const int intr,
                  const int lif, const int port, const int intpin)
{
    const int legacy = 1;
    const int fmask = 0;

    intr_fwcfg(intr, lif, port, legacy, intpin, fmask);
}

/*
 * Set the function_mask for this interrupt resource.
 * Return the previous value of the mask so caller can
 * restore to previous value if desired.
 */
int
intr_fwcfg_function_mask(const int intr, const int on)
{
    const u_int64_t pa = intr_fwcfg_addr(intr);
    const int omask = pal_reg_rd32(pa); /* function_mask word[0] of fwcfg */
    pal_reg_wr32(pa, on);
    return omask;
}

/*
 * Set an interrupt resource in "local" mode which makes the
 * message address to be a local address, otherwise the
 * message address is a host address.  Set to local for interrupts
 * to be sent to the local CPU interrupt controller.
 */
void
intr_fwcfg_local(const int intr, const int on)
{
    const u_int64_t pa = intr_fwcfg_addr(intr);
    intr_fwcfg_t v;
    int omask;

    /* mask via function_mask while making changes */
    omask = intr_fwcfg_function_mask(intr, 1);
    {
        pal_reg_rd32w(pa, v.w, NWORDS(v.w));
        v.local_int = on;
        pal_reg_wr32w(pa, v.w, NWORDS(v.w));
    }
    intr_fwcfg_function_mask(intr, omask);
}

/*
 * Change the mode of the interrupt between legacy and msi mode.
 *
 * Note:  We are careful to make config changes to fwcfg only with
 * the function_mask set.  Masking the interrupt will deassert the
 * interrupt if asserted in legacy mode, then we change any config,
 * then re-enable with the new config.  If necessary the interrupt
 * will re-assert with the new config.
 */
void
intr_fwcfg_mode(const int intr, const int legacy, const int fmask)
{
    const u_int64_t pa = intr_fwcfg_addr(intr);
    intr_fwcfg_t v;

    /* mask via function_mask while making changes */
    intr_fwcfg_function_mask(intr, 1);
    {
        pal_reg_rd32w(pa, v.w, NWORDS(v.w));
        v.legacy = legacy;
        pal_reg_wr32w(pa, v.w, NWORDS(v.w));
    }
    intr_fwcfg_function_mask(intr, fmask);
}

int
intr_config_local_msi(const int intr, u_int64_t msgaddr, u_int32_t msgdata)
{
    /* lif,port unused for local intrs */
    intr_fwcfg_msi(intr, 0, 0);
    /* allow local interrupt destination */
    intr_fwcfg_local(intr, 1);
    /* set msgaddr/data, unmask at msixcfg */
    intr_msixcfg(intr, msgaddr, msgdata, 0);
    /* default drvcfg settings, unmasked */
    intr_drvcfg(intr, 0, 0, 0);

    return 0;
}

void
intr_state_get(const int intr, intr_state_t *st)
{
    const u_int64_t pa = intr_state_addr(intr);

    pal_reg_rd32w(pa, st->w, NWORDS(st->w));
}

void
intr_state_set(const int intr, intr_state_t *st)
{
    const u_int64_t pa = intr_state_addr(intr);

    pal_reg_wr32w(pa, st->w, NWORDS(st->w));
}

/*****************************************************************
 * Reset section
 */

/*****************
 * pba
 */

/*
 * Reset this interrupt's contribution to the interrupt status
 * Pending Bit Array (PBA).  We clear the PBA bit for this interrupt
 * resource by returning all the "credits" for the interrupt.
 *
 * The driver interface to return credits is drvcfg.int_credits,
 * but that register has special semantics where the value written
 * to this register is atomically subtracted from the current value.
 * We could use this interface to read the value X then write X back
 * to the register to X - X = 0.  This works even for negative values
 * since (-X) - (-X) = 0.
 *
 * The "intr_state" alias of int_credits intr_state.drvcfg_int_credits
 * does not have the special "subtract" semantics.  The value written to
 * intr_state.drvcfg_int_credits replaces the current value.  We just
 * set intr_state.drvcfg_int_credits = 0 here to reset int_credits
 * and clear any PBA bit for this interrupt.
 *
 * We mask via function_mask while adjusting credits.
 * This is probably overly conservative for this case.
 */
static void
reset_pba(const int intr)
{
    intr_state_t st;
    u_int32_t omask;

    /* mask via function_mask while making changes */
    omask = intr_fwcfg_function_mask(intr, 1);
    {
        intr_state_get(intr, &st);
        st.drvcfg_int_credits = 0;
        intr_state_set(intr, &st);
    }
    intr_fwcfg_function_mask(intr, omask);
}

void
intr_reset_pba(const int intrb, const int intrc)
{
    int intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        reset_pba(intr);
    }
}

/*****************
 * drvcfg
 */

/*
 * Reset the drvcfg register group.  This register group is often
 * mapped to the host through the bar (e.g. eth, accel) so the driver
 * can manage the settings.  For that case we'll leave the interrupt
 * drvcfg.mask set so the resource won't send an interrupt until
 * the driver has initialized and is ready clearing the mask.
 *
 * For some host devices (e.g. nvme) the interrupt resource is *not*
 * mapped through the bar.  When we reset interrupts for this case
 * we want to leave drvcfg.mask clear so we can send the interrupt
 * to the device when we're ready to do so.
 */
static void
reset_drvcfg(const int intr, const int mask)
{
    intr_drvcfg(intr, mask, 0, 0);
}

static void
intr_reset_drvcfg(const int intrb, const int intrc, const int dmask)
{
    int intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        reset_drvcfg(intr, dmask);
    }
}

/*****************
 * msixcfg
 */

/*
 * Reset the msix control register group.  This group is usually
 * owned by the host OS and the behavior, including these reset values,
 * are specified by the PCIe spec.
 */
static void
reset_msixcfg(const int intr)
{
    /* clear msg addr/data, vector_ctrl mask=1 */
    intr_msixcfg(intr, 0, 0, 1);
}

static void
intr_reset_msixcfg(const int intrb, const int intrc)
{
    int intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        reset_msixcfg(intr);
    }
}

/*****************
 * intr mode
 */

/*
 * Reset the interrupt "mode" to "legacy".
 */
static void
reset_mode(const int intr)
{
    /* reset to legacy mode, no fmask (CMD.int_disable == 0) */
    intr_fwcfg_mode(intr, 1, 0);
}

static void
intr_reset_mode(const int intrb, const int intrc)
{
    int intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        reset_mode(intr);
    }
}

/*****************
 * external reset apis
 */

void
intr_reset_pci(const int intrb, const int intrc, const int dmask)
{
    intr_reset_msixcfg(intrb, intrc);
    intr_reset_mode(intrb, intrc);
}

void
intr_reset_dev(const int intrb, const int intrc, const int dmask)
{
    intr_reset_drvcfg(intrb, intrc, dmask);
    intr_reset_pba(intrb, intrc);
}

/*
 * Default hw config for INTX message needs adjustment
 * for correct operation.
 */
static void
intr_cfg_legacy_intx(void)
{
    const u_int64_t pa =
        (INTR_BASE + CAP_INTR_CSR_CFG_LEGACY_INTX_PCIE_MSG_HDR_BYTE_ADDRESS);
    u_int32_t w[4];

    pal_reg_rd32w(pa, w, NWORDS(w));
    w[0] = 0x34;
    pal_reg_wr32w(pa, w, NWORDS(w));
}

/*
 * One-time hardware initialization.
 */
void
intr_hwinit(void)
{
    intr_cfg_legacy_intx();
    if (!pal_is_asic()) {
        intr_coal_set_resolution(83);
    } else {
        /* set 3.0us resolution */
        intr_coal_set_resolution(2500);
    }
}
