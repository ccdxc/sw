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

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgr/include/pciehw.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

#define PMT_BASE        PXB_(DHS_TGT_PMT)
#define PMT_STRIDE      \
    (ASIC_(PXB_CSR_DHS_TGT_PMT_ENTRY_ARRAY_ELEMENT_SIZE) * 4 * 8)
#define PMT_GRST        PXB_(CFG_TGT_PMT_GRST)
#define PMR_BASE        PXB_(DHS_TGT_PMR)
#define PMR_STRIDE      ASIC_(PXB_CSR_DHS_TGT_PMR_ENTRY_BYTE_SIZE)

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
pmt_set_cfg(const int pmti,
            const u_int8_t port,
            const u_int16_t bdf,
            const u_int16_t bdfm,
            const u_int16_t bdfst,
            const u_int16_t bdflim,
            const u_int64_t cfgpa,
            const u_int16_t addr,
            const u_int16_t addrm,
            const u_int8_t romsksel,
            const u_int8_t vfstridesel,
            const u_int32_t pmtflags)
{
    pmt_t pmt;

    pmt_cfg_enc(&pmt,
                port, bdf, bdfm, bdfst, bdflim,
                cfgpa, addr, addrm,
                romsksel, vfstridesel,
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
    u_int64_t cfgpa = pal_mem_vtop(&phwmem->cfgcur[hwdevh]);
    const u_int64_t zerospa = pal_mem_vtop(phwmem->zeros);
    const u_int8_t port = phwdev->port;
    u_int16_t bdf, bdfm, bdfst, bdflim;
    pciehwdevh_t cfgdevh;
    int pmti, i;

    /* bdf default for exact match */
    bdf = phwdev->bdf;
    bdfm = 0xffff;
    bdfst = bdf;
    bdflim = bdf;
    cfgdevh = hwdevh;
    /*
     * Install cfg space entries for PF or VF0.
     */
    if (!phwdev->vf || phwdev->vfidx == 0) {
        /*
         * If VF0 adjust bdflim,bdfm to match all totalvfs.
         */
        if (phwdev->vf) {
            const pciehwdev_t *parent = pciehwdev_get(phwdev->parenth);
            const u_int32_t totalvfs = parent->totalvfs;
            u_int16_t bdfmax;

            /*
             * Start at the "bdfst" of our parent, which for sriov pf
             * is always device 0, function 0 on our bus.  The sriov vfs
             * show up as additional functions on this bus.
             * Set "bdfm" to match bus number and wildcard on dev,fnc.
             * Then set "bdflim" limit to upper bound of dev, fnc based
             * on totalvfs.
             *
             * XXX this is too permissive!  For example, if we want 9 vfs
             * we set bdflim.dev=2 and bdflim.fnc=7, so cfg space is active
             * for 2*8=16 vfs.  For now, rely on host not to probe vfs
             * that it doesn't expect to find.  Fix this by adding 2 entries
             * for each, one that covers dev=1,fnc=0-7, and one that covers
             * dev=2,fnc=0.
             */
            bdfst = parent->bdf;
            bdfmax = bdfst + totalvfs;
            bdfm = 0xff00; /* XXX match all VFs on this bus */
            bdflim = bdf_make(bdf_to_bus(bdfmax),
                              totalvfs > 256 ? 0x1f : bdf_to_dev(bdfmax),
                              totalvfs > 7   ? 0xf  : bdf_to_fnc(bdfmax));
            /* adjust these to match bdfst */
            cfgdevh = phwdev->parenth;
            cfgpa = pal_mem_vtop(&phwmem->cfgcur[phwdev->parenth]);
        }

        for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
            const int romsk = phwdev->romsksel[i];
            const u_int32_t pmtf = phwdev->cfgpmtf[i];
            if (romsk != ROMSK_RDONLY || pmtf != PMTF_NONE) {
                pmti = pmt_alloc(1);
                if (pmti < 0) {
                    pciesys_logerror("load_cfg: pmt_alloc failed i %d\n", i);
                    goto error_out;
                }
                pmt_set_owner(pmti, cfgdevh);
                pmt_set_cfg(pmti, port, bdf, bdfm, bdfst, bdflim,
                            cfgpa, i << 2, 0xffff,
                            romsk, VFSTRIDE_IDX_DEVCFG, pmtf);
            }
        }
        /*
         * Catchall entry.  We add a read-write entry for all addresses,
         * but ROMSK_RDONLY selects the read-only entry so effectively this
         * claims all read/write transactions but writes have no effect.
         * We limit the range to PCIEHW_CFGSZ because that is all we have
         * here in cfgpa.
         */
        pmti = pmt_alloc(1);
        if (pmti < 0) {
            pciesys_logerror("load_cfg: pmt_alloc failed catchall\n");
            goto error_out;
        }
        pmt_set_owner(pmti, cfgdevh);
        pmt_set_cfg(pmti, port, bdf, bdfm, bdfst, bdflim,
                    cfgpa, 0, ~(PCIEHW_CFGSZ - 1),
                    ROMSK_RDONLY, VFSTRIDE_IDX_DEVCFG, PMTF_RW);
    }
    /*
     * For access to cfgspace above PCIEHW_CFGSZ add an entry to zerospa.
     * Note we can't use an entry that works for all VFs here because
     * the bdf wildcard match will always attempt to scale by bdf offset
     *
     * XXX Handle this region in sw with a single "force indirect" entry?
     */
    pmti = pmt_alloc(1);
    if (pmti < 0) {
        pciesys_logerror("load_cfg: pmt_alloc failed catchall zeros\n");
        goto error_out;
    }
    pmt_set_owner(pmti, hwdevh);
    pmt_set_cfg(pmti, port, bdf, 0xffff, bdf, bdf,
                zerospa, 0, 0,
                ROMSK_RDONLY, VFSTRIDE_IDX_4K, PMTF_RW);
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
    const u_int16_t bdf = pciehwdev_get_hostbdf(phwdev);

    if (!phwbar->valid) return;

#ifdef __aarch64__
#ifdef PCIEMGR_DEBUG
    pciesys_loginfo("%s: bar %d pmt %d loaded\n",
                    pciehwdev_get_name(phwdev), spmt->cfgidx, pmti);
#endif
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
#ifdef PCIEMGR_DEBUG
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);
    pciesys_loginfo("%s: bar %d pmt %d unloaded\n",
                    pciehwdev_get_name(phwdev), spmt->cfgidx, pmti);
