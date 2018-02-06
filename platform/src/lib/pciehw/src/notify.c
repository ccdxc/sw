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
#include "pcietlp.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "notify.h"

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

static int notify_verbose;
static int skip_notify = 1;

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
notify_get_ring_base(pciehw_t *phw, const int port, u_int64_t *rbp)
{
    const u_int64_t base = pal_reg_rd32(req_notify_addr(port));
    *rbp = base << 16;
}

static void
notify_set_ring_base(pciehw_t *phw, const int port, u_int64_t rb)
{
    const u_int64_t notify_addr = req_notify_addr(port);
    assert((rb & 0xffff) == 0);
    pal_reg_wr32(notify_addr, rb >> 16);
}

static void
notify_get_pici(const int port, int *pip, int *cip)
{
    const u_int32_t pici = pal_reg_rd32(notify_addr(port));

    *pip = pici & 0xffff;
    *cip = pici >> 16;
}

static void
notify_get_masked_pici(const int port, int *pip, int *cip,
                       const u_int32_t ring_mask)
{
    int pi, ci;

    notify_get_pici(port, &pi, &ci);
    *pip = pi & ring_mask;
    *cip = ci & ring_mask;
}

/*
 * NOTE: The hw doesn't allow sw to write to PI,
 * when we write to the NOTIFY register only the CI is updated.
 * To reset to empty ring, set CI = PI.
 */
static void
notify_set_ci(const int port, const int ci)
{
    const u_int32_t pici = (ci << 16);
    pal_reg_wr32(notify_addr(port), pici);
}

/*
 * reset pi, ci - The hw doesn't allow sw to write to PI,
 * when we write to the NOTIFY register only the CI is updated.
 * To reset to empty ring, set CI = PI.
 */
static void
notify_reset_pici(const int port)
{
    int pi, ci;

    notify_get_pici(port, &pi, &ci);
    notify_set_ci(port, pi);    /* set ci = pi to reset */
}

static u_int32_t
notify_pici_delta(const int pi, const int ci)
{
    return abs(pi - ci);
}

static void
notify_enable(const u_int32_t mask)
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
        } __attribute__((packed));
        u_int32_t w;
    } en = { 0 };

    en.w = mask;
    pal_reg_wr32(NOTIFY_EN, en.w);
}

static void
notify_int_set(pciehw_t *phw, const u_int64_t addr, const u_int32_t data)
{
    union {
        struct {
            u_int32_t data:32;
            u_int32_t addrdw_lo:32;
            u_int32_t addrdw_hi:2;
        } __attribute__((packed));
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
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    u_int64_t pa;

    pa = pal_mem_vtop(&phwmem->notify_area[port]);
    notify_set_ring_base(phw, port, pa);
    notify_reset_pici(port);
}

static void
notify_init(pciehw_t *phw)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
     u_int64_t pa;
    u_int32_t maxents, ring_size;

    pa = pal_mem_vtop(&phwmem->notify_intr_dest);
    notify_int_set(phw, pa, 1);

    maxents = NOTIFY_NENTRIES;

    /*
     * ring_size must be a power-of-2.
     * Find the largest power-of-2 that is <= maxents.
     */
    for (ring_size = 1; ring_size; ring_size <<= 1) {
        if ((ring_size << 1) > maxents) break;
    }
    if (ring_size) {
        phwmem->notify_ring_mask = ring_size - 1;
        notify_set_ring_size(phw, phwmem->notify_ring_mask);
    }
}

static int
notify_ring_inc(const int idx, const int inc, const u_int32_t ring_mask)
{
    return (idx + inc) & ring_mask;
}

