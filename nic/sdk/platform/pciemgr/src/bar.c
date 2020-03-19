/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

/* bar show flags */
#define BARSF_NONE      0x0
#define BARSF_VERBOSE   0x1

static pciehwbar_t *
pciehw_bar_get(pciehwdev_t *phwdev, const int idx)
{
    if (idx < 0 || idx > 7) return NULL;
    if (idx == 7) return &phwdev->rombar;
    return &phwdev->bar[idx];
}

static pciehwbar_t *
pciehw_bar_find(const char *name, const int idx)
{
    pciehwdev_t *phwdev = pciehwdev_find_by_name(name);
    return phwdev ? pciehw_bar_get(phwdev, idx) : NULL;
}

static char *
bar_type_str(pciehwbartype_t bartype)
{
    static char *bartypestr[8] = {
        "NONE0", "mem", "mem64", "NONE3", "NONE4", "io", "NONE6", "NONE7"
    };
    return bartypestr[bartype & 0x7];
}

static pciehwbartype_t
bar_type_to_hwbar_type(pciehbartype_t bartype)
{
    pciehwbartype_t hwbartype;

    switch (bartype) {
    case PCIEHBARTYPE_MEM:   hwbartype = PCIEHWBARTYPE_MEM;   break;
    case PCIEHBARTYPE_MEM64: hwbartype = PCIEHWBARTYPE_MEM64; break;
    case PCIEHBARTYPE_IO:    hwbartype = PCIEHWBARTYPE_IO;    break;
    case PCIEHBARTYPE_NONE:
    default:
        pciesys_logerror("unexpected bartype: %d\n", bartype);
        assert(0);
        break;
    }
    return hwbartype;
}

static int
bar_finalize(const pciehwdev_t *phwdev,
             const pciehbar_t *bar,
             pciehwbar_t *phwbar)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehbarreg_t *reg;
    pciehw_spmt_t *spmt;
    int i, pmtb, pmtc;

    pmtc = bar->nregs;
    pmtb = pmt_alloc(pmtc);
    assert(pmtb >= 0);

    phwbar->type = bar_type_to_hwbar_type(bar->type);
    phwbar->cfgidx = bar->cfgidx;
    phwbar->size = bar->size;
    phwbar->pmtb = pmtb;
    phwbar->pmtc = pmtc;
    phwbar->valid = 1;

    spmt = &pshmem->spmt[phwbar->pmtb];
    for (reg = bar->regs, i = 0; i < bar->nregs; i++, reg++, spmt++) {
        pciehw_sprt_t *sprt;
        prt_t *prt;
        int j, prtb, prtc;

        spmt->owner = hwdevh;
        spmt->baroff = reg->baroff;
        spmt->cfgidx = bar->cfgidx;

        prtc = reg->nprts;
        prtb = prt_alloc(prtc);
        assert(prtb >= 0);

        prt = reg->prts;
        sprt = &pshmem->sprt[prtb];
        for (j = 0; j < prtc; j++, prt++, sprt++) {
            sprt->prt = *prt;
        }

        assert(reg->pmt.pmre.bar.valid);
        spmt->pmt = reg->pmt;
        pmt_bar_set_bdf(&spmt->pmt, phwdev->bdf);
        pmt_bar_set_prts(&spmt->pmt, prtb, prtc);
    }
    return 0;
}

int
pciehw_bars_finalize(pciehdev_t *pdev)
{
    pciehwdev_t *phwdev = pciehdev_get_hwdev(pdev);
    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    pciehbar_t *bar;

    for (bar = pciehbars_get_first(pbars);
         bar;
         bar = pciehbars_get_next(pbars, bar)) {
        pciehwbar_t *phwbar;

        assert(bar->cfgidx < PCIEHW_NBAR);
        phwbar = &phwdev->bar[bar->cfgidx];
        bar_finalize(phwdev, bar, phwbar);
    }
    bar = pciehbars_get_rombar(pbars);
    if (bar != NULL) {
        bar_finalize(phwdev, bar, &phwdev->rombar);
    }
    return 0;
}

