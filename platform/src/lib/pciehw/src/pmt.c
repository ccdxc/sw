/*
 * Copyright (c) 2017, Pensando Systems Inc.
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

#include "misc.h"
#include "bdf.h"
#include "pal.h"
#include "pciehsys.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "pmt.h"

static int
pmt_size(void)
{
    pciehw_t *phw = pciehw_get();
    return phw->npmt;
}

static char *
pmt_type_str(int type)
{
    static char *typestr[] = {
        "cfg", "mem", "rc", "UNK"
    };
    return typestr[type & 0x3];
}

static int
pmt_alloc(pciehw_t *phw, const pciehwdevh_t phwdevh)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    int i;

    for (i = 0; i < phw->npmt; i++) {
        if (phwmem->pmt_inuse[i] == 0) {
            phwmem->pmt_inuse[i] = phwdevh;
            return i;
        }
    }
    /* out of pmt entries? */
    pciehsys_error("pmt_alloc failed for 0x%08x\n", phwdevh);
    return -1;
}

static u_int64_t
pmt_addr(const int pmti)
{
    assert(pmti >= 0 && pmti < pmt_size());
    return PMT_BASE + (pmti * PMT_STRIDE);
}

static u_int64_t
pmr_addr(const int pmti)
{
    assert(pmti >= 0 && pmti < pmt_size());
    return PMR_BASE + (pmti * PMR_STRIDE);
}

static void
pmt_get_tcam(pciehw_t *phw, const int pmti, pmt_tcam_entry_t *e)
{
    pal_reg_rd32w(pmt_addr(pmti), e->words, PMT_NWORDS);
}

static void
pmt_set_tcam(pciehw_t *phw, const int pmti, const pmt_tcam_entry_t *e)
{
    pal_reg_wr32w(pmt_addr(pmti), e->words, PMT_NWORDS);
}

static void
pmt_get_pmr(pciehw_t *phw,
            const int pmti,
            pmr_t pmr)
{
    pal_reg_rd32w(pmr_addr(pmti), pmr, PMR_NWORDS);
}

static void
pmt_set_pmr(pciehw_t *phw,
            const int pmti,
            const pmr_t pmr)
{
    pal_reg_wr32w(pmr_addr(pmti), pmr, PMR_NWORDS);
}

/*
 * Retrieve an entry from hardware.
 */
static void
pmt_get(pciehw_t *phw, const int pmti, pmt_t *p)
{
    pmt_tcam_entry_t e;

    pmt_get_tcam(phw, pmti, &e);
    p->valid = e.v;
    p->data = e.x;
    p->mask = e.x ^ e.y;

    pmt_get_pmr(phw, pmti, p->pmr);
}

/*
 * Install tcam entry.
 *     "p->data" is the entry data values
 *     "p->mask" has 1's for bits we want to match in "data",
 *               0's for bits in "data" we want to ignore.
 *
 * {X Y} result
 * -------
 * {0 0} (always match)
 * {0 1} 0
 * {1 0} 1
 * {1 1} (never match)
 */
static void
pmt_set(pciehw_t *phw, const int pmti, const pmt_t *p)
{
    pmt_tcam_entry_t e;

    e.x =  p->data & p->mask;
    e.y = ~p->data & p->mask;
    e.v = p->valid;

    /*
     * Set PMR entry first, then TCAM, so by the time a tcam search
     * can hit an entry the corresponding ram entry is valid too.
     */
    pmt_set_pmr(phw, pmti, p->pmr);
    pmt_set_tcam(phw, pmti, &e);
}

static void
pmt_clr_tcam(pciehw_t *phw, const int pmti)
{
    pmt_tcam_entry_t e = { 0 };

    pmt_set_tcam(phw, pmti, &e);
}

/*
 * Clear a pmt slot.  For now we just invalidate
 * the tcam entry so searches don't hit, but we
 * don't write anything to PMR.
 */
static void
pmt_clr(pciehw_t *phw, const int pmti)
{
    pmt_clr_tcam(phw, pmti);
}