#endif
#endif

    assert(phwbar->valid);
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
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const u_int32_t pmti = phwbar->pmtb;
    pciehw_spmt_t *spmt = &pshmem->spmt[pmti];

    if (on && !spmt->loaded) {
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
#ifdef PCIEMGR_DEBUG
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);
    pciesys_loginfo("%s: bar %d pmt %d setaddr 0x%" PRIx64 "\n",
                    pciehwdev_get_name(phwdev), spmt->cfgidx, pmti, addr);
#endif
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
    STATIC_ASSERT(PMT_COUNT == ASIC_(PXB_CSR_DHS_TGT_PMT_ENTRIES));

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
idx  id typ p:bb:dd.f bdfmsk p:bdflim  rw cfgreg vfst romsk address     flags
1    0  cfg 0:b4:00.0 0xffff 0:b4:00.0 -w 0x0004 0    0     0x0c4802000 vi---
 */
static void
pmt_show_cfg_entry_hdr(void)
{
    pciesys_loginfo("%-4s %-2s %-3s "
                    "%-9s %-6s %-9s "
                    "%-2s %-6s %-4s %-5s "
                    "%-11s %-5s\n",
                    "idx", "id", "typ",
                    "p:bb:dd.f", "bdfmsk", "p:bdflim",
                    "rw", "cfgreg", "vfst", "romsk",
                    "address", "flags");
}

