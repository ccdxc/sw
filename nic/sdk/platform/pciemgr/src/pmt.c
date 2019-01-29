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
#include <fcntl.h>

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/misc/include/bdf.h"
#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/pciemgr/include/pciehw.h"
#include "nic/sdk/platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

#define PMT_BASE        \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_PMT_BYTE_ADDRESS)
#define PMT_STRIDE      \
    (CAP_PXB_CSR_DHS_TGT_PMT_ENTRY_ARRAY_ELEMENT_SIZE * 4 * 8)
#define PMT_GRST   \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_CFG_TGT_PMT_GRST_BYTE_ADDRESS)
#define PMR_BASE        \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_DHS_TGT_PMR_BYTE_ADDRESS)
#define PMR_STRIDE      \
    CAP_PXB_CSR_DHS_TGT_PMR_ENTRY_BYTE_SIZE

static int
pmt_count(void)
{
    return PMT_COUNT;
}

static void
assert_pmti_in_range(const int pmti)
{
    assert(pmti >= 0 && pmti < pmt_count());
}

static void
assert_pmts_in_range(const int pmtb, const int pmtc)
{
    if (pmtc > 0) {
        assert_pmti_in_range(pmtb);
        assert_pmti_in_range(pmtb + pmtc - 1);
    }
}

static u_int64_t
pmt_addr(const int pmti)
{
    assert_pmti_in_range(pmti);
    return PMT_BASE + (pmti * PMT_STRIDE);
}

static u_int64_t
pmr_addr(const int pmti)
{
    assert_pmti_in_range(pmti);
    return PMR_BASE + (pmti * PMR_STRIDE);
}

int
pmt_alloc(const int n)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    int pmti;

    if (pshmem->allocpmt + n >= pmt_count()) {
        return -1;
    }
    pmti = pshmem->allocpmt;
    pshmem->allocpmt += n;
    return pmti;
}

void
pmt_free(const int pmtb, const int pmtc)
{
    assert_pmts_in_range(pmtb, pmtc);
    /* XXX */
}

static void
pmt_get_entry(const int pmti, pmt_entry_t *pmte)
{
    pal_reg_rd32w(pmt_addr(pmti), pmte->w, PMT_NWORDS);
}

static void
pmt_set_entry(const int pmti, const pmt_entry_t *pmte)
{
    pal_reg_wr32w(pmt_addr(pmti), pmte->w, PMT_NWORDS);
}

static void
pmr_get_entry(const int pmti, pmr_entry_t *pmre)
{
    pal_reg_rd32w(pmr_addr(pmti), pmre->w, PMR_NWORDS);
}

static void
pmr_set_entry(const int pmti, const pmr_entry_t *pmre)
{
    pal_reg_wr32w(pmr_addr(pmti), pmre->w, PMR_NWORDS);
}

/*
 * Retrieve an entry from hardware.
 */
void
pmt_get(const int pmti, pmt_t *pmt)
{
    pmt_get_entry(pmti, &pmt->pmte);
    pmr_get_entry(pmti, &pmt->pmre);
}

/*
 * Install an entry in hardware at the specified index.
 */
void
pmt_set(const int pmti, const pmt_t *pmt)
{
    /*
     * Set PMR entry first, then TCAM, so by the time a tcam search
     * can hit an entry the corresponding ram entry is valid too.
     */
    pmr_set_entry(pmti, &pmt->pmre);
    pmt_set_entry(pmti, &pmt->pmte);
}

static void
pmt_clr_tcam(const int pmti)
{
    pmt_entry_t pmte0;

    memset(&pmte0, 0, sizeof(pmte0));
    pmt_set_entry(pmti, &pmte0);
}

/*
 * Clear a pmt slot.  For now we just invalidate the tcam entry
 * so searches don't hit, but we don't write anything to PMR.
 */
static void
pmt_clr(const int pmti)
{
    pmt_clr_tcam(pmti);
}

