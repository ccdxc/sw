/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "misc.h"
#include "bdf.h"
#include "pal.h"
#include "pciehsys.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"

#define NOTIFY_EN       \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_TGT_NOTIFY_EN_BYTE_OFFSET)
#define NOTIFY_RING_SIZE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + \
     CAP_PXB_CSR_CFG_TGT_REQ_NOTIFY_RING_SIZE_BYTE_OFFSET)
#define NOTIFY_INT \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + \
     CAP_PXB_CSR_CFG_TGT_REQ_NOTIFY_INT_BYTE_OFFSET)
#define NOTIFY_INT_NWORDS 3

#define NOTIFY_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_NOTIFY_BYTE_OFFSET)
#define NOTIFY_STRIDE 4

#define REQ_NOTIFY_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_TGT_REQ_NOTIFY_BYTE_OFFSET)
#define REQ_NOTIFY_STRIDE 4

static u_int64_t
notify_addr(const int port)
{
    return NOTIFY_BASE + (port * NOTIFY_STRIDE);
}

static u_int64_t
req_notify_addr(const int port)
{
    return REQ_NOTIFY_BASE + (port * REQ_NOTIFY_STRIDE);
}

static void
notify_get_ring_size(pciehw_t *phw, u_int32_t *ring_size)
{
    *ring_size = pal_reg_rd32(NOTIFY_RING_SIZE);
}

static void
notify_set_ring_size(pciehw_t *phw, const u_int32_t ring_size)
{
    pal_reg_wr32(NOTIFY_RING_SIZE, ring_size);
}

static void
notify_enable(pciehw_t *phw)
{
    union {
        struct {
            u_int32_t msg:1;
            u_int32_t pmv:1;
            u_int32_t db_pmv:1;
            u_int32_t unsupp:1;
            u_int32_t atomic:1;
            u_int32_t pmt_miss:1;
            u_int32_t pmr_invalid:1;
            u_int32_t prt_invalid:1;
            u_int32_t rc_vfid_miss:1;
            u_int32_t prt_oor:1;
            u_int32_t vfid_oor:1;
            u_int32_t cfg_bdf_oor:1;
            u_int32_t pmr_ecc_err:1;
            u_int32_t prt_ecc_err:1;
        };
        u_int32_t w;
    } en = { 0 };

    en.msg = 1;
    en.pmv = 1;
    en.db_pmv = 1;
    en.unsupp = 1;
    en.atomic = 1;
    en.pmt_miss = 1;
    en.pmr_invalid = 1;
    en.prt_invalid = 1;
    en.rc_vfid_miss = 1;
    en.prt_oor = 1;
    en.vfid_oor = 1;
    en.cfg_bdf_oor = 1;
    en.pmr_ecc_err = 1;
    en.prt_ecc_err = 1;

    pal_reg_wr32(NOTIFY_EN, en.w);
}

static void
notify_disable(pciehw_t *phw)
{
    pal_reg_wr32(NOTIFY_EN, 0);
}

static void
notify_int_set(pciehw_t *phw, const u_int64_t addr, const u_int32_t data)
{
    union {
        struct {
            u_int32_t data:32;
            u_int32_t addrdw_lo:32;
            u_int32_t addrdw_hi:2;
        };
        u_int32_t w[NOTIFY_INT_NWORDS];
    } in;
    const u_int64_t addrdw = addr >> 2;

    in.data = data;
    in.addrdw_lo = addrdw;
    in.addrdw_hi = addrdw >> 32;

    pal_reg_wr32w(NOTIFY_INT, in.w, NOTIFY_INT_NWORDS);
}

static void
notify_ring_init(pciehw_t *phw, const int port)
{
    pciehw_mem_t *phwmem = phw->pciehwmem;
    u_int64_t pa;
    u_int32_t pici;
    int pi, ci;

    pa = pal_mem_vtop(&phwmem->notify_area[port]);
    assert((pa & 0xffff) == 0);
    pal_reg_wr32(req_notify_addr(port), pa >> 16);

    /*
     * reset pi, ci - The hw doesn't allow sw to write to PI,
     * when we write to the NOTIFY register only the CI is updated.
     * To reset to empty ring, set CI = PI.
     */
    pici = pal_reg_rd32(notify_addr(port));
    pi = pici & 0xffff;
    ci = pi;    /* set ci = pi to reset */
    pici = (ci << 16) | pi;
    pal_reg_wr32(notify_addr(port), pici);
}

static void
notify_init(pciehw_t *phw)
{
    pciehw_mem_t *phwmem = phw->pciehwmem;
    u_int64_t pa;
    u_int32_t ring_size;

    pa = pal_mem_vtop(&phwmem->notify_intr_dest);
    notify_int_set(phw, pa, 1);

    // XXX should be sizeof(notify_entry) XXX
    ring_size = sizeof(phwmem->notify_area[0]) / 128;
    notify_set_ring_size(phw, ring_size);
}

/******************************************************************
 * apis
 */

int
pciehw_notify_init(pciehw_t *phw)
{
    int i;

    notify_init(phw);
    for (i = 0; i < phw->nports; i++) {
        notify_ring_init(phw, i);
    }
    return 0;
}

static int
pciehw_notify_intr(pciehw_t *phw, const int port)
{
    //pciehw_mem_t *phwmem = phw->pciehwmem;
    //pciehw_port_t *p = &phwmem->port[port];
    u_int32_t pici;
    int pi, ci;

    pici = pal_reg_rd32(notify_addr(port));
    pi = pici & 0xffff;
    ci = pici >> 16;

    if (ci == pi) return -1;

    return 0;
}

int
pciehw_notify_poll(pciehw_t *phw)
{
    return pciehw_notify_intr(phw, 0);
}

/******************************************************************
 * debug
 */

static void
notify_show(void)
{
    pciehw_t *phw = pciehw_get();
    u_int64_t addrdw;
    u_int32_t v;
    u_int32_t in[NOTIFY_INT_NWORDS];
    int i;
    const int w = 20;

    notify_get_ring_size(phw, &v);
    pciehsys_log("%-*s : 0x%08x\n", w, "ring_size", v);
    v = pal_reg_rd32(NOTIFY_EN);
    pciehsys_log("%-*s : 0x%08x\n", w, "notify_en", v);

    pal_reg_rd32w(NOTIFY_INT, in, NOTIFY_INT_NWORDS);
    addrdw = in[2] & 0x3;
    addrdw <<= 32;
    addrdw |= in[1];
    pciehsys_log("%-*s : 0x%08"PRIx64"\n", w, "int_addr", addrdw << 2);
    pciehsys_log("%-*s : 0x%08x\n", w, "int_data", in[0]);

    pciehsys_log("%-4s %-10s %5s %5s\n", "port", "ring_base", "pi", "ci");
    for (i = 0; i < phw->nports; i++) {
        u_int64_t ring_base;
        int pi, ci;

        v = pal_reg_rd32(req_notify_addr(i));
        ring_base = v << 16;
        v = pal_reg_rd32(notify_addr(i));
        pi = v & 0xffff;
        ci = (v >> 16) & 0xffff;

        pciehsys_log("%-4d 0x%08"PRIx64" %5d %5d\n", i, ring_base, pi, ci);
    }
}

void
pciehw_notify_dbg(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    int opt, do_enable, do_disable;

    do_enable = 0;
    do_disable = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "de")) != -1) {
        switch (opt) {
        case 'd': do_disable = 1; break;
        case 'e': do_enable = 1; break;
        default:
            return;
        }
    }

    if (do_enable) {
        notify_enable(phw);
    }
    if (do_disable) {
        notify_disable(phw);
    }
    notify_show();
}