u_int64_t
pciehw_bar_getsize(pciehwbar_t *phwbar)
{
    if (!phwbar->valid) return 0;
    return phwbar->size;
}

void
pciehw_bar_setaddr(pciehwbar_t *phwbar, const u_int64_t addr)
{
    pciehw_pmt_setaddr(phwbar, addr);
}

void
pciehw_bar_load(pciehwbar_t *phwbar)
{
    pciehw_pmt_load_bar(phwbar);
}

void
pciehw_bar_enable(pciehwbar_t *phwbar, const int on)
{
    pciehw_pmt_enable_bar(phwbar, on);
}

int
pciehw_bar_init(void)
{
    pmt_init();
    return 0;
}

static void
pciehw_barrw_notify(const pciehdev_event_t evtype,
                    pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const tlpauxinfo_t *info,
                    const pciehw_spmt_t *spmt)
{
    pciehdev_eventdata_t evd;
    pciehdev_memrw_notify_t *memrw;

    memset(&evd, 0, sizeof(evd));
    evd.evtype = evtype;
    evd.port = phwdev->port; /* XXX port in, rather than from phwdev? */
    evd.lif = phwdev->lifb;
    memrw = &evd.memrw_notify;
    memrw->baraddr = stlp->addr;
    memrw->cfgidx = spmt->cfgidx;
    memrw->baroffset = stlp->addr - pmt_bar_getaddr(&spmt->pmt);
    memrw->size = stlp->size;
    memrw->localpa = info->direct_addr;
    memrw->data = stlp->data; /* data, if write */
    pciehw_event(phwdev, &evd);
}

void
pciehw_barrd_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const tlpauxinfo_t *info,
                    const pciehw_spmt_t *spmt)
{
    pciehw_barrw_notify(PCIEHDEV_EV_MEMRD_NOTIFY, phwdev, stlp, info, spmt);
}

void
pciehw_barwr_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const tlpauxinfo_t *info,
                    const pciehw_spmt_t *spmt)
{
    pciehw_barrw_notify(PCIEHDEV_EV_MEMWR_NOTIFY, phwdev, stlp, info, spmt);
}

void
pciehw_barrd_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp)
{
#if 0
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t pmti = ientry->info.pmti;
    const pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    const pciehwdevh_t hwdevh = spmt->owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);
#endif
    u_int64_t pa = ientry->info.direct_addr;
    size_t sz = ientry->info.direct_size;

    pal_mem_rd(pa, ientry->data, sz, 0);
    pciehw_indirect_complete(ientry);
}

void
pciehw_barwr_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp)
{
#if 0
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t pmti = ientry->info.pmti;
    const pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    const pciehwdevh_t hwdevh = spmt->owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);
#endif
    u_int64_t pa = ientry->info.direct_addr;
    size_t sz = ientry->info.direct_size;

    pal_mem_wr(pa, &stlp->data, sz, 0);
    pciehw_indirect_complete(ientry);
}

/******************************************************************
 * debug
 */

static void
bar_pmt_get(const int pmti, pmt_t *pmt, prt_t *prt)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];

    /*
     * If loaded, grab hardware copy,
     * else use shadow copy.
     */
    if (spmt->loaded) {
        pmt_get(pmti, pmt);
    } else {
        *pmt = spmt->pmt;
    }

    if (prt) {
        const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
        const int prti = pmr->prtb;
        pciehw_sprt_t *sprt = &pshmem->sprt[prti];
        if (spmt->loaded) {
            prt_get(prti, prt);
        } else {
            *prt = sprt->prt;
        }
    }
}

