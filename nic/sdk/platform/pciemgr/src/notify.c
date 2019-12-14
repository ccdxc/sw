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

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgr/include/pciemgr.h"
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
notify_int_get(u_int64_t *addrp, u_int32_t *datap)
{
    req_int_get(notify_int_addr(), addrp, datap);
}

static void
notify_get_ring_size(u_int32_t *ring_size)
{
    *ring_size = pal_reg_rd32(NOTIFY_RING_SIZE);
}

static void
notify_set_ring_size(const u_int32_t ring_size)
{
    pal_reg_wr32(NOTIFY_RING_SIZE, ring_size);
}

static void
notify_get_ring_base(const int port, u_int64_t *rbp)
{
    const u_int64_t base = pal_reg_rd32(req_notify_addr(port));
    *rbp = base << 16;
}

static void
notify_set_ring_base(const int port, u_int64_t rb)
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
notify_pici_delta(const int pi, const int ci, const u_int32_t ring_mask)
{
    if (pi > ci) {
        return pi - ci;
    } else {
        return pi + ring_mask + 1 - ci;
    }
}

static void
notify_set_enable(const u_int32_t mask)
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
    } en;

    en.w = pal_reg_rd32(NOTIFY_EN);
    en.w = mask;
    pal_reg_wr32(NOTIFY_EN, en.w);
}

static void
notify_enable(void)
{
    notify_set_enable(0x3fff); /* enable all sources */
}

static void
notify_disable(void)
{
    notify_set_enable(0); /* disable all sources */
}

static void
notify_ring_init(const int port)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    u_int64_t pa;

    pa = pal_mem_vtop(&phwmem->notify_area[port]);
    notify_set_ring_base(port, pa);
    notify_reset_pici(port);
}

static void
notify_init(void)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    u_int32_t maxents, ring_size;

    maxents = NOTIFY_NENTRIES;

    /*
     * ring_size must be a power-of-2.
     * Find the largest power-of-2 that is <= maxents.
     */
    for (ring_size = 1; ring_size; ring_size <<= 1) {
        if ((ring_size << 1) > maxents) break;
    }
    /*
     * Limit default notify ring size on fpga to 256 so software
     * doesn't get too far behind the ring causing pcie bus timeouts.
     */
    if (!pal_is_asic() && ring_size > 0x100) {
        ring_size = 0x100;
    }
    if (ring_size) {
        pshmem->notify_ring_mask = ring_size - 1;
        notify_set_ring_size(pshmem->notify_ring_mask);
    }
}

static int
notify_ring_inc(const int idx, const int inc, const u_int32_t ring_mask)
{
    return (idx + inc) & ring_mask;
}

static void
handle_notify(pciehw_port_t *p, notify_entry_t *nentry)
{
    const tlpauxinfo_t *info = &nentry->info;

    /*
     * If info->indirect_reason == 0 means we hit an entry we installed
     * in the PMT for indirect handling.  Go process the transaction.
     *
     * If info->indirect_reason != 0 then perhaps
     * this is an exception or error.  Track reason code stats.
     */
    if (info->indirect_reason == 0) {
        pciehw_shmem_t *pshmem = pciehw_get_shmem();
        const u_int32_t pmti = info->pmti;
        pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
        const pciehwdevh_t hwdevh = spmt->owner + info->vfid;
        pciehwdev_t *phwdev = pciehwdev_get(hwdevh);
        pcie_stlp_t stlpbuf, *stlp = &stlpbuf;

        pcietlp_decode(stlp, nentry->rtlp, sizeof(nentry->rtlp));

        if (pshmem->notify_verbose) {
            pciesys_loginfo("%s\n", pcietlp_str(stlp));
            pciesys_loginfo("  %s: pmti %d %c%c%c%c%c%c addr 0x%08"PRIx64"\n",
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

        switch (stlp->type) {
        case PCIE_STLP_CFGRD:
        case PCIE_STLP_CFGRD1:
            pciehw_cfgrd_notify(phwdev, stlp, info, spmt);
            spmt->swrd++;
            p->stats.not_cfgrd++;
            break;
        case PCIE_STLP_CFGWR:
        case PCIE_STLP_CFGWR1:
            pciehw_cfgwr_notify(phwdev, stlp, info, spmt);
            spmt->swwr++;
            p->stats.not_cfgwr++;
            break;
        case PCIE_STLP_MEMRD:
        case PCIE_STLP_MEMRD64:
            pciehw_barrd_notify(phwdev, stlp, info, spmt);
            spmt->swrd++;
            p->stats.not_memrd++;
            break;
        case PCIE_STLP_MEMWR:
        case PCIE_STLP_MEMWR64:
            pciehw_barwr_notify(phwdev, stlp, info, spmt);
            spmt->swwr++;
            p->stats.not_memwr++;
            break;
        case PCIE_STLP_IORD:
            pciehw_barrd_notify(phwdev, stlp, info, spmt);
            spmt->swrd++;
            p->stats.not_iord++;
            break;
        case PCIE_STLP_IOWR:
            pciehw_barwr_notify(phwdev, stlp, info, spmt);
            spmt->swwr++;
            p->stats.not_iowr++;
            break;
        default:
            p->stats.not_unknown++;
            break;
        }
    } else {
        uint64_t *notify_reasons = &p->stats.notify_reason_stats;
        notify_reasons[info->indirect_reason]++;
    }
}

/******************************************************************
 * apis
 */

int
pciehw_notify_init(void)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    int i;

    /*
     * skip_notify on by default so host doesn't hang
     * if pciemgr is broken
     */
    pshmem->skip_notify = 1;

    notify_init();
    for (i = 0; i < PCIEHW_NPORTS; i++) {
        pciehw_tgt_port_skip_notify(i, pshmem->skip_notify);
        notify_ring_init(i);
    }
    return 0;
}

int
pciehw_notify_intr(const int port)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_port_t *p = &pshmem->port[port];
    const u_int32_t ring_mask = pshmem->notify_ring_mask;
    notify_entry_t *notify_ring = (notify_entry_t *)phwmem->notify_area[port];
    int pi, ci, i, endidx;
    u_int32_t pici_delta;

    p->stats.not_intr++;
    notify_get_masked_pici(port, &pi, &ci, ring_mask);

    if (ci == pi) {
        p->stats.not_spurious++;
        return -1;
    }

    pici_delta = notify_pici_delta(pi, ci, ring_mask);

    p->stats.not_cnt += pici_delta;
    if (pici_delta > p->stats.not_max) {
        p->stats.not_max = pici_delta;
    }

    endidx = notify_ring_inc(pi, 1, ring_mask);
    for (i = notify_ring_inc(ci, 1, ring_mask);
         i != endidx;
         i = notify_ring_inc(i, 1, ring_mask)) {

#ifdef DEVICEMEM_WAR
        notify_entry_t nentry;

        /* XXX avoid an alignment crash on "io mem" */
        pciehw_memcpy(&nentry, &notify_ring[i], sizeof(nentry));
        handle_notify(p, &nentry);
#else
        handle_notify(p, &notify_ring[i]);
#endif
        /* return some slots occasionally while processing */
        if ((i & 0xff) == 0) {
            notify_set_ci(port, i);
        }
    }

    /* we consumed these, adjust ci */
    notify_set_ci(port, pi);
    return 0;
}