static void
pmt_set_cfg(pciehw_t *phw,
            pciehwdev_t *phwdev,
            const int pmti,
            const u_int64_t cfgpa,
            const u_int16_t addr,
            const u_int16_t addrm,
            const u_int8_t romsksel,
            const int oflags)
{
    const u_int64_t cfgpadw = cfgpa >> 2;
    const u_int16_t bdf = phwdev->bdf;
    int rw, rw_m;
    pmt_t pmt = { 0 };
    pmt_cfg_t *d = (pmt_cfg_t *)&pmt.data;
    pmt_cfg_t *m = (pmt_cfg_t *)&pmt.mask;
    pmr_cfg_t *r = (pmr_cfg_t *)&pmt.pmr;

    switch (oflags) {
    case O_RDONLY: rw = 0; rw_m = 1; break;
    case O_WRONLY: rw = 1; rw_m = 1; break;
    case O_RDWR:   rw = 0; rw_m = 0; break;
    }

    d->valid     = 1;
    d->tblid     = 0;
    d->type      = PMT_TYPE_CFG;
    d->port      = phwdev->port;
    d->rw        = rw;
    d->bdf       = bdf;
    d->addrdw    = addr >> 2;
    d->rsrv      = 0x0;

    m->valid     = 0x1;
    m->tblid     = 0x0; /* don't care, for now */
    m->type      = 0x7;
    m->port      = 0x7;
    m->rw        = rw_m;
    m->bdf       = 0xffff;
    m->addrdw    = addrm >> 2;
    m->rsrv      = 0x0; /* don't care */

    r->valid     = 1;
    r->type      = PMT_TYPE_CFG;
    r->vfbase    = 0;
    r->indirect  = 0;
    r->notify    = phw->hwparams.force_notify_cfg;
    r->pstart    = phwdev->port;
    r->bstart    = bdf_to_bus(bdf);
    r->dstart    = bdf_to_dev(bdf);
    r->fstart    = bdf_to_fnc(bdf);
    r->plimit    = phwdev->port;
    r->blimit    = bdf_to_bus(bdf);
    r->dlimit    = bdf_to_dev(bdf);
    r->flimit    = bdf_to_fnc(bdf);
    r->stridesel = phwdev->vfstridesel;
    r->td        = 0;
    r->addrdw_lo = cfgpadw;
    r->addrdw_hi = cfgpadw >> 4;
    r->aspace    = 0;    /* cfgpadw is local */
    r->romsksel  = romsksel;

    pmt.valid = 1;
    pmt_set(phw, pmti, &pmt);
}

static void
pmt_set_bar(pciehw_t *phw,
            const int pmti,
            const u_int8_t port,
            const u_int16_t bdf,
            const u_int64_t addr,
            const u_int64_t addrm,
            const u_int32_t prtbase,
            const u_int32_t prtcount,
            const u_int32_t prtsize)
{
    pmt_t pmt = { 0 };
    pmt_bar_t *d = (pmt_bar_t *)&pmt.data;
    pmt_bar_t *m = (pmt_bar_t *)&pmt.mask;
    pmr_bar_t *r = (pmr_bar_t *)&pmt.pmr;

    d->valid     = 1;
    d->tblid     = 0;
    d->type      = PMT_TYPE_BAR;
    d->port      = port;
    d->rw        = 0;
    d->addrdw    = addr >> 2;

    m->valid     = 0x1;
    m->tblid     = 0x0; /* don't care, for now */
    m->type      = 0x7;
    m->port      = 0x7;
    m->rw        = 0;
    m->addrdw    = addrm >> 2;

    r->valid     = 1;
    r->type      = PMT_TYPE_BAR;
    r->vfbase    = 0;
    r->indirect  = 0;
    r->notify    = phw->hwparams.force_notify_bar;
    r->prtbase   = prtbase;
    r->prtcount  = prtcount;
    r->prtsize   = prtsize;
    r->vfstart   = 0;
    r->vfend     = 1;
    r->vflimit_lo= 1;
    r->vflimit_hi= 0 >> 6;
    r->bdf       = bdf;
    r->td        = 0;
    r->pagesize  = 0;   /* 4k page size for now */
    r->qtypestart= 0;
    r->qtypemask = 0;
    r->qidstart  = 0;
    r->qidend    = 1;

    pmt.valid = 1;
    pmt_set(phw, pmti, &pmt);
}

static void
pmt_grst(pciehw_t *phw)
{
#ifdef __aarch64__
    pal_reg_wr32(PMT_GRST, 1);
#else
    int i;

    for (i = 0; i < phw->npmt; i++) {
        pmt_clr_tcam(phw, i);
    }
#endif
}

static void
pmt_pmr_init(pciehw_t *phw)
{
    const pmr_t pmr = { 0 };
    int i;

    for (i = 0; i < phw->npmt; i++) {
        pmt_set_pmr(phw, i, pmr);
    }
}

static void
pmt_reset(pciehw_t *phw)
{
    pmt_grst(phw);
    pmt_pmr_init(phw);
}

/******************************************************************
 * apis
 */