static void
bar_show_prt(const int pmti,
             const int prti,
             const int prtsize,
             const u_int64_t baraddr)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    pciehw_sprt_t *sprt = &pshmem->sprt[prti];
    prt_t lprt, *prt;
    char vfstr[16];

    /*
     * If loaded, grab hardware copy,
     * else use shadow copy.
     */
    if (spmt->loaded) {
        prt_get(prti, &lprt);
        prt = &lprt;
    } else {
        prt = &sprt->prt;
    }

    if (prt->cmn.vfstride >= 2) {
        snprintf(vfstr, sizeof(vfstr), "%d", 1 << prt->cmn.vfstride);
    } else {
        vfstr[0] = '\0';
    }

    switch (prt_type(prt)) {
    case PRT_TYPE_RES: {
        prt_res_t *r = &prt->res;

        pciesys_loginfo("    %-4d %-5s %c%c%c%c  %-5s 0x%013" PRIx64 " "
                        "0x%09" PRIx64 "%s%s\n",
                        prti,
                        prt_type_str(r->type),
                        r->indirect ? 'i' : '-',
                        r->notify   ? 'n' : '-',
                        r->pmvdis   ? 'p' : '-',
                        r->aspace   ? 'h' : '-',
                        human_readable(prt_size_decode(r->sizedw)),
                        baraddr,
                        (u_int64_t)r->addrdw << 2,
                        vfstr[0] ? "             " : "",
                        vfstr);
        break;
    }
    case PRT_TYPE_DB16:
    case PRT_TYPE_DB32: {
        prt_db_t *db = &prt->db;
        const int prtsz = prtsize;

        pciesys_loginfo("    %-4d %-5s %c%c    %-5s 0x%013" PRIx64 " "
                        "LIF=%d idx=%2d:%-2d qid=%2d:%-2d%s%s\n",
                        prti,
                        prt_type_str(db->type),
                        db->indirect ? 'i' : '-',
                        db->notify   ? 'n' : '-',
                        human_readable(prtsz),
                        baraddr,
                        db->lif,
                        db->idxshift * 8,
                        db->idxwidth ? db->idxshift * 8 + db->idxwidth - 1 : 0,
                        db->qidshift * 8,
                        db->qidwidth ? db->qidshift * 8 + db->qidwidth - 1 : 0,
                        vfstr[0] ? " " : "",
                        vfstr);
        break;
    }
    case PRT_TYPE_DB64: {
        prt_db_t *db = &prt->db;
        const int prtsz = 64;

        pciesys_loginfo("    %-4d %-5s %c%c    %-5s 0x%013" PRIx64 " "
                        "LIF=%d%s%s\n",
                        prti,
                        prt_type_str(db->type),
                        db->indirect ? 'i' : '-',
                        db->notify   ? 'n' : '-',
                        human_readable(prtsz),
                        baraddr,
                        db->lif,
                        vfstr[0] ? "             " : "",
                        vfstr);
        break;
    }
    default:
        break;
    }
}

static void
bar_show_prts(const int pmti, const pmt_t *pmt, const u_int32_t flags)
{
    const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const int prtb = pmr->prtb;
    const int prtc = pmr->prtc;
    const int prtsize = 1 << pmr->prtsize;
    const u_int64_t baraddr = pmt_bar_getaddr(pmt);
    int i, prti;

    for (prti = prtb, i = 0; i < prtc; i++, prti++) {
        const u_int64_t baroff = (const u_int64_t)i * prtsize;
        bar_show_prt(pmti, prti, prtsize, baraddr + baroff);
    }
}

static int
bitspan(const int bits, const int bite)
{
    return bite > bits ? bite - bits : 0;
}

static void
bar_address_format_addr(const pmt_t *pmt, const prt_t *prt, char *line)
{
    pmt_datamask_t dm;
    int i;

    pmt_entry_dec(&pmt->pmte, &dm);
    for (i = 0; i < 64; i++) {
        const u_int64_t bitdw = (1ULL << i) >> 2;

        /*
         * dword address so lower 2 bits are "don't care".
         * If mask bit is set then we match the value in data bit.
         * Otherwise we "don't care" and match any address.
         */
        if (i < 2) {
            line[i] = '.';
        } else if (dm.mask.bar.addrdw & bitdw) {
            line[i] = dm.data.bar.addrdw & bitdw ? '1' : '0';
        } else {
            line[i] = '.';
        }
    }
}

