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
    static char *typestr[8] = {
        "cfg", "mem", "rc", "UNK3", "UNK4", "io", "UNK6", "UNK7"
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
static void
pmt_get(const int pmti, pmt_t *p)
{
    pmt_get_entry(pmti, &p->pmte);
    pmr_get_entry(pmti, &p->pmre);
}

/*
 * Install an entry in hardware at the specified index.
 */
static void
pmt_set(const int pmti, const pmt_t *p)
{
    /*
     * Set PMR entry first, then TCAM, so by the time a tcam search
     * can hit an entry the corresponding ram entry is valid too.
     */
    pmr_set_entry(pmti, &p->pmre);
    pmt_set_entry(pmti, &p->pmte);
}

static void
pmt_clr_tcam(const int pmti)
{
    pmt_entry_t pmte;

    memset(&pmte, 0, sizeof(pmte));
    pmt_set_entry(pmti, &pmte);
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

static int
pmt_is_valid(const pmt_t *pmt)
{
    return pmt->pmte.tcam.v;
}

/*
 * dm->data is the entry data values
 * dm->mask is the entry mask bits,
 *     1's for bits we want to match in "data",
 *     0's for bits in "data" we want to ignore.
 *
 * {X Y} result
 * -------
 * {0 0} (always match)
 * {0 1} match if 0
 * {1 0} match if 1
 * {1 1} (never match)
 */
static void
pmt_make_entry(pmt_entry_t *pmte, const pmt_datamask_t *dm)
{
    const u_int64_t data = dm->data.all;
    const u_int64_t mask = dm->mask.all;

    pmte->tcam.x =  data & mask;
    pmte->tcam.y = ~data & mask;
    pmte->tcam.v = 1;
}

static void
pmt_datamask_get(pmt_datamask_t *dm, const pmt_entry_t *pmte)
{
    dm->data.all = pmte->tcam.x;
    dm->mask.all = pmte->tcam.x ^ pmte->tcam.y;
}

static void
pmt_make_cfg(pmt_t *pmt,
             pciehwdev_t *phwdev,
             const u_int64_t cfgpa,
             const u_int16_t addr,
             const u_int16_t addrm,
             const u_int8_t romsksel,
             const u_int8_t notify,
             const u_int8_t indirect)
{
    const u_int64_t cfgpadw = cfgpa >> 2;
    const u_int16_t bdf = phwdev->bdf;
    pmt_datamask_t dm;
    pmr_cfg_entry_t *r = &pmt->pmre.cfg;

    pciehw_memset(pmt, 0, sizeof(*pmt));

    dm.data.all = 0;
    dm.mask.all = 0;

#define DM_SET_CFG(dm, field, dval, mval)       \
    do {                                        \
        dm.data.cfg.field = dval;               \
        dm.mask.cfg.field = mval;               \
    } while (0)

    DM_SET_CFG(dm, valid, 1, 0x1);
    DM_SET_CFG(dm, tblid, 0, 0x0); /* don't care, for now */
    DM_SET_CFG(dm, type, PMT_TYPE_CFG, 0x7);
    DM_SET_CFG(dm, port, phwdev->port, 0x7);
    DM_SET_CFG(dm, rw, 0, 0x0);
    DM_SET_CFG(dm, bdf, bdf, 0xffff);
    DM_SET_CFG(dm, addrdw, addr >> 2, addrm >> 2);

    pmt_make_entry(&pmt->pmte, &dm);

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
}

static void
pmt_make_bar(pmt_t *pmt,
             const u_int8_t port,
             const u_int16_t bdf,
             const pciehw_spmt_t *spmt)
{
    const u_int64_t addr = spmt->baraddr;
    const u_int64_t addrm = ~((1 << (ffsl(spmt->barsize) - 1)) - 1);
    pmt_datamask_t dm;
    pmr_bar_entry_t *r = &pmt->pmre.bar;

    assert((spmt->prtcount & (spmt->prtcount - 1)) == 0);
    assert((spmt->prtsize & (spmt->prtsize - 1)) == 0);

    pciehw_memset(pmt, 0, sizeof(*pmt));

    dm.data.all = 0;
    dm.mask.all = 0;

#define DM_SET_BAR(dm, field, dval, mval)       \
    do {                                        \
        dm.data.bar.field = dval;               \
        dm.mask.bar.field = mval;               \
    } while (0)

    DM_SET_BAR(dm, valid, 1, 0x1);
    DM_SET_BAR(dm, tblid, 0, 0x0); /* don't care, for now */
    DM_SET_BAR(dm, type, spmt->type, 0x7);
    DM_SET_BAR(dm, port, port, 0x7);
    DM_SET_BAR(dm, rw, 0, 0x0);
    DM_SET_BAR(dm, addrdw, addr >> 2, addrm >> 2);

    pmt_make_entry(&pmt->pmte, &dm);

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
    r->qidend    = 0;
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
    pmt_t pmt;

    pmt_make_cfg(&pmt, phwdev, cfgpa, addr, addrm, romsksel, notify, indirect);
    pmt_set(pmti, &pmt);
}

static void
pmt_set_bar(const int pmti,
            const u_int8_t port,
            const u_int16_t bdf,
            const pciehw_spmt_t *spmt)
{
    pmt_t pmt;

    pmt_make_bar(&pmt, port, bdf, spmt);
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
    const pmr_entry_t pmre = { 0 };
    int i;

    for (i = 0; i < pmt_size(); i++) {
        pmr_set_entry(i, &pmre);
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
    pciehbarreg_t *reg;
    pciehw_spmt_t *spmt;
    int i, pmti, prti;

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

    for (reg = bar->regs, i = 0; i < bar->nregs; i++, reg++) {
        /*
         * qtypestart/mask are stored in PMT so this value
         * is shared for all regions, so all regions must
         * request the same value.  Verify that if a region
         * requests a qtypestart/mask that it matches what
         * we have already.
         */
        if (reg->qtywidth) {
            const u_int8_t qtypestart = reg->qtyshift;
            const u_int8_t qtypemask = (1 << reg->qtywidth) - 1;

            if (!spmt->qtypemask) {
                spmt->qtypestart = qtypestart;
                spmt->qtypemask = qtypemask;
            } else {
                assert(spmt->qtypestart == qtypestart);
                assert(spmt->qtypemask == qtypemask);
            }
        }
        /*
         * npids is a property of PMT so npids must have
         * the same value for all regions.
         */
        if (reg->npids) {
            const unsigned int npids = reg->npids;

            /* must be power-of-2 */
            assert((npids & (npids - 1)) == 0);
            if (!spmt->npids) {
                spmt->npids = npids;
            } else {
                assert(spmt->npids == npids);
            }
        }
    }
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

#define PMTF_BAR        0x01
#define PMTF_CFG        0x02
#define PMTF_RAW        0x04

static int last_hdr_displayed;

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
pmt_show_cfg_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pmt_cfg_format_t *d = &dm->data.cfg;
    const pmt_cfg_format_t *m = &dm->mask.cfg;
    const pmr_cfg_entry_t *r = &pmt->pmre.cfg;
    const int rw = d->rw;
    const int rw_m = m->rw;

    if (last_hdr_displayed != PMTF_CFG) {
        pmt_show_cfg_entry_hdr();
        last_hdr_displayed = PMTF_CFG;
    }

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
    pciehsys_log("%-4s %-2s %-3s %-2s %-9s %-10s "
                 "%-5s %-5s %-5s %-5s %-5s\n",
                 "idx", "id", "typ", "rw", "p:bb:dd.f", "baraddr",
                 " vf  ", " lif ", " prt ", " pid ", "qtype");

}

static u_int32_t
pmr_pagesize_dec(const u_int32_t encoded_pagesize)
{
    static u_int32_t pagesize_tab[8] = {
        0x1000, 0x2000, 0x10000, 0x40000, 0x100000, 0x400000, 0, 0
    };

    return pagesize_tab[encoded_pagesize & 0x7];
}

static void
pmt_show_bar_entry(const int pmti, const pmt_t *pmt, const pmt_datamask_t *dm)
{
    const pmt_bar_format_t *d = &dm->data.bar;
    const pmt_bar_format_t *m = &dm->mask.bar;
    const pmr_bar_entry_t *r = &pmt->pmre.bar;
    const u_int32_t pagesize = pmr_pagesize_dec(r->pagesize);
    const int rw = d->rw;
    const int rw_m = m->rw;
    int pid_s, pid_e, qtype_s, qtype_e;
    int vf_s, vf_e, lif_s, lif_e, prt_s, prt_e;

    if (last_hdr_displayed != PMTF_BAR) {
        pmt_show_bar_entry_hdr();
        last_hdr_displayed = PMTF_BAR;
    }

    vf_s = r->vfstart;
    vf_e = r->vfend - 1;

    lif_s = lif_e = 0;
    prt_s = prt_e = 0;
    pid_s = pid_e = 0;
    qtype_s = qtype_e = 0;    

    if (r->qtypemask) {
        /* 64b db pmt entry */
        lif_s = r->prtsize;
        lif_e = r->vfstart;

        prt_s = r->prtsize; /* no real prt selection for 64b db */
        prt_e = r->prtsize;

        pid_s = ffs(pagesize) - 1;
        pid_e = r->prtsize - 1;

        qtype_s = r->qtypestart;
        qtype_e = r->qtypemask ? qtype_s + ffs(r->qtypemask + 1) - 2 : qtype_s;

    } else if (r->qidend) {
        /* 32b/16b db pmt entry */
        lif_s = lif_e = -1;
        qtype_s = qtype_e = -1;

    } else {
        /* resource pmt entry */
        prt_s = r->prtsize;
        prt_e = r->prtsize + (ffs(r->prtsize) - 1);
    }

    pciehsys_log("%4d %2d %-3s %c%c %1d:%-7s 0x%08"PRIx64" "
                 "%2d:%-2d %2d:%-2d %2d:%-2d %2d:%-2d %2d:%-2d\n",
                 pmti, d->tblid,
                 d->type == r->type ? pmt_type_str(d->type) : "BAD",
                 ((!rw && rw_m) || !rw_m) ? 'r' : ' ',
                 (( rw && rw_m) || !rw_m) ? 'w' : ' ',
                 d->port,
                 bdf_to_str(r->bdf),
                 (u_int64_t)d->addrdw << 2,
                 vf_e, vf_s,
                 lif_e, lif_s,
                 prt_e, prt_s,
                 pid_e, pid_s,
                 qtype_e, qtype_s);
}

static void
pmt_show_raw_entry_hdr(void)
{
    pciehsys_log("%-4s %-16s %-16s %-3s\n",
                 "idx", "x", "y", "pmr");
}

static void
pmt_show_raw_entry(const int pmti,
                   const pmt_t *pmt,
                   const pmt_datamask_t *dm)
{
    const pmt_tcam_t *tcam = &pmt->pmte.tcam;
    const u_int32_t *w = pmt->pmre.w;

    if (last_hdr_displayed != PMTF_RAW) {
        pmt_show_raw_entry_hdr();
        last_hdr_displayed = PMTF_RAW;
    }

    pciehsys_log("%4d %016"PRIx64" %016"PRIx64" %08x %08x %08x %08x\n",
                 pmti, tcam->x, tcam->y,
                 w[0], w[1], w[2], w[3]);
}

static void
pmt_show_entry(const int pmti, const pmt_t *pmt, const int flags)
{
    pmt_datamask_t dm;
    const pmt_cmn_format_t *cmn;

    pmt_datamask_get(&dm, &pmt->pmte);
    cmn = &dm.data.cmn;

    switch (cmn->type) {
    case PMT_TYPE_CFG:
        if (flags & PMTF_CFG) {
            if (flags & PMTF_RAW) {
                pmt_show_raw_entry(pmti, pmt, &dm);
            } else {
                pmt_show_cfg_entry(pmti, pmt, &dm);
            }
        }
        break;
    case PMT_TYPE_MEM:
    case PMT_TYPE_IO:
        if (flags & PMTF_BAR) {
            if (flags & PMTF_RAW) {
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
    for (i = 0; i < pmt_size(); i++) {
        pmt_get(i, &pmt);
        if (pmt_is_valid(&pmt)) {
            pmt_show_entry(i, &pmt, flags);
        }
    }
}

void
pciehw_pmt_dbg(int argc, char *argv[])
{
    int opt, flags;

    flags = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "bcr")) != -1) {
        switch (opt) {
        case 'b':
            flags |= PMTF_BAR;
            break;
        case 'c':
            flags |= PMTF_CFG;
            break;
        case 'r':
            flags |= PMTF_RAW;
            break;
        default:
            return;
        }
    }
    if ((flags & (PMTF_BAR | PMTF_CFG)) == 0) {
        flags |= PMTF_BAR | PMTF_CFG;
    }

    pmt_show(flags);
}