static void
pciehw_notify(notify_entry_t *nentry)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_spmt_t *spmt = phwmem->spmt;
    u_int32_t pmti = nentry->info.pmti;
    pciehwdevh_t hwdevh = spmt[pmti].owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);
    pcie_stlp_t stlpbuf, *stlp = &stlpbuf;

    pcietlp_decode(stlp, nentry->rtlp, sizeof(nentry->rtlp));

    if (notify_verbose) {
        pciehsys_log("%s\n", pcietlp_str(stlp));
        pciehsys_log("  %s: pmti %d %c%c%c%c%c%c addr 0x%08"PRIx64"\n",
                     pciehwdev_get_name(phwdev),
                     pmti,
                     nentry->info.is_notify   ? 'n' : '-',
                     nentry->info.pmt_hit     ? 'p' : '-',
                     nentry->info.is_direct   ? 'd' : '-',
                     nentry->info.is_indirect ? 'i' : '-',
                     nentry->info.is_UR       ? 'u' : '-',
                     nentry->info.is_CA       ? 'a' : '-',
                     (u_int64_t)nentry->info.direct_addr);
    }

    /*
     * If info.pmt_hit means we hit an entry we installed
     * in the PMT for a reason.  Go process the transaction.
     *
     * If !info.pmt then perhaps this is an exception or error.
     */
    if (nentry->info.pmt_hit) {
        switch (stlp->type) {
        case PCIE_STLP_CFGRD:
        case PCIE_STLP_CFGRD1:
            pciehw_cfgrd_notify(phwdev, stlp, spmt);
            break;
        case PCIE_STLP_CFGWR:
        case PCIE_STLP_CFGWR1:
            pciehw_cfgwr_notify(phwdev, stlp, spmt);
            break;
        case PCIE_STLP_MEMRD:
            pciehw_barrd_notify(phwdev, stlp, spmt);
            break;
        case PCIE_STLP_MEMWR:
            pciehw_barwr_notify(phwdev, stlp, spmt);
            break;
        }
    }
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
        pciehw_port_skip_notify(i, skip_notify);
        notify_ring_init(phw, i);
    }
    return 0;
}

static int
pciehw_notify_intr(pciehw_t *phw, const int port)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_port_t *p = &phwmem->port[port];
    const u_int32_t ring_mask = phwmem->notify_ring_mask;
    notify_entry_t *notify_ring = (notify_entry_t *)phwmem->notify_area[port];
    int pi, ci, i, endidx;
    u_int32_t pici_delta;

    notify_get_masked_pici(port, &pi, &ci, ring_mask);

    if (ci == pi) return -1;

    pici_delta = notify_pici_delta(pi, ci);
    if (pici_delta > p->notify_max) {
        p->notify_max = pici_delta;
    }

    endidx = notify_ring_inc(pi, 1, ring_mask);
    for (i = notify_ring_inc(ci, 1, ring_mask);
         i != endidx;
         i = notify_ring_inc(i, 1, ring_mask)) {

        pciehw_notify(&notify_ring[i]);
    }

    /* we consumed these, adjust ci */
    notify_set_ci(port, pi);
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
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
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
    pciehsys_log("%-*s : %d\n", w, "notify_verbose", notify_verbose);

    pciehsys_log("%-4s %-11s %5s %5s %4s\n",
                 "port", "ring_base", "pi", "ci", "max");
    for (i = 0; i < phw->nports; i++) {
        pciehw_port_t *p = &phwmem->port[i];
        u_int64_t ring_base;
        int pi, ci;

        notify_get_ring_base(phw, i, &ring_base);
        notify_get_pici(i, &pi, &ci);

        pciehsys_log("%-4d 0x%09"PRIx64" %5d %5d %4d\n",
                     i, ring_base, pi, ci, p->notify_max);
    }
}

static void
notify_dev(const char *devname, const int on)
{
    pciehwdev_t *phwdev = pciehwdev_find_by_name(devname);

    if (phwdev == NULL) {
        pciehsys_log("%s: not found\n", devname);
        return;
    }
    pciehw_pmt_set_notify(phwdev, on);
}

//
// notify
// notify -e eth0
// notify -d eth0
// notify <mask>
//
void
pciehw_notify_dbg(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    int opt, do_enable, do_disable;
    int port;
    char *devname;

    do_enable = 0;
    do_disable = 0;
    port = 0;
    devname = NULL;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:efp:r:s:v")) != -1) {
        switch (opt) {
        case 'd':
            devname = optarg;
            do_enable = 0;
            do_disable = 1;
            break;
        case 'e':
            devname  = optarg;
            do_enable = 1;
            do_disable = 0;
            break;
        case 'f': /* flush */
            notify_reset_pici(port);
            break;
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        case 'r':
            phwmem->notify_ring_mask = strtoul(optarg, NULL, 0);
            notify_set_ring_size(phw, phwmem->notify_ring_mask);
            break;
        case 's': /* skip_notify_if_qfull */
            skip_notify = strtoul(optarg, NULL, 0) != 0;
            pciehw_port_skip_notify(port, skip_notify);
            break;
        case 'v':
            notify_verbose = !notify_verbose;
            break;
        default:
            return;
        }
    }

    if (optind < argc) {
        const u_int32_t enable_mask = strtoull(argv[optind], NULL, 0);
        notify_enable(enable_mask);
        return;
    }

    if (do_enable) {
        notify_dev(devname, 1);
    } else if (do_disable) {
        notify_dev(devname, 0);
    } else {
        notify_show();
    }
}