/*
 * ........vvvvrrrr....
 *        ^---^---^
 *        | | | | +-------- PMR.prtsize: lsb of resource
 *        | | | +-----| (r)
 *        | | +------------ PMR.vfstart: lsb of vf
 *        | +---------| (v)
 *        +---------------- PMR.vfend: msb+1 of vf
 *
 * (r) resource PRT index
 * (v) VF index beyond vfbase (with vfstride)
 */
static void
bar_address_format_res(const pmt_t *pmt, const prt_t *prt, char *line)
{
    const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const u_int32_t resb = pmr->prtsize;
    const u_int32_t resc = bitspan(resb, pmr->vfstart);
    const u_int32_t vfb = pmr->vfstart;
    /* vf is used only if vfstride is >= 2 */
    const u_int32_t vfc = prt->res.vfstride >= 2 ? bitspan(vfb, pmr->vfend):0;

    bar_address_format_addr(pmt, prt, line);
    memset(&line[resb], 'r', resc);
    memset(&line[vfb],  'v', vfc);
}

static void
bar_address_format_db(const pmt_t *pmt, const prt_t *prt, char *line,
                      const u_int32_t qtyb, const u_int32_t qtym,
                      const u_int32_t qidb, const u_int32_t qidc)
{
    const pmr_bar_entry_t *r = &pmt->pmre.bar;
    const u_int32_t pagesize = pmt_bar_get_pagesize(pmt);
    const u_int32_t prtb = r->prtsize;
    const u_int32_t resb = prtb;
    const u_int32_t resc = bitspan(resb, r->vfstart);
    const u_int32_t pidb = ffs(pagesize) - 1;
    const u_int32_t pidc = bitspan(pidb, prtb);
    const u_int32_t lifb = prtb;
    const u_int32_t lifc = bitspan(lifb, r->vfstart);
    const u_int32_t vfb = r->vfstart;
    const u_int32_t vfc = bitspan(vfb, r->vfend);
    int i;

    bar_address_format_addr(pmt, prt, line);

    /* qtype is a 3-bit field selecting up to 8 types */
    for (i = 0; i < 3; i++) {
        if (qtym & (1 << i)) line[qtyb + i] = 't';
    }

    memset(&line[qidb], 'q', qidc);
    memset(&line[resb], 'r', resc);
    memset(&line[pidb], 'p', pidc);
    memset(&line[lifb], 'L', lifc);
    memset(&line[vfb],  'v', vfc);
}

/*
 * ....vvvvllllpppp......ttt...
 *    ^---^---^---^      --^
 *    | | | | | | |       |+------ PMR.qtype_start: lsb of qtype (qtypemask)
 *    | | | | | | |       +--| (t)
 *    | | | | | | +--------------- PMR.pagesize: lsb of pid
 *    | | | | | +------------| (p)
 *    | | | | +------------------- PMR.prtsize: lsb of lif
 *    | | | +----------------| (l)
 *    | | +----------------------- PMR.vfstart: lsb of vf
 *    | +--------------------| (v)
 *    +--------------------------- PMR.vfend: msb+1 of vf
 *
 * (t) qtype (with qtypemask)
 * (p) pid
 * (l) lif
 * (v) VF index beyond vfbase (with vfstride)
 */
static void
bar_address_format_db64(const pmt_t *pmt, const prt_t *prt, char *line)
{
    const u_int32_t qtyb = pmt->pmre.bar.qtypestart;
    const u_int32_t qtym = pmt->pmre.bar.qtypemask;
    const u_int32_t qidb = 0; /* qid is in data (not bar addr) for 64b db */
    const u_int32_t qidc = 0;

    bar_address_format_db(pmt, prt, line, qtyb, qtym, qidb, qidc);
}

