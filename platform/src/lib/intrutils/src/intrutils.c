/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "cap_top_csr_defines.h"
#include "cap_intr_c_hdr.h"

#include "pal.h"
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

#define INTR_COALESCE_OFFSET    CAP_INTR_CSR_DHS_INTR_COALESCE_BYTE_OFFSET
#define INTR_COALESCE_BASE      (INTR_BASE + INTR_COALESCE_OFFSET)
#define INTR_COALESCE_STRIDE    0x8

#define INTR_STATE_OFFSET       CAP_INTR_CSR_DHS_INTR_STATE_BYTE_OFFSET
#define INTR_STATE_BASE         (INTR_BASE + INTR_STATE_OFFSET)
#define INTR_STATE_STRIDE       0x10

u_int64_t
intr_msixcfg_addr(const int intr)
{
    return INTR_MSIXCFG_BASE + (intr * INTR_MSIXCFG_STRIDE);
}

u_int64_t
intr_fwcfg_addr(const int intr)
{
    return INTR_FWCFG_BASE + (intr * INTR_FWCFG_STRIDE);
}

u_int64_t
intr_drvcfg_addr(const int intr)
{
    return INTR_DRVCFG_BASE + (intr * INTR_DRVCFG_STRIDE);
}

u_int64_t
intr_assert_addr(const int intr)
{
    return INTR_ASSERT_BASE + (intr * INTR_ASSERT_STRIDE);
}

u_int64_t
intr_pba_addr(const int lif)
{
    return INTR_PBA_BASE + (lif * INTR_PBA_STRIDE);
}

u_int64_t
intr_pba_cfg_addr(const int lif)
{
    return INTR_PBA_CFG_BASE + (lif * INTR_PBA_CFG_STRIDE);
}

void
intr_pba_cfg(const int lif, const int intr_start, const size_t intr_count)
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
        .start = intr_start,
        .count = intr_count,
    };

    pal_reg_wr32(pa, v.w[0]);
}

void
intr_drvcfg(const int intr)
{
    u_int64_t pa = intr_drvcfg_addr(intr);

    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask), 1);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, coal_init), 0);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask_on_assert), 0);
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, coal_curr), 0);
}

void
intr_pba_clear(const int intr)
{
    const u_int64_t pa = intr_drvcfg_addr(intr);
    u_int32_t omask, credits;

    omask = pal_reg_rd32(pa + offsetof(intr_drvcfg_t, mask));
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask), 1);
    {
        credits = pal_reg_rd32(pa + offsetof(intr_drvcfg_t, int_credits));
        pal_reg_wr32(pa + offsetof(intr_drvcfg_t, int_credits), credits);
    }
    pal_reg_wr32(pa + offsetof(intr_drvcfg_t, mask), omask);
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
 * then re-enable with the new config.  If necessary the interrupt
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