static void
pmt_set_cfg(pciehwdev_t *phwdev,
            const int pmti,
            const u_int64_t cfgpa,
            const u_int16_t addr,
            const u_int16_t addrm,
            const u_int8_t romsksel,
            const u_int32_t pmtflags)
{
    pmt_t pmt;

    pmt_cfg_enc(&pmt,
                phwdev->port,
                phwdev->bdf, 0xffff,
                cfgpa, addr, addrm,
                romsksel, phwdev->stridesel,
                pmtflags);
    pmt_set(pmti, &pmt);
}

static void
pmt_grst(void)
{
#ifdef __aarch64__
    pal_reg_wr32(PMT_GRST, 1);
#else
    int pmti;

    for (pmti = 0; pmti < pmt_count(); pmti++) {
        pmt_clr_tcam(pmti);
    }
#endif
}

static void
pmt_pmr_init(void)
{
    const pmr_entry_t pmre0 = { 0 };
    int prti;

    for (prti = 0; prti < pmt_count(); prti++) {
        pmr_set_entry(prti, &pmre0);
    }
}

static void
pmt_reset()
{
    pmt_grst();
    pmt_pmr_init();
}

#if 0
static void
pmt_load_cfg_rcdev(pciehwdev_t *phwdev)
{
    const pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    const u_int64_t cfgpa = 0x800000000ULL; /* rc address */
    const u_int16_t addr = 0;
    const u_int16_t addrm = 0; /* don't care - match any cfg addr */
    const u_int8_t romsksel = 0; /* all writable */
    const u_int8_t notify = 0; /* no notify */
    const u_int8_t indirect = 0; /* no indirect */
    int pmti;
    pmt_t pmt;

    pmti = pmt_alloc(hwdevh);
    assert(pmti >= 0);

    pmt_make_cfg(&pmt, phwdev, cfgpa, addr, addrm, romsksel, notify, indirect);
    pmt.pmre.cfg.vfbase = 200;
    pmt_set(pmti, &pmt);
}

static void
pmt_load_bar_rcdev(pciehwbar_t *phwbar)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t pmti = phwbar->pmti;
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    pciehw_sprt_t *sprt = &pshmem->sprt[spmt->pmt.pmre.bar.prtb];
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);
    pmt_t pmt;

    /*
     * Set res addr to baraddr so full address gets forwarded to RC.
     */
    sprt->resaddr = spmt->baraddr;

    /*
     * Load PRT first, then load PMT so PMT tcam search hit
     * will find valid PRT entries.
     */
    pciehw_prt_load(spmt->pmt.pmre.bar.prtb, spmt->pmt.pmre.bar.prtc);

    //pmt_load_bar(phwbar);
    pmt_make_bar(&pmt, phwdev->port, phwdev->bdf, spmt);
    pmt.pmre.bar.vfbase = 201;
    pmt.pmre.bar.vflimit = 0x7ff; /* make 201 within range */
    pmt_set(pmti, &pmt);

    if (!spmt->loaded) {
        spmt->loaded = 1;
    }
}
#endif

/******************************************************************
 * apis
 */

static void
pmt_set_owner(int pmti, pciehwdevh_t hwdevh)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];

    spmt->owner = hwdevh;
}

int
pciehw_pmt_load_cfg(pciehwdev_t *phwdev)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    const pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    const u_int64_t cfgpa = pal_mem_vtop(&phwmem->cfgcur[hwdevh]);
    int pmti, i;

#if 0
    if (strncmp(pciehwdev_get_name(phwdev), "rcdev", 5) == 0) {
        pmt_load_cfg_rcdev(phwdev);
        return 0;
    }