/*
 * db16 or db32, the "small" doorbells, aka dbsm.
 * PRT.qidsel, PRT.qidshift and PRT.qidwidth are *only*
 * used by db16/db32.
 *
 * ....vvvv.........qqqq...tt.
 *    ^---^        ^---^   -^
 *    | | |        | | |   ||
 *    | | |        | | |   |+------ PMR.qtypestart (qtypemask)
 *    | | |        | | |   +--| (t)
 *    | | |        | | +----------- PRT.qidshift (if qidsel=1)
 *    | | |        | +--------| (q)
 *    | | |        +--------------- PRT.qidshift + PRT.qidwidth
 *    | | +------------------------ PMR.vfstart: lsb of vf
 *    | +---------------------| (v)
 *    +---------------------------- PMR.vfend: msb+1 of vf
 *
 * (t) qtype (with qtypemask)
 * (q) qid in addr (if qidsel=1) else qid is in data
 * (v) VF index beyond vfbase (with vfstride)
 */
static void
bar_address_format_dbsm(const pmt_t *pmt, const prt_t *prt, char *line)
{
    const u_int32_t qtyb = pmt->pmre.bar.qtypestart;
    const u_int32_t qtym = pmt->pmre.bar.qtypemask;
    u_int32_t qidb, qidc;

    if (prt->db.qidsel) {
        /* these address bits are used as qid */
        qidb = pmt->pmre.bar.qidstart;
        qidc = pmt->pmre.bar.qidend - qidb;
    } else {
        /* qid is encoded in data */
        qidb = 0;
        qidc = 0;
    }
    bar_address_format_db(pmt, prt, line, qtyb, qtym, qidb, qidc);
}

static void
bar_address_format(const pmt_t *pmt)
{
    prt_t lprt, *prt;
    char linele[65], linebe[65];
    int i;

    assert(pmt->pmre.bar.prtc > 0);
    prt = &lprt;
    prt_get(pmt->pmre.bar.prtb, prt);

    pciesys_loginfo("+--------------------------------"
                    "--------------------------------+\n");
    pciesys_loginfo("|   6         5         4        "
                    " 3         2         1          |\n");
    pciesys_loginfo("|32109876543210987654321098765432"
                    "10987654321098765432109876543210|\n");

    switch (prt_type(prt)) {
    case PRT_TYPE_RES:
        bar_address_format_res(pmt, prt, linele);
        break;
    case PRT_TYPE_DB64:
        bar_address_format_db64(pmt, prt, linele);
        break;
    case PRT_TYPE_DB32:
    case PRT_TYPE_DB16:
        bar_address_format_dbsm(pmt, prt, linele);
        break;
    default:
        memset(linele, '.', sizeof(linele));
        break;
    }
    /* flip around to big endian format for more natural view of addr */
    for (i = 0; i < 64; i++) {
        linebe[i] = linele[63 - i];
    }
    linebe[64] = '\0';
    pciesys_loginfo("|%s|\n", linebe);

    pciesys_loginfo("+--------------------------------"
                    "--------------------------------+\n");
}

