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

static int npmt = PMT_COUNT_FPGA; /* XXX runtime */

static int
pmt_size(void)
{
    return npmt;
}

static char *
pmt_type_str(int type)
{
    static char *typestr[] = {
        "cfg", "mem", "rc", "UNK3", "UNK4", "io", "UNK5", "UNK6", "UNK7"
    };
    return typestr[type & 0x7];
}

static int
pmt_alloc(const pciehwdevh_t phwdevh)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_spmt_t *spmt;
    int i;

    for (spmt = phwmem->spmt, i = 0; i < pmt_size(); i++, spmt++) {
        if (spmt->owner == 0) {
            spmt->owner = phwdevh;
            return i;
        }
    }
    /* out of pmt entries? */
    pciehsys_error("pmt_alloc failed for 0x%08x\n", phwdevh);
    return -1;
}

static void
pmt_free(const int pmti)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];

    assert(pmti >= 0 && pmti < pmt_size());
    assert(spmt->owner != 0);

    spmt->owner = 0;
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
pmt_get_tcam(const int pmti, pmt_tcam_entry_t *e)
{
    pal_reg_rd32w(pmt_addr(pmti), e->words, PMT_NWORDS);
}

static void
pmt_set_tcam(const int pmti, const pmt_tcam_entry_t *e)
{
    pal_reg_wr32w(pmt_addr(pmti), e->words, PMT_NWORDS);
}

static void
pmt_get_pmr(const int pmti, pmr_t pmr)
{
    pal_reg_rd32w(pmr_addr(pmti), pmr, PMR_NWORDS);
}

static void
pmt_set_pmr(const int pmti, const pmr_t pmr)
{
    pal_reg_wr32w(pmr_addr(pmti), pmr, PMR_NWORDS);
}

/*
 * Retrieve an entry from hardware.
 */
static void
pmt_get(const int pmti, pmt_t *p)
{
    pmt_tcam_entry_t e;

    pmt_get_tcam(pmti, &e);
    p->valid = e.v;
    p->data = e.x;
    p->mask = e.x ^ e.y;

    pmt_get_pmr(pmti, p->pmr);
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
pmt_set(const int pmti, const pmt_t *p)
{
    pmt_tcam_entry_t e;

    e.x =  p->data & p->mask;
    e.y = ~p->data & p->mask;
    e.v = p->valid;

    /*
     * Set PMR entry first, then TCAM, so by the time a tcam search
     * can hit an entry the corresponding ram entry is valid too.
     */
    pmt_set_pmr(pmti, p->pmr);
    pmt_set_tcam(pmti, &e);
}

static void
pmt_clr_tcam(const int pmti)
{
    const pmt_tcam_entry_t e = { 0 };

    pmt_set_tcam(pmti, &e);
}

/*
 * Clear a pmt slot.  For now we just invalidate
 * the tcam entry so searches don't hit, but we
 * don't write anything to PMR.
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
            const u_int8_t notify,
            const u_int8_t indirect)
{
    const u_int64_t cfgpadw = cfgpa >> 2;
    const u_int16_t bdf = phwdev->bdf;
    pmt_t pmt = { 0 };
    pmt_cfg_t *d = (pmt_cfg_t *)&pmt.data;
    pmt_cfg_t *m = (pmt_cfg_t *)&pmt.mask;
    pmr_cfg_t *r = (pmr_cfg_t *)&pmt.pmr;

    d->valid     = 1;
    d->tblid     = 0;
    d->type      = PMT_TYPE_CFG;
    d->port      = phwdev->port;
    d->rw        = 0;
    d->bdf       = bdf;
    d->addrdw    = addr >> 2;
    d->rsrv      = 0x0;

    m->valid     = 0x1;
    m->tblid     = 0x0; /* don't care, for now */
    m->type      = 0x7;
    m->port      = 0x7;
    m->rw        = 0;
    m->bdf       = 0xffff;
    m->addrdw    = addrm >> 2;
    m->rsrv      = 0x0; /* don't care */

    r->valid     = 1;
    r->type      = PMT_TYPE_CFG;
    r->vfbase    = 0;
    r->indirect  = indirect;
    r->notify    = notify;
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
    r->addrdw    = cfgpadw;
    r->aspace    = 0;    /* cfgpadw is local not host */
    r->romsksel  = romsksel;

    pmt.valid = 1;
    pmt_set(pmti, &pmt);
}

