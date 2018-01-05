/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimdev/src/dev_utils.h"
#include "src/sim/libsimdev/src/simdev_impl.h"

u_int64_t
lif_qstate_map_addr(const int lif)
{
#define PR_BASE                 0x6000000
#define LIF_QSTATE_MAP_OFFSET   0x0128000   
#define LIF_QSTATE_MAP_BASE     (PR_BASE + LIF_QSTATE_MAP_OFFSET)
#define LIF_QSTATE_MAP_STRIDE   0x10
#define LIF_QSTATE_MAP_COUNT    0x800

    return LIF_QSTATE_MAP_BASE + (lif * LIF_QSTATE_MAP_STRIDE);
}

#define INTR_BASE               0x6000000

#define INTR_MSIXCFG_OFFSET     0x0010000
#define INTR_MSIXCFG_BASE       (INTR_BASE + INTR_MSIXCFG_OFFSET)
#define INTR_MSIXCFG_STRIDE     0x10

#define INTR_FWCFG_OFFSET       0x0020000
#define INTR_FWCFG_BASE         (INTR_BASE + INTR_FWCFG_OFFSET)
#define INTR_FWCFG_STRIDE       0x8

#define INTR_DRVCFG_OFFSET      0x0040000
#define INTR_DRVCFG_BASE        (INTR_BASE + INTR_DRVCFG_OFFSET)
#define INTR_DRVCFG_STRIDE      0x20

#define INTR_PBA_CFG_OFFSET     0x0060000
#define INTR_PBA_CFG_BASE       (INTR_BASE + INTR_PBA_CFG_OFFSET)
#define INTR_PBA_CFG_STRIDE     0x4

#define INTR_PBA_OFFSET         0x0064000
#define INTR_PBA_BASE           (INTR_BASE + INTR_PBA_OFFSET)
#define INTR_PBA_STRIDE         0x8

#define INTR_ASSERT_OFFSET      0x0068000
#define INTR_ASSERT_BASE        (INTR_BASE + INTR_ASSERT_OFFSET)
#define INTR_ASSERT_STRIDE      0x4

#define INTR_COALESCE_OFFSET    0x006C000
#define INTR_COALESCE_BASE      (INTR_BASE + INTR_COALESCE_OFFSET)
#define INTR_COALESCE_STRIDE    0x8

#define INTR_STATE_OFFSET       0x0070000
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

u_int64_t
db_host_addr(const int lif)
{
#define DB_HOST_BASE            (0x00400000)
#define DB_HOST_STRIDE          0x40

    return DB_HOST_BASE + (lif * DB_HOST_STRIDE);
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
        };
        u_int32_t w[1];
    } PACKED v = {
        .start = intr_start,
        .count = intr_count,
    };

    simdev_write_reg(pa, v.w[0]);
}

typedef struct intr_drvcfg {
    u_int32_t coal_init;
    u_int32_t mask;
    u_int32_t int_credits;
    u_int32_t mask_on_assert;
    u_int32_t coal_curr;
} PACKED intr_drvcfg_t;

void
intr_drvcfg(const int intr)
{
    u_int64_t pa = intr_drvcfg_addr(intr);

    simdev_write_reg(pa + offsetof(intr_drvcfg_t, mask), 1);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, coal_init), 0);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, mask_on_assert), 0);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, coal_curr), 0);
}

void
intr_drvcfg_unmasked(const int intr)
{
    u_int64_t pa = intr_drvcfg_addr(intr);

    simdev_write_reg(pa + offsetof(intr_drvcfg_t, mask), 0);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, coal_init), 0);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, mask_on_assert), 0);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, coal_curr), 0);
}

void
intr_pba_clear(const int intr)
{
    const u_int64_t pa = intr_drvcfg_addr(intr);
    u_int32_t omask = 0, credits = 0;

    simdev_read_reg (pa + offsetof(intr_drvcfg_t, mask), &omask);
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, mask), 1);
    {
        simdev_read_reg (pa + offsetof(intr_drvcfg_t, int_credits), &credits);
        simdev_write_reg(pa + offsetof(intr_drvcfg_t, int_credits), credits);
    }
    simdev_write_reg(pa + offsetof(intr_drvcfg_t, mask), omask);
}