int
pciehw_pmt_load_cfg(pciehw_t *phw, pciehwdev_t *phwdev)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    const u_int64_t cfgpa = pal_mem_vtop(&phwmem->cfgcur[hwdevh]);
    int pmti, i;

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        const int romsk = phwdev->romsksel[i];
        if (romsk != ROMSK_RDONLY) {
            pmti = pmt_alloc(phw, hwdevh);
            if (pmti < 0) {
                pciehsys_error("load_cfg: pmt_alloc failed i %d\n", i);
                goto error_out;
            }
            pmt_set_cfg(phw, phwdev,
                        pmti, cfgpa, i << 2, 0xffff, romsk, O_RDWR);
        }
    }
    /*
     * Catchall entry.  We add a read-write entry for all addresses,
     * but romsk=1 selects the read-only entry so effectively this
     * claims all read/write transactions but writes have no effect.
     */
    pmti = pmt_alloc(phw, hwdevh);
    if (pmti < 0) {
        pciehsys_error("load_cfg: pmt_alloc failed catchall\n");
        goto error_out;
    }
    pmt_set_cfg(phw, phwdev, pmti, cfgpa, 0, 0, ROMSK_RDONLY, O_RDWR);
    return 0;

 error_out:
    pciehw_pmt_unload_cfg(phw, phwdev);
    return -1;
}

void
pciehw_pmt_unload_cfg(pciehw_t *phw, pciehwdev_t *phwdev)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    int i;

    for (i = 0; i < pmt_size(); i++) {
        if (phwmem->pmt_inuse[i] == hwdevh) {
            pmt_clr(phw, i);
            phwmem->pmt_inuse[i] = 0;
        }
    }
}

int
pciehw_pmt_alloc(pciehwdev_t *phwdev, pciehbar_t *bar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehwbar_t *phwbar;
    pciehw_spmt_t *spmt;
    int pmti;

    pmti = pmt_alloc(phw, hwdevh);
    if (pmti < 0) {
        pciehsys_error("pmt_alloc: pmt_alloc failed\n");
        return -1;
    }
    phwbar = &phwdev->bar[bar->cfgidx];
    phwbar->pmti = pmti;
    phwbar->type = bar->type;
    phwbar->valid = 1;
    spmt = &phwmem->spmt[pmti];
    spmt->barsize = bar->size;
    return 0;
}

void
pciehw_pmt_load_bar(pciehw_t *phw,
                    pciehwdev_t *phwdev,
                    pciehwbar_t *phwbar)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = phwbar->pmti;
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];
    const u_int64_t addr = spmt->baraddr;
    const u_int64_t addrm = ~((1 << (ffsl(spmt->barsize) - 1)) - 1);

    assert(phwbar->valid);

    pmt_set_bar(phw,
                pmti,
                phwdev->port,
                phwdev->bdf,
                addr,
                addrm,
                phwbar->prtbase,
                phwbar->prtcount,
                phwbar->prtsize);

    if (!spmt->loaded) {
        spmt->loaded = 1;
    }
}

void
pciehw_pmt_init(pciehw_t *phw)
{
    phw->npmt = phw->is_asic ? PCIEHW_NPMT : PCIEHW_NPMT / 4;
    pmt_reset(phw);
}

/******************************************************************
 * debug
 */

/*
idx  id typ rw p:00:00.0 0xreg_
                                vfid port 00:00.0 vfst romsk 0xaddress VINTA
 */
static void
pmt_show_cfg_entry_hdr(void)
{
    pciehsys_log("%-4s %-2s %-3s %-2s %-9s %-6s "
                 "%-4s %-9s %-4s %-5s %-10s %-5s\n",
                 "idx", "id", "typ", "rw", "p:bdf", "cfgreg",
                 "vfid", "p:bdflim", "vfst", "romsk",
                 "address", "flags");
}

static void
pmt_show_cfg_entry(pciehw_t *phw, const int pmti, pmt_t *pmt)
{
    const pmt_cfg_t *d = (pmt_cfg_t *)&pmt->data;
    const pmt_cfg_t *m = (pmt_cfg_t *)&pmt->mask;
    const pmr_cfg_t *r = (pmr_cfg_t *)&pmt->pmr;
    const u_int64_t addrdw = (r->addrdw_hi << 4 | r->addrdw_lo);
    const int rw = d->rw;
    const int rw_m = m->rw;

    pciehsys_log("%4d %2d %-3s %c%c %1d:%-7s 0x%04x "
                 "%4d %d:%-7s %4d %5d 0x%08"PRIx64" %c%c%c%c%c\n",
                 pmti, d->tblid,
                 d->type == r->type ? pmt_type_str(d->type) : "BAD",
                 ((!rw && rw_m) || !rw_m) ? 'r' : ' ',
                 (( rw && rw_m) || !rw_m) ? 'w' : ' ',
                 d->port, bdf_to_str(d->bdf), d->addrdw << 2,
                 r->vfbase,
                 r->plimit,
                 bdf_to_str(bdf_make(r->blimit, r->dlimit, r->flimit)),
                 r->stridesel,
                 r->romsksel,
                 addrdw << 2,
                 d->valid && r->valid ? 'v' : '-',
                 r->indirect ? 'i' : '-',
                 r->notify ? 'n' : '-',
                 r->td ? 't' : '-',
                 r->aspace ? 'h' : '-');
}