static void
pmt_show_cfg_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pmt_cfg_format_t *d = &dm->data.cfg;
    /* const pmt_cfg_format_t *m = &dm->mask.cfg; */
    const pmr_cfg_entry_t *r = &pmt->pmre.cfg;
    /* bus adjust cfg bdf to match display of bar bdf */
    const u_int16_t bdf = pciehw_hostbdf(d->port,
                                         bdf_make(r->bstart,
                                                  r->dstart,
                                                  r->fstart));
    const u_int16_t bdflim = pciehw_hostbdf(d->port,
                                            bdf_make(r->blimit,
                                                     r->dlimit,
                                                     r->flimit));

    if (last_hdr_displayed != PMTT_CFG) {
        pmt_show_cfg_entry_hdr();
        last_hdr_displayed = PMTT_CFG;
    }

    pciesys_loginfo("%-4d %-2d %-3s "
                    "%1d:%-7s 0x%04x %1d:%-7s "
                    "%c%c 0x%04x %-4d %-5d "
                    "0x%09" PRIx64 " %c%c%c%c%c\n",
                    pmti, d->tblid,
                    d->type == r->type ? pmt_type_str(d->type) : "BAD",
                    d->port, bdf_to_str(bdf),
                    dm->mask.cfg.bdf,
                    r->plimit, bdf_to_str(bdflim),
                    pmt_allows_rd(pmt) ? 'r' : '-',
                    pmt_allows_wr(pmt) ? 'w' : '-',
                    d->addrdw << 2,
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
    pciesys_loginfo("%-4s %-2s %-3s %-2s %-9s %-10s %-4s %-4s\n",
                    "idx", "id", "typ", "rw", "p:bb:dd.f", "baraddr",
                    "size", "prts");
}

static int
bitspan(const int bits, const int bite)
{
    return bite > bits ? bite - bits : 0;
}

static int
qtypemask_to_count(const int qtypemask)
{
    static int maskmap[8] = { 0, 1, 2, 2, 3, 3, 3, 3 };
    return maskmap[qtypemask & 0x7];
}

static void
pmt_show_bar_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const pciehw_spmt_t *spmt = &pshmem->spmt[pmti];
    const pmt_bar_format_t *d = &dm->data.bar;
    /* const pmt_bar_format_t *m = &dm->mask.bar; */
    const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const u_int16_t bdf = pmr->bdf; /* already host adjusted */
    prt_t lprt, *prt = &lprt;
    int pidb, pidc, qtyb, qtyc, qidb, qidc;
    int vfb, vfc, lifb, lifc, resb, resc;
    char qtys[16] = { '\0' };
    char ress[16] = { '\0' };
    char pids[16] = { '\0' };
    char lifs[16] = { '\0' };
    char vfs [16] = { '\0' };
    char qids[16] = { '\0' };
    char prts[16] = { '\0' };

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
    qidb = qidc = 0;

    switch (prt_type(prt)) {
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
        qtyc = qtypemask_to_count(pmr->qtypemask);

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
        qtyb = pmr->qtypestart;
        qtyc = qtypemask_to_count(pmr->qtypemask);

        resb = pmr->prtsize;
        resc = bitspan(resb, pmr->vfstart);

        pidb = ffs(pmt_bar_get_pagesize(pmt)) - 1;
        pidc = bitspan(pidb, pmr->prtsize);

        lifb = pmr->prtsize;
        lifc = bitspan(lifb, pmr->vfstart);

        vfb = pmr->vfstart;
        vfc = bitspan(vfb, pmr->vfend);

        if (prt->db.qidsel) {
            /* if qidsel, then qid encoded in address */
            qidb = pmt->pmre.bar.qidstart;
            qidc = pmt->pmre.bar.qidend - qidb;
        }

        snprintf(prts, sizeof(prts), "%d", pmr->prtb);
        break;
    }

#define S(x) \
    if (x##c) \
        snprintf(x##s, sizeof(x##s), " %s=%d:%d", #x, x##b + x##c - 1, x##b)
    S(qty); S(res); S(pid); S(lif); S(vf); S(qid);

    pciesys_loginfo("%-4d %-2d %-3s %c%c %1d:%-7s 0x%08" PRIx64 " %-4s %-4s"
                    "%s%s%s%s%s%s\n",
                    pmti, d->tblid,
                    d->type == pmr->type ? pmt_type_str(d->type) : "BAD",
                    pmt_allows_rd(pmt) ? 'r' : '-',
                    pmt_allows_wr(pmt) ? 'w' : '-',
                    d->port, bdf_to_str(bdf),
                    (u_int64_t)d->addrdw << 2,
                    human_readable(pmt_bar_getsize(pmt)),
                    prts,
                    vfs, lifs, ress, pids, qids, qtys);
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
                    w[3], w[2], w[1], w[0]);
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