static void
pmt_set_bar(const int pmti,
            const u_int8_t port,
            const u_int16_t bdf,
            const pciehw_spmt_t *spmt)
{
    pmt_t pmt = { 0 };
    pmt_bar_t *d = (pmt_bar_t *)&pmt.data;
    pmt_bar_t *m = (pmt_bar_t *)&pmt.mask;
    pmr_bar_t *r = (pmr_bar_t *)&pmt.pmr;
    const u_int64_t addr = spmt->baraddr;
    const u_int64_t addrm = ~((1 << (ffsl(spmt->barsize) - 1)) - 1);

    assert((spmt->prtcount & (spmt->prtcount - 1)) == 0);
    assert((spmt->prtsize & (spmt->prtsize - 1)) == 0);

    d->valid     = 1;
    m->valid     = 0x1;

    d->tblid     = 0;
    m->tblid     = 0x0; /* don't care, for now */

    d->type      = spmt->type;
    m->type      = 0x7;

    d->port      = port;
    m->port      = 0x7;

    d->rw        = 0;
    m->rw        = 0;

    d->addrdw    = addr >> 2;
    m->addrdw    = addrm >> 2;

    r->valid     = 1;
    r->type      = spmt->type;
    r->vfbase    = 0;
    r->indirect  = spmt->indirect;
    r->notify    = spmt->notify;
    r->prtbase   = spmt->prtbase;
    r->prtcount  = spmt->prtcount;
    r->prtsize   = ffs(spmt->prtsize) - 1;
    r->vfstart   = r->prtsize + ffs(spmt->prtcount) - 1;
    r->vfend     = r->vfstart + 1;
    r->vflimit   = 1;
    r->bdf       = bdf;
    r->td        = 0;
    r->pagesize  = 0;   /* 4k page size for now */
    r->qtypestart= spmt->qtypestart;
    r->qtypemask = spmt->qtypemask;
    r->qidstart  = 0;
    r->qidend    = 1;

    pmt.valid = 1;
    pmt_set(pmti, &pmt);
}

static void
pmt_grst(void)
{
#ifdef __aarch64__
    pal_reg_wr32(PMT_GRST, 1);
#else
    int i;

    for (i = 0; i < pmt_size(); i++) {
        pmt_clr_tcam(i);
    }
#endif
}

static void
pmt_pmr_init(void)
{
    const pmr_t pmr = { 0 };
    int i;

    for (i = 0; i < pmt_size(); i++) {
        pmt_set_pmr(i, pmr);
    }
}

static void
pmt_reset(pciehw_t *phw)
{
    pmt_grst();
    pmt_pmr_init();
}

static int
bar_type_to_pmt_type(pciehbartype_t bartype)
{
    int pmttype = 0;

    switch (bartype) {
    case PCIEHBARTYPE_MEM:
    case PCIEHBARTYPE_MEM64: pmttype = PMT_TYPE_MEM; break;
    case PCIEHBARTYPE_IO:    pmttype = PMT_TYPE_IO;  break;
    case PCIEHBARTYPE_NONE:
    default:
        pciehsys_error("unexpected bartype: %d\n", bartype);
        assert(0);
        break;
    }
    return pmttype;
}

static void
pmt_load_bar(pciehwbar_t *phwbar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = phwbar->pmti;
    const pciehw_spmt_t *spmt = &phwmem->spmt[pmti];
    const pciehwdev_t *phwdev = pciehwdev_get(spmt->owner);

    assert(phwbar->valid);

    pmt_set_bar(pmti,
                phwdev->port,
                phwdev->bdf,
                spmt);
}

/******************************************************************
 * apis
 */

int
pciehw_pmt_load_cfg(pciehwdev_t *phwdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    const u_int64_t cfgpa = pal_mem_vtop(&phwmem->cfgcur[hwdevh]);
    int pmti, i;

    for (i = 0; i < PCIEHW_ROMSKSZ; i++) {
        const int romsk = phwdev->romsksel[i];
        const int notify = phwdev->cfghnd[i] != 0;
        if (romsk != ROMSK_RDONLY) {
            pmti = pmt_alloc(hwdevh);
            if (pmti < 0) {
                pciehsys_error("load_cfg: pmt_alloc failed i %d\n", i);
                goto error_out;
            }
            pmt_set_cfg(phwdev, pmti, cfgpa, i << 2, 0xffff, romsk, notify, 0);
        }
    }
    /*
     * Catchall entry.  We add a read-write entry for all addresses,
     * but romsk=1 selects the read-only entry so effectively this
     * claims all read/write transactions but writes have no effect.
     */
    pmti = pmt_alloc(hwdevh);
    if (pmti < 0) {
        pciehsys_error("load_cfg: pmt_alloc failed catchall\n");
        goto error_out;
    }
    pmt_set_cfg(phwdev, pmti, cfgpa, 0, 0, ROMSK_RDONLY, 0, 0);
    return 0;

 error_out:
    pciehw_pmt_unload_cfg(phwdev);
    return -1;
}

void
pciehw_pmt_unload_cfg(pciehwdev_t *phwdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehw_spmt_t *spmt;
    int i;

    for (spmt = phwmem->spmt, i = 0; i < pmt_size(); i++, spmt++) {
        if (spmt->owner == hwdevh && spmt->type == PMT_TYPE_CFG) {
            pmt_clr(i);
            pmt_free(i);
        }
    }
}