#endif

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        const int romsk = phwdev->romsksel[i];
        const u_int32_t pmtf = phwdev->cfgpmtf[i];
        if (romsk != ROMSK_RDONLY || pmtf != PMTF_NONE) {
            pmti = pmt_alloc(1);
            if (pmti < 0) {
                pciesys_logerror("load_cfg: pmt_alloc failed i %d\n", i);
                goto error_out;
            }
            pmt_set_owner(pmti, hwdevh);
            pmt_set_cfg(phwdev, pmti, cfgpa, i << 2, 0xffff, romsk, pmtf);
        }
    }
    /*
     * Catchall entry.  We add a read-write entry for all addresses,
     * but romsk=1 selects the read-only entry so effectively this
     * claims all read/write transactions but writes have no effect.
     * We limit the range to PCIEHW_CFGSZ because that is all we have
     * here in cfgpa.  For access to cfgspace above PCIEHW_CFGSZ a wildcard
     * entry for all bdf's will catch it.
     */
    pmti = pmt_alloc(1);
    if (pmti < 0) {
        pciesys_logerror("load_cfg: pmt_alloc failed catchall\n");
        goto error_out;
    }
    pmt_set_owner(pmti, hwdevh);
#if 1
    pmt_set_cfg(phwdev, pmti, cfgpa, 0, 0,
                ROMSK_RDONLY, PMTF_RW);
#else
    pmt_set_cfg(phwdev, pmti, cfgpa, 0, PCIEHW_CFGSZ - 1,
                ROMSK_RDONLY, PMTF_RW);
#endif
    return 0;

 error_out:
    /* XXX pciehw_pmt_unload_cfg(phwdev); */
    return -1;
}

void
pciehw_pmt_load_bar(pciehwbar_t *phwbar)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    u_int32_t pmti = phwbar->pmtb;
    pciehw_spmt_t *spmt = &pshmem->spmt[phwbar->pmtb];
    pciehw_spmt_t *spmte = spmt + phwbar->pmtc;
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);
    const u_int16_t bdf = pciehwdev_get_bdf(phwdev);

#if 0
    if (strncmp(pciehwdev_get_name(phwdev), "rcdev", 5) == 0) {
        pmt_load_bar_rcdev(phwbar);
        return;
    }
#endif

    assert(phwbar->valid);

#ifdef __aarch64__
    pciesys_loginfo("%s: bar %d pmt %d loaded\n",
                    pciehwdev_get_name(phwdev), spmt->cfgidx, pmti);
#endif

    for ( ; spmt < spmte; spmt++, pmti++) {
        /*
         * Load PRT first, then load PMT so PMT tcam search hit
         * will find valid PRT entries.
         */
        pciehw_prt_load(spmt->pmt.pmre.bar.prtb, spmt->pmt.pmre.bar.prtc);

        /* place bus-adjusted bdf in pmt, then load in hw */
        pmt_bar_set_bdf(&spmt->pmt, bdf);
        pmt_set(pmti, &spmt->pmt);

        if (!spmt->loaded) {
            spmt->loaded = 1;
        }
    }
}

void
pciehw_pmt_unload_bar(pciehwbar_t *phwbar)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_spmt_t *spmt  = &pshmem->spmt[phwbar->pmtb];
    pciehw_spmt_t *spmte = spmt + phwbar->pmtc;
    u_int32_t pmti = phwbar->pmtb;

#ifdef __aarch64__
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);
    pciesys_loginfo("%s: bar %d pmt %d unloaded\n",
                    pciehwdev_get_name(phwdev), spmt->cfgidx, pmti);
#endif

    for ( ; spmt < spmte; spmt++, pmti++) {
        /*
         * Unload PMT first THEN PRT, so PMT tcam search will not hit
         * and PRT is unreferenced.  Then safe to unload PRT.
         */
        if (spmt->loaded) {
            pmt_clr(pmti);
            pciehw_prt_unload(spmt->pmt.pmre.bar.prtb,
                              spmt->pmt.pmre.bar.prtc);
            spmt->loaded = 0;
        }
    }
}