typedef struct intr_msixcfg {
    u_int64_t msgaddr;
    u_int32_t msgdata;
    u_int32_t vector_ctrl;
} PACKED intr_msixcfg_t;

void
intr_msixcfg(const int intr,
             const u_int64_t msgaddr, const u_int32_t msgdata, const int vctrl)
{
    const u_int64_t pa = intr_msixcfg_addr(intr);
    u_int32_t w[2];

    w[0] = msgaddr;
    w[1] = msgaddr >> 32;
    simdev_write_regs(pa + offsetof(intr_msixcfg_t, msgaddr), w, 2);
    simdev_write_reg (pa + offsetof(intr_msixcfg_t, msgdata), msgdata);
    simdev_write_reg (pa + offsetof(intr_msixcfg_t, vector_ctrl), vctrl);
}

typedef union intr_fwcfg {
    struct {
        u_int32_t function_mask:1;
        u_int32_t rsrv:31;
        u_int32_t lif:11;
        u_int32_t port_id:3;
        u_int32_t local_int:1;
        u_int32_t legacy:1;
        u_int32_t int_pin:2;
        u_int32_t rsrv2:14;
    };
    u_int32_t w[2];
} PACKED intr_fwcfg_t;

void
intr_fwcfg_msi(const int intr, const int lif, const int port_id)
{
    const u_int64_t pa = intr_fwcfg_addr(intr);
    union intr_fwcfg v = {
        .function_mask = 1, /* masked while making updates, then unmasked */
        .lif = lif,
        .port_id = port_id,
        .local_int = 0,
        .legacy = 0,
        .int_pin = 0,
    };
    
    /* mask via function_mask while making changes */
    simdev_write_reg(pa, 1); /* function_mask = 1 */
    {
        simdev_write_regs(pa, v.w, 2);
    }
    simdev_write_reg(pa, 0); /* function_mask = 0 */
}

int
msixtbl_rd(const int intrb,
           const u_int64_t offset,
           const u_int8_t size,
           u_int64_t *valp)
{
    const u_int64_t base = intr_msixcfg_addr(intrb);
    u_int32_t val;

    simdev_log("msixtbl read offset 0x%"PRIx64" size %d\n", offset, size);
    if (size != 4 && size != 8) {
        simdev_error("msixtbl read size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("msixtbl read size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

int
msixtbl_wr(const int intrb,
           const u_int64_t offset,
           const u_int8_t size,
           const u_int64_t val)
{
    const u_int64_t base = intr_msixcfg_addr(intrb);

    simdev_log("msixtbl write offset 0x%"PRIx64" size %d\n", offset, size);
    if (size != 4 && size != 8) {
        simdev_error("msixtbl write size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("msixtbl write size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_write_reg(base + offset, val) < 0) {
        return -1;
    }
    return 0;
}

int
msixpba_rd(const int lif,
           const u_int64_t offset,
           const u_int8_t size,
           u_int64_t *valp)
{
    const u_int64_t base = intr_pba_addr(lif);
    u_int32_t val;

    simdev_log("msixpba read offset 0x%"PRIx64" "
               "pba_base 0x%"PRIx64" size %d\n",
               offset, base, size);
    if (size != 4 && size != 8) {
        simdev_error("msixpba read size %d invalid, ignoring\n", size);
        return -1;
    }
    if (size == 8) { /* XXX */
        simdev_error("msixpba read size %d unimplemented\n", size);
        assert(0);
    }
    if (simdev_read_reg(base + offset, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

int
msixpba_wr(const int lif,
           const u_int64_t offset,
           const u_int8_t size,
           const u_int64_t val)
{
    /* msixpba is read-only */
    simdev_error("msixpba_wr: off 0x%"PRIx64" size %d = val 0x%"PRIx64"\n",
                 offset, size, val);
    return -1;
}
