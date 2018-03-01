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

static u_int64_t
notify_int_addr(void)
{
    return (CAP_ADDR_BASE_PXB_PXB_OFFSET +
            CAP_PXB_CSR_CFG_TGT_REQ_NOTIFY_INT_BYTE_OFFSET);
}

static void
notify_int_set(const u_int64_t addr, const u_int32_t data)
{
    req_int_set(notify_int_addr(), addr, data);
}

static void
notify_int_get(u_int64_t *addrp, u_int32_t *datap)
{
    req_int_get(notify_int_addr(), addrp, datap);
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
    notify_int_set(pa, 1);

    maxents = NOTIFY_NENTRIES;

    /*
     * ring_size must be a power-of-2.
     * Find the largest power-of-2 that is <= maxents.
     */
    for (ring_size = 1; ring_size; ring_size <<= 1) {
        if ((ring_size << 1) > maxents) break;
    }
    /*
     * Limit default notify ring size on fpga to 256 to software
     * doesn't get to far behind the ring causing pcie bus timeouts.
     */
    if (ring_size > 0x100) {
        ring_size = 0x100;
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
pciehw_notify(pciehw_port_t *p, notify_entry_t *nentry)
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
                     nentry->info.is_ur       ? 'u' : '-',
                     nentry->info.is_ca       ? 'a' : '-',
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
            p->notcfgrd++;
            break;
        case PCIE_STLP_CFGWR:
        case PCIE_STLP_CFGWR1:
            pciehw_cfgwr_notify(phwdev, stlp, spmt);
            p->notcfgwr++;
            break;
        case PCIE_STLP_MEMRD:
        case PCIE_STLP_MEMRD64:
            pciehw_barrd_notify(phwdev, stlp, spmt);
            p->notmemrd++;
            break;
        case PCIE_STLP_MEMWR:
        case PCIE_STLP_MEMWR64:
            pciehw_barwr_notify(phwdev, stlp, spmt);
            p->notmemwr++;
            break;
        case PCIE_STLP_IORD:
            pciehw_barrd_notify(phwdev, stlp, spmt);
            p->notiord++;
            break;
        case PCIE_STLP_IOWR:
            pciehw_barwr_notify(phwdev, stlp, spmt);
            p->notiowr++;
            break;
        default:
            p->notunknown++;
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

    p->notify_cnt += pici_delta;
    if (pici_delta > p->notify_max) {
        p->notify_max = pici_delta;
    }

    endidx = notify_ring_inc(pi, 1, ring_mask);
    for (i = notify_ring_inc(ci, 1, ring_mask);
         i != endidx;
         i = notify_ring_inc(i, 1, ring_mask)) {

#if 1
        notify_entry_t nentry;

        /* XXX avoid an alignment crash on "io mem" */
        pciehw_memcpy(&nentry, &notify_ring[i], sizeof(nentry));
        pciehw_notify(p, &nentry);
#else
        pciehw_notify(p, &notify_ring[i]);
#endif
        /* return some slots occasionally while processing */
        if ((i & 0xff) == 0) {
            notify_set_ci(port, i);
        }
    }

    /* we consumed these, adjust ci */
    notify_set_ci(port, pi);
    phwmem->notify_intr_dest = 0;
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
    u_int32_t v;
    u_int64_t addr;
    int i;
    const int w = 20;

    notify_get_ring_size(phw, &v);
    pciehsys_log("%-*s : 0x%08x\n", w, "ring_size", v);
    v = pal_reg_rd32(NOTIFY_EN);
    pciehsys_log("%-*s : 0x%08x\n", w, "notify_en", v);

    notify_int_get(&addr, &v);
    pciehsys_log("%-*s : 0x%08"PRIx64"\n", w, "notify_int_addr", addr);
    pciehsys_log("%-*s : 0x%08x\n", w, "notify_int_data", v);
    pciehsys_log("%-*s : 0x%08x\n", w,
                 "notify_intr_dest", phwmem->notify_intr_dest);

    pciehsys_log("%-*s : %d\n", w, "notify_verbose", notify_verbose);
    pciehsys_log("%-*s : %d\n", w, "skip_notify", skip_notify);

    pciehsys_log("%-4s %-11s %5s %5s %4s %4s\n",
                 "port", "ring_base", "pi", "ci", "max", "cnt");
    for (i = 0; i < phw->nports; i++) {
        pciehw_port_t *p = &phwmem->port[i];
        u_int64_t ring_base;
        int pi, ci;

        notify_get_ring_base(phw, i, &ring_base);
        notify_get_pici(i, &pi, &ci);

        pciehsys_log("%-4d 0x%09"PRIx64" %5d %5d %4d %4"PRId64"\n",
                     i, ring_base, pi, ci, p->notify_max, p->notify_cnt);
    }
}

//
// notify
// notify <mask>
//
void
pciehw_notify_dbg(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    int opt, port;

    port = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "fp:r:s:v")) != -1) {
        switch (opt) {
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

    notify_show();
}