void
pciehw_pmt_enable_bar(pciehwbar_t *phwbar, const int on)
{
    pciehdev_params_t *params = pciehw_get_params();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t pmti = phwbar->pmtb;
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];

    if (params->force_bars_load) {
        if (!spmt->loaded) {
            pciehw_pmt_load_bar(phwbar);
        }
    } else if (on && !spmt->loaded) {
        pciehw_pmt_load_bar(phwbar);
    } else if (!on && spmt->loaded) {
        pciehw_pmt_unload_bar(phwbar);
    }
}

void
pciehw_pmt_setaddr(pciehwbar_t *phwbar, const u_int64_t addr)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    u_int32_t pmti = phwbar->pmtb;
    pciehw_spmt_t *spmt = &pshmem->spmt[phwbar->pmtb];
    pciehw_spmt_t *spmte = spmt + phwbar->pmtc;

#ifdef __aarch64__
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);
    pciesys_loginfo("%s: bar %d pmt %d setaddr 0x%" PRIx64 "\n",
                    pciehwdev_get_name(phwdev), spmt->cfgidx, pmti, addr);
#endif

    /* update addr */
    for ( ; spmt < spmte; spmt++, pmti++) {
        pmt_bar_setaddr(&spmt->pmt, addr + spmt->baroff);

        /* if loaded, update hw too */
        if (spmt->loaded) {
            pmt_set(pmti, &spmt->pmt);
        }
    }
}

void
pmt_init(void)
{
    pmt_reset();
    prt_init();
}

/******************************************************************
 * debug
 */

#define PMTT_BAR        0x01
#define PMTT_CFG        0x02
#define PMTT_RAW        0x04

static int last_hdr_displayed;

/*
idx  id typ rw p:00:00.0 0xreg_
                                vfid port 00:00.0 vfst romsk 0xaddress VINTA
 */
static void
pmt_show_cfg_entry_hdr(void)
{
    pciesys_loginfo("%-4s %-2s %-3s %-2s %-9s %-6s "
                    "%-4s %-9s %-4s %-5s %-11s %-5s\n",
                    "idx", "id", "typ", "rw", "p:bb:dd.f", "cfgreg",
                    "vfid", "p:bdflim", "vfst", "romsk",
                    "address", "flags");
}

static void
pmt_show_cfg_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pmt_cfg_format_t *d = &dm->data.cfg;
    const pmt_cfg_format_t *m = &dm->mask.cfg;
    const pmr_cfg_entry_t *r = &pmt->pmre.cfg;
    const int rw = d->rw;
    const int rw_m = m->rw;

    if (last_hdr_displayed != PMTT_CFG) {
        pmt_show_cfg_entry_hdr();
        last_hdr_displayed = PMTT_CFG;
    }

    pciesys_loginfo("%-4d %2d %-3s %c%c %1d:%-7s 0x%04x "
                    "%4d %d:%-7s %4d %5d 0x%09" PRIx64 " %c%c%c%c%c\n",
                    pmti, d->tblid,
                    d->type == r->type ? pmt_type_str(d->type) : "BAD",
                    ((!rw && rw_m) || !rw_m) ? 'r' : ' ',
                    (( rw && rw_m) || !rw_m) ? 'w' : ' ',
                    d->port, bdf_to_str(d->bdf), d->addrdw << 2,
                    r->vfbase,
                    r->plimit,
                    bdf_to_str(bdf_make(r->blimit, r->dlimit, r->flimit)),
                    r->vfstridesel,
                    r->romsksel,
                    (u_int64_t)r->addrdw << 2,
                    d->valid && r->valid ? 'v' : '-',
                    r->indirect ? 'i' : '-',
                    r->notify ? 'n' : '-',
                    r->td ? 't' : '-',
                    r->aspace ? 'h' : '-');
}