static void
bar_show_pmt(const char *label,
             const int pmti, const pciehwbar_t *phwbar, const u_int32_t flags)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    pmt_t lpmt, *pmt = &lpmt;
    prt_t lprt, *prt = &lprt;
    pmt_datamask_t dm;
    pmr_bar_entry_t *pmr;
    char vfstr[16];

    bar_pmt_get(pmti, pmt, prt);
    pmr = &pmt->pmre.bar;
    pmt_entry_dec(&pmt->pmte, &dm);

    if (flags & BARSF_VERBOSE) {
        bar_address_format(pmt);

        pciesys_loginfo("vfe(V4) %d vfs(V3) %d prt_size(V2) %d pagesz(V1) %d "
                        "qide(V7) %d qids(V6) %d\n",
                        pmr->vfend - 1,
                        pmr->vfstart,
                        pmr->prtsize ,
                        ffs(pmt_bar_get_pagesize(pmt)) - 1,
                        pmr->qidend,
                        pmr->qidstart);
    }

    if (prt->res.vfstride >= 2) {
        snprintf(vfstr, sizeof(vfstr), " %d-%d", pmr->vfbase, pmr->vflimit);
    } else {
        vfstr[0] = '\0';
    }
    pciesys_loginfo("%-3s %-4d %-5s %c%c%c   "
                    "%-5s 0x%013" PRIx64 " %1d:%s %-3d "
                    "%-4" PRIu64 " %-4" PRIu64 "%s\n",
                    label,
                    pmti,
                    bar_type_str(dm.data.bar.type),
                    pmt_allows_rd(pmt) ? 'r' : '-',
                    pmt_allows_wr(pmt) ? 'w' : '-',
                    spmt->loaded ? 'l' : '-',
                    human_readable(pmt_bar_getsize(pmt)),
                    pmt_bar_getaddr(pmt),
                    dm.data.bar.port,
                    bdf_to_str(pciehw_hostbdf(dm.data.bar.port, pmr->bdf)),
                    dm.data.bar.tblid,
                    spmt->swrd,
                    spmt->swwr,
                    vfstr);

    bar_show_prts(pmti, pmt, flags);
}

static void
bar_show_bar(const char *label,
             const pciehwbar_t *phwbar, const u_int32_t flags)
{
    int i, pmti;

    pmti = phwbar->pmtb;
    for (i = 0; i < phwbar->pmtc; i++, pmti++) {
        bar_show_pmt(label, pmti, phwbar, flags);
    }
}

static void
bar_show_dev(pciehwdev_t *phwdev, const u_int32_t flags)
{
    pciehwbar_t *phwbar;
    int i, header;
    char label[4];

    header = 0;
    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid) {
            if (!header) {
                pciesys_loginfo("%s:\n", pciehwdev_get_name(phwdev));
                header = 1;
            }
            snprintf(label, sizeof(label), "%d", i);
            bar_show_bar(label, phwbar, flags);
        }
    }
    if (phwdev->rombar.valid) {
        bar_show_bar("rom", &phwdev->rombar, flags);
    }
    if (header) pciesys_loginfo("\n");
}

static void
bar_show_hdr(void)
{
    pciesys_loginfo("%-3s %-4s %-5s %-5s "
                    "%-5s %-15s %-9s %-3s "
                    "%-4s %-4s %-s\n",
                    "BAR", "PMT", "TYPE", "FLAGS",
                    "SIZE", "BAR ADDR", "P:BB:DD.F", "TID",
                    "SWRD", "SWWR", "VF");

    pciesys_loginfo("    %-4s %-5s %-5s "
                    "%-5s %-15s %-11s "
                    "            %-s\n",
                    "PRT", "TYPE", "FLAGS",
                    "SIZE", "BAR ADDR", "CAPRI ADDR",
                    "VFSTRIDE");
}

void
pciehw_bar_show(int argc, char *argv[])
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    int opt, idx, i;
    char *name;
    pciehwdev_t *phwdev;
    u_int32_t flags;

    idx = -1;
    name = NULL;
    flags = BARSF_NONE;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:b:v")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        case 'b':
            idx = strtoul(optarg, NULL, 0);
            break;
        case 'v':
            flags |= BARSF_VERBOSE;
            break;
        default:
            return;
        }
    }

    /*
     * No device name specified, show all devs
     */
    if (name == NULL ) {
        bar_show_hdr();
        phwdev = &pshmem->dev[1];
        for (i = 1; i <= pshmem->allocdev; i++, phwdev++) {
            bar_show_dev(phwdev, flags);
        }
    } else {
        phwdev = pciehwdev_find_by_name(name);
        if (phwdev == NULL) {
            pciesys_logerror("device %s not found\n", name);
            return;
        }
        /*
         * No idx given, show all bars of this device,
         * else show only the specified bar idx.
         */
        if (idx == -1) {
            bar_show_hdr();
            bar_show_dev(phwdev, flags);
        } else {
            pciehwbar_t *phwbar;
            char label[4];

            phwbar = pciehw_bar_get(phwdev, idx);
            if (phwbar == NULL) {
                pciesys_logerror("device %s bar %d not found\n", name, idx);
                return;
            }
            if (!phwbar->valid) {
                pciesys_logerror("device %s bar %d invalid\n", name, idx);
                return;
            }

            bar_show_hdr();
            snprintf(label, sizeof(label), "%d", idx);
            bar_show_bar(label, phwbar, flags);
        }
    }
}