static void
pmt_show_bar_entry_hdr(void)
{
    pciehsys_log("%-4s %-2s %-3s %-2s %-1s %-10s "
                 "%-4s %-9s %-7s %-9s %-5s "
                 "%-7s %-4s %-5s %-5s\n",
                 "idx", "id", "typ", "rw", "p", "baraddr",
                 "vfid", "prts", "prtsize", "vfrange", "vflim",
                 "bdf", "pgsz", "qty:m", "qid");
}

static void
pmt_show_bar_entry(pciehw_t *phw, const int pmti, pmt_t *pmt)
{
    const pmt_bar_t *d = (pmt_bar_t *)&pmt->data;
    const pmt_bar_t *m = (pmt_bar_t *)&pmt->mask;
    const pmr_bar_t *r = (pmr_bar_t *)&pmt->pmr;
    const int rw = d->rw;
    const int rw_m = m->rw;

    pciehsys_log("%4d %2d %-3s %c%c %1d 0x%08"PRIx64" "
                 "%4d %4d +%-3d %7s %4d +%-3d %5d "
                 "%-7s %-4d %3d:%1d %2d-%-2d\n",
                 pmti, d->tblid,
                 d->type == r->type ? pmt_type_str(d->type) : "BAD",
                 ((!rw && rw_m) || !rw_m) ? 'r' : ' ',
                 (( rw && rw_m) || !rw_m) ? 'w' : ' ',
                 d->port,
                 (u_int64_t)d->addrdw << 2,
                 r->vfbase,
                 r->prtbase,
                 r->prtcount,
                 human_readable(1 << r->prtsize),
                 r->vfstart,
                 r->vfend,
                 (r->vflimit_hi << 6) | r->vflimit_lo,
                 bdf_to_str(r->bdf),
                 r->pagesize,
                 r->qtypestart,
                 r->qtypemask,
                 r->qidstart,
                 r->qidend);
}

static int last_hdr_displayed = -1;

static void
pmt_show_entry(pciehw_t *phw, const int pmti, pmt_t *pmt)
{
    pmt_common_t *pmt_common = (pmt_common_t *)&pmt->data;

    switch (pmt_common->type) {
    case PMT_TYPE_CFG:
        if (last_hdr_displayed != PMT_TYPE_CFG) {
            pmt_show_cfg_entry_hdr();
        }
        pmt_show_cfg_entry(phw, pmti, pmt);
        break;
    case PMT_TYPE_BAR:
        if (last_hdr_displayed != PMT_TYPE_BAR) {
            pmt_show_bar_entry_hdr();
        }
        pmt_show_bar_entry(phw, pmti, pmt);
        break;
    case PMT_TYPE_RC:
        break;
    default:
        break;
    }
    last_hdr_displayed = pmt_common->type;
}

static void
pmt_show_raw_entry_hdr(void)
{
    pciehsys_log("%-4s %-16s %-16s %-3s\n",
                 "idx", "x", "y", "pmr");
}

static void
pmt_show_raw_entry(pciehw_t *phw, const int pmti,
                   const pmt_tcam_entry_t *e,
                   const pmr_t pmr)
{
    const u_int32_t *w = pmr;

    pciehsys_log("%4d %016"PRIx64" %016"PRIx64" %08x %08x %08x %08x\n",
                 pmti, e->x, e->y,
                 w[0], w[1], w[2], w[3]);
}

static void
pmt_show(pciehw_t *phw)
{
    pmt_t pmt;
    int i;

    last_hdr_displayed = -1;
    for (i = 0; i < phw->npmt; i++) {
        pmt_get(phw, i, &pmt);
        if (pmt.valid) {
            pmt_show_entry(phw, i, &pmt);
        }
    }
}

static void
pmt_show_raw(pciehw_t *phw)
{
    pmt_tcam_entry_t e;
    pmr_t pmr;
    int i;

    pmt_show_raw_entry_hdr();
    for (i = 0; i < phw->npmt; i++) {
        pmt_get_tcam(phw, i, &e);
        pmt_get_pmr(phw, i, pmr);
        if (e.v) {
            pmt_show_raw_entry(phw, i, &e, pmr);
        }
    }
}

void
pciehw_pmt_dbg(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    int opt, raw;

    raw = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
        case 'r':
            raw = 1;
            break;
        default:
            return;
        }
    }

    if (raw) {
        pmt_show_raw(phw);
    } else {
        pmt_show(phw);
    }
}