/*
 * CFG_TGT_REQ_NOTIFY_INT
 */
int
pciehw_notify_intr_init(const int port, u_int64_t msgaddr, u_int32_t msgdata)
{
    notify_enable();
    return req_int_init(notify_int_addr(), "notify_intr", port,
                        msgaddr, msgdata | 0x80000000);
}

int
pciehw_notify_poll(const int port)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t ring_mask = pshmem->notify_ring_mask;
    int pi, ci;

    notify_get_masked_pici(port, &pi, &ci, ring_mask);
    if (ci != pi) {
        pciehw_notify_intr(port);
    }
    return 0;
}

/*
 * Arrange to have the notify interrupt written to memory,
 * then we can poll memory locations to see if there is work to do.
 */
int
pciehw_notify_poll_init(const int port)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    const u_int64_t msgaddr = pal_mem_vtop(&phwmem->notify_intr_dest[port]);
    const u_int32_t msgdata = 1;

    notify_enable();
    return req_int_init(notify_int_addr(), "notify_intr", port,
                        msgaddr, msgdata);
}

/*
 * Disable any hw notifications.
 */
void
pciehw_notify_disable(const int port)
{
    notify_disable();
}

void
pciehw_notify_disable_all_ports(void)
{
    int port;

    for (port = 0; port < PCIEHW_NPORTS; port++) {
        pciehw_notify_disable(port);
    }
}

/******************************************************************
 * debug
 */

static void
notify_show(void)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    u_int32_t v;
    u_int64_t addr;
    int i;
    const int w = 20;

    notify_get_ring_size(&v);
    pciesys_loginfo("%-*s : 0x%08x\n", w, "ring_size", v);
    v = pal_reg_rd32(NOTIFY_EN);
    pciesys_loginfo("%-*s : 0x%08x\n", w, "notify_en", v);

    notify_int_get(&addr, &v);
    pciesys_loginfo("%-*s : 0x%08"PRIx64"\n", w, "notify_int_addr", addr);
    pciesys_loginfo("%-*s : 0x%08x\n", w, "notify_int_data", v);

    pciesys_loginfo("%-*s : %d\n", w, "notify_verbose",pshmem->notify_verbose);
    pciesys_loginfo("%-*s : %d\n", w, "skip_notify", pshmem->skip_notify);

    pciesys_loginfo("%-4s %-11s %-9s %5s %5s "
                    "%4s %4s\n",
                    "port", "ring_base", "intr_dest", "pi", "ci",
                    "max", "cnt");
    for (i = 0; i < PCIEHW_NPORTS; i++) {
        pciehw_port_t *p = &pshmem->port[i];
        u_int64_t ring_base;
        int pi, ci;

        notify_get_ring_base(i, &ring_base);
        notify_get_pici(i, &pi, &ci);

        pciesys_loginfo("%-4d 0x%09" PRIx64 " "
                        "%9d %5d %5d %4" PRId64 " %4" PRId64 "\n",
                        i, ring_base, phwmem->notify_intr_dest[i],
                        pi, ci, p->stats.not_max, p->stats.not_cnt);
    }
}

//
// notify
//
void
pciehw_notify_dbg(int argc, char *argv[])
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
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
            pshmem->notify_ring_mask = strtoul(optarg, NULL, 0);
            notify_set_ring_size(pshmem->notify_ring_mask);
            break;
        case 's': /* skip_notify_if_qfull */
            pshmem->skip_notify = strtoul(optarg, NULL, 0) != 0;
            pciehw_tgt_port_skip_notify(port, pshmem->skip_notify);
            break;
        case 'v':
            pshmem->notify_verbose = !pshmem->notify_verbose;
            break;
        default:
            return;
        }
    }

    notify_show();
}