static void
cmd_show(int argc, char *argv[])
{
    pciehw_bar_show(argc, argv);
}

/*
 * setaddr -d debug0 -b 4 0x12345678
 */
static void
cmd_setaddr(int argc, char *argv[])
{
    int opt, idx;
    char *name;
    pciehwbar_t *phwbar;
    u_int64_t baraddr;

    idx = -1;
    name = NULL;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:b:")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        case 'b':
            idx = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (name == NULL || idx == -1 || optind >= argc) {
        pciesys_logerror("Usage: setaddr -d <devname> -b <baridx> <addr>\n");
        return;
    }

    baraddr = strtoull(argv[optind], NULL, 0);

    phwbar = pciehw_bar_find(name, idx);
    if (phwbar == NULL) {
        pciesys_logerror("setaddr: %s bar %d not found\n", name, idx);
        return;
    }
    if (!phwbar->valid) {
        pciesys_logerror("setaddr: %s bar %d invalid\n", name, idx);
    }

    pciehw_bar_setaddr(phwbar, baraddr);
}

/*
 * load -d debug0 -b 4
 */
static void
cmd_load(int argc, char *argv[])
{
    pciehwdev_t *phwdev;
    pciehwbar_t *phwbar;
    int opt, idx;
    char *name;

    idx = -1;
    name = NULL;
    optind = 0;
    while ((opt = getopt(argc, argv, "d:b:")) != -1) {
        switch (opt) {
        case 'd':
            name = optarg;
            break;
        case 'b':
            idx = strtoul(optarg, NULL, 0);
            break;
        default:
            return;
        }
    }

    if (name == NULL || idx == -1) {
        pciesys_logerror("Usage: load -d <devname> -b <baridx>\n");
        return;
    }

    phwdev = pciehwdev_find_by_name(name);
    if (phwdev == NULL) {
        pciesys_logerror("load: %s not found\n", name);
        return;
    }
    phwbar = pciehw_bar_get(phwdev, idx);
    if (phwbar == NULL) {
        pciesys_logerror("load: %s bar %d not found\n", name, idx);
        return;
    }
    if (!phwbar->valid) {
        pciesys_logerror("load: %s bar %d invalid\n", name, idx);
    }

    pciehw_bar_load(phwbar);
}

typedef struct cmd_s {
    const char *name;
    void (*f)(int argc, char *argv[]);
    const char *usage;
    const char *helpstr;
} cmd_t;

static cmd_t cmdtab[] = {
#define CMDENT(name, usage, helpstr) \
    { #name, cmd_##name, usage, helpstr }
    CMDENT(show, "[-d <dev> [-b <bar>]]", ""),
    CMDENT(setaddr, "-d <dev> -b <bar> <addr>", ""),
    CMDENT(load, "-d <dev> -b <bar> <addr>", ""),
    { NULL, NULL }
};

static cmd_t *
cmd_lookup(cmd_t *cmdtab, const char *name)
{
    cmd_t *c;

    for (c = cmdtab; c->name; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

void
pciehw_bar_dbg(int argc, char *argv[])
{
    cmd_t *c;

    if (argc <= 1) {
        pciesys_loginfo("usage: %s <cmd>\n", argv[0]);

        for (c = cmdtab; c->name; c++) {
            pciesys_loginfo("    %-8s %s\n", c->name, c->usage);
        }
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciesys_logerror("%s: debug command not found\n", argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}