/*
 * 6                               3
 * 3                               1                              0
 * ................................................................
 * bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbVVVVLLLPPPPP......TTT...
 * ........................................VVVVVVLLLLLPPPP...TTT...
 * bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbppppppp.................
 *
 *
 * idx id typ rw p:bb:dd.f baraddr     vf  lif   prt   pid  qtype
 *   0  0 mem rw 0:00:00.0 0x90600000     18:15 15:12 17:12 05:03
 *     prt 15 res  0x13e000000 4k -n--
 *     prt 16 db64 idx 0:0 qid 0:0  --d -pp:spp:spp:spp:spp:-pp:-pp:---
 *
 *
 * idx  tid typ rw p-bb:dd.f baraddr prtsize pgsz
 *
 * idx  tid typ rw p-bb:dd.f baraddr    vfid prts      prtsize vfrange   vflim bdf     pgsz qty:m qid
 *  32    0 mem rw 0 0x00000000    0    1 +8        4K   15 +16      1 02:00.0 0      0:0  0-1
 *  33  0 mem rw 0 0x00000000    0    9 +1       16K   14 +15      1 02:00.0 0      3:7  0-1

 *
 */
static void
pmt_show_bar_entry_hdr(void)
{
    pciesys_loginfo("%-4s %-2s %-4s %-2s %-9s %-10s %-4s "
                    "%-5s %-5s %-5s %-5s %-5s "
                    "%-4s\n",
                    "idx", "id", "type", "rw", "p:bb:dd.f", "baraddr",
                    "size", " vf  ", " lif ", " prt ", " pid ", "qtype",
                    "prts");
}

static int
bitspan(const int bits, const int bite)
{
    return bite > bits ? bite - bits : 0;
}

static void
pmt_show_bar_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    const pmt_bar_format_t *d = &dm->data.bar;
    const pmt_bar_format_t *m = &dm->mask.bar;
    const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const int rw = d->rw;
    const int rw_m = m->rw;
    prt_t lprt, *prt = &lprt;
    int pidb, pidc, qtyb, qtyc;
    int vfb, vfc, lifb, lifc, resb, resc;
    char qtys[8] = { '\0' };
    char ress[8] = { '\0' };
    char pids[8] = { '\0' };
    char lifs[8] = { '\0' };
    char vfs [8] = { '\0' };
    char prts[8] = { '\0' };

    if (last_hdr_displayed != PMTT_BAR) {
        pmt_show_bar_entry_hdr();
        last_hdr_displayed = PMTT_BAR;
    }

    if (spmt->loaded) {
        prt_get(pmr->prtb, prt);
    } else {
        const pciehw_sprt_t *sprt = &pshmem->sprt[pmr->prtb];
        *prt = sprt->prt;
    }

    qtyb = qtyc = 0;
    resb = resc = 0;
    pidb = pidc = 0;
    lifb = lifc = 0;
    vfb  = vfc  = 0;

    switch (prt->cmn.type) {
    case PRT_TYPE_RES:
        resb = pmr->prtsize;
        resc = bitspan(resb, pmr->vfstart);
        vfb = pmr->vfstart;
        vfc = bitspan(vfb, pmr->vfend);
        if (pmr->prtc) {
            if (pmr->prtc == 1) {
                snprintf(prts, sizeof(prts), "%d", pmr->prtb);
            } else {
                snprintf(prts, sizeof(prts), "%d-%d",
                         pmr->prtb, pmr->prtb + pmr->prtc - 1);
            }
        }
        break;
    case PRT_TYPE_DB64:
        qtyb = pmr->qtypestart;
        qtyc = 3;

        resb = pmr->prtsize;
        resc = bitspan(resb, pmr->vfstart);

        pidb = ffs(pmt_bar_get_pagesize(pmt)) - 1;
        pidc = bitspan(pidb, pmr->prtsize);

        lifb = pmr->prtsize;
        lifc = bitspan(lifb, pmr->vfstart);

        vfb = pmr->vfstart;
        vfc = bitspan(vfb, pmr->vfend);

        snprintf(prts, sizeof(prts), "%d", pmr->prtb);
        break;
    case PRT_TYPE_DB32:
    case PRT_TYPE_DB16:
        /* XXX add these */
        break;
    }