int
pciehw_pmt_alloc_bar(pciehwdev_t *phwdev, const pciehbar_t *bar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehw_spmt_t *spmt;
    int pmti, prti;

    pmti = pmt_alloc(hwdevh);
    if (pmti < 0) {
        pciehsys_error("pmt_alloc: pmt_alloc failed\n");
        return -1;
    }
    prti = pciehw_prt_alloc(phwdev, bar);
    if (prti < 0) {
        pmt_free(pmti);
        return -1;
    }

    spmt = &phwmem->spmt[pmti];
    spmt->barsize = bar->size;
    spmt->type = bar_type_to_pmt_type(bar->type);
    spmt->prtbase = prti;
    spmt->prtcount = bar->nregs;
    spmt->prtsize = bar->size / bar->nregs;
    /* XXX get these from bar->reg[] */
    spmt->qtypestart = 3;
    spmt->qtypemask = 0x7;
    return pmti;
}

void
pciehw_pmt_free_bar(const int pmti)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];

    assert(spmt->loaded == 0);
    pciehw_prt_free(spmt->prtbase, spmt->prtcount);
    pmt_free(pmti);
}

void
pciehw_pmt_load_bar(pciehwbar_t *phwbar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = phwbar->pmti;
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];

    /*
     * Load PRT first, then load PMT so PMT tcam search hit
     * will find valid PRT entries.
     */
    pciehw_prt_load(spmt->prtbase, spmt->prtcount);
    pmt_load_bar(phwbar);
    if (!spmt->loaded) {
        spmt->loaded = 1;
    }
}

void
pciehw_pmt_setaddr(pciehwbar_t *phwbar, const u_int64_t addr)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = phwbar->pmti;
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];

    spmt->baraddr = addr;
    /* if loaded, reload with new addr */
    if (spmt->loaded) {
        pmt_load_bar(phwbar);
    }
}

void
pciehw_pmt_unload_bar(pciehwbar_t *phwbar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = phwbar->pmti;
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];

    /*
     * Unload PMT first, then PRT so PMT tcam search will not hit
     * and PRT is unreferenced.
     */
    if (spmt->loaded) {
        pmt_clr(pmti);
        pciehw_prt_unload(spmt->prtbase, spmt->prtcount);
        spmt->loaded = 0;
    }
}

void
pciehw_pmt_enable_bar(pciehwbar_t *phwbar, const int on)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = phwbar->pmti;
    pciehw_spmt_t *spmt = &phwmem->spmt[pmti];

    if (on && !spmt->loaded) {
        pciehw_pmt_load_bar(phwbar);
    } else if (!on && spmt->loaded) {
        pciehw_pmt_unload_bar(phwbar);
    }
}

void
pciehw_pmt_init(pciehw_t *phw)
{
    pmt_reset(phw);
    pciehw_prt_init(phw);
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
                 "%-4s %-9s %-4s %-5s %-11s %-5s\n",
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
    const int rw = d->rw;
    const int rw_m = m->rw;

    pciehsys_log("%4d %2d %-3s %c%c %1d:%-7s 0x%04x "
                 "%4d %d:%-7s %4d %5d 0x%09"PRIx64" %c%c%c%c%c\n",
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
                 (u_int64_t)r->addrdw << 2,
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
                 r->vflimit,
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
    case PMT_TYPE_MEM:
    case PMT_TYPE_IO:
        if (last_hdr_displayed != PMT_TYPE_MEM &&
            last_hdr_displayed != PMT_TYPE_IO) {
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
    for (i = 0; i < pmt_size(); i++) {
        pmt_get(i, &pmt);
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
    for (i = 0; i < pmt_size(); i++) {
        pmt_get_tcam(i, &e);
        pmt_get_pmr(i, pmr);
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

void
pciehw_pmt_set_notify(pciehwdev_t *phwdev, const int on)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehw_spmt_t *spmt;
    pmt_t pmt;
    int i;

    for (spmt = phwmem->spmt, i = 0; i < pmt_size(); i++, spmt++) {
        if (spmt->owner == hwdevh) {
            pmt_common_t *pmt_common;

            pmt_get(i, &pmt);
            pmt_common = (pmt_common_t *)&pmt.data;
            switch (pmt_common->type) {
            case PMT_TYPE_CFG: {
                pmr_cfg_t *r = (pmr_cfg_t *)&pmt.pmr;
                r->notify = on;
                break;
            }
            case PMT_TYPE_MEM:
            case PMT_TYPE_IO: {
                pmr_bar_t *r = (pmr_bar_t *)&pmt.pmr;
                r->notify = on;
                break;
            }
            default:
                break;
            }
            pmt_set(i, &pmt);
        }
    }
}