#define S(x) \
    if (x##c) snprintf(x##s, sizeof(x##s), "%2d:%-2d", x##b + x##c - 1, x##b);
    S(qty); S(res); S(pid); S(lif); S(vf);

    pciesys_loginfo("%-4d %2d %-4s %c%c %1d:%-7s 0x%08" PRIx64 " %-4s "
                    "%5s %5s %5s %5s %5s "
                    "%-4s\n",
                    pmti, d->tblid,
                    d->type == pmr->type ? pmt_type_str(d->type) : "BAD",
                    ((!rw && rw_m) || !rw_m) ? 'r' : '-',
                    (( rw && rw_m) || !rw_m) ? 'w' : '-',
                    d->port, bdf_to_str(pmr->bdf),
                    (u_int64_t)d->addrdw << 2,
                    human_readable(pmt_bar_getsize(pmt)),
                    vfs, lifs, ress, pids, qtys, prts);
}

static void
pmt_show_raw_entry_hdr(void)
{
    pciesys_loginfo("%-4s %-16s %-16s %-3s\n",
                    "idx", "x", "y", "pmr");
}

static void
pmt_show_raw_entry(const int pmti,
                   const pmt_t *pmt,
                   const pmt_datamask_t *dm)
{
    const pmt_tcam_t *tcam = &pmt->pmte.tcam;
    const u_int32_t *w = pmt->pmre.w;

    if (last_hdr_displayed != PMTT_RAW) {
        pmt_show_raw_entry_hdr();
        last_hdr_displayed = PMTT_RAW;
    }

    pciesys_loginfo("%-4d %016" PRIx64 " %016" PRIx64 " %08x %08x %08x %08x\n",
                    pmti, tcam->x, tcam->y,
                    w[0], w[1], w[2], w[3]);
}

static void
pmt_show_entry(const int pmti, const pmt_t *pmt, const int flags)
{
    pmt_datamask_t dm;
    const pmt_cmn_format_t *cmn;

    pmt_entry_dec(&pmt->pmte, &dm);
    cmn = &dm.data.cmn;

    switch (cmn->type) {
    case PMT_TYPE_CFG:
        if (flags & PMTT_CFG) {
            if (flags & PMTT_RAW) {
                pmt_show_raw_entry(pmti, pmt, &dm);
            } else {
                pmt_show_cfg_entry(pmti, pmt, &dm);
            }
        }
        break;
    case PMT_TYPE_MEM:
    case PMT_TYPE_IO:
        if (flags & PMTT_BAR) {
            if (flags & PMTT_RAW) {
                pmt_show_raw_entry(pmti, pmt, &dm);
            } else {
                pmt_show_bar_entry(pmti, pmt, &dm);
            }
        }
        break;
    case PMT_TYPE_RC:
        break;
    default:
        break;
    }
}

static void
pmt_show(const int flags)
{
    pmt_t pmt;
    int i;

    last_hdr_displayed = -1;
    for (i = 0; i < pmt_count(); i++) {
        pmt_get(i, &pmt);
        if (pmt_is_valid(&pmt)) {
            pmt_show_entry(i, &pmt, flags);
        }
    }
}

void
pciehw_pmt_show(int argc, char *argv[])
{
    int opt, flags;

    flags = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "bcr")) != -1) {
        switch (opt) {
        case 'b':
            flags |= PMTT_BAR;
            break;
        case 'c':
            flags |= PMTT_CFG;
            break;
        case 'r':
            flags |= PMTT_RAW;
            break;
        default:
            return;
        }
    }
    if ((flags & (PMTT_BAR | PMTT_CFG)) == 0) {
        flags |= PMTT_BAR | PMTT_CFG;
    }

    pmt_show(flags);
}

void
pciehw_pmt_dbg(int argc, char *argv[])
{
    pciehw_pmt_show(argc, argv);
}
