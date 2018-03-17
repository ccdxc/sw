/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __USE_GNU
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <fcntl.h>

#include "misc.h"
#include "bdf.h"
#include "pmt.h"

int
pmt_is_valid(const pmt_t *pmt)
{
    return pmt->pmte.tcam.v;
}

const char *
pmt_type_str(int type)
{
    static const char *typestr[8] = {
        "cfg", "mem", "rc", "UNK3", "UNK4", "io", "UNK6", "UNK7"
    };
    return typestr[type & 0x7];
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
void
pmt_entry_enc(pmt_entry_t *pmte, const pmt_datamask_t *dm)
{
    const u_int64_t data = dm->data.all;
    const u_int64_t mask = dm->mask.all;

    pmte->tcam.x =  data & mask;
    pmte->tcam.y = ~data & mask;
    pmte->tcam.v = 1;
}

/*
 * Fetch the dm->data/mask values from this pmt_entry.
 */
void
pmt_entry_dec(const pmt_entry_t *pmte, pmt_datamask_t *dm)
{
    dm->data.all = pmte->tcam.x;
    dm->mask.all = pmte->tcam.x ^ pmte->tcam.y;
}

u_int32_t
pmr_pagesize_enc(const u_int32_t pagesize)
{
    u_int32_t encoded_pagesize;

    switch (pagesize) {
    case 0x00001000: encoded_pagesize = 0; break;
    case 0x00002000: encoded_pagesize = 1; break;
    case 0x00010000: encoded_pagesize = 2; break;
    case 0x00040000: encoded_pagesize = 3; break;
    case 0x00100000: encoded_pagesize = 4; break;
    case 0x00400000: encoded_pagesize = 5; break;
    default:
        assert(0);
        encoded_pagesize = 0;
        break;
    }
    return encoded_pagesize;
}

u_int32_t
pmr_pagesize_dec(const u_int32_t encoded_pagesize)
{
    static u_int32_t pagesize_tab[8] = {
        0x1000, 0x2000, 0x10000, 0x40000, 0x100000, 0x400000, 0, 0
    };

    assert(encoded_pagesize < 6);
    return pagesize_tab[encoded_pagesize & 0x7];
}

void
pmt_bar_set_vfparams(pmt_t *pmt,
                     const int bitb, const int bitc,
                     const int vfb, const int vfc)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmt_bar_setr_vf(pmt, bitb, bitc);
    pmr->vfbase  = vfb;
    pmr->vflimit = vfb + vfc - 1;
}

void
pmt_bar_set_bdf(pmt_t *pmt, const u_int16_t bdf)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmr->bdf = bdf;
}

void
pmt_bar_set_prts(pmt_t *pmt, const u_int32_t prtb, const u_int32_t prtc)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmr->prtb = prtb;
    pmr->prtc = prtc;
}

void
pmt_bar_set_qtype(pmt_t *pmt, const u_int8_t qtstart, const u_int8_t qtmask)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmr->qtypestart = qtstart;
    pmr->qtypemask  = qtmask;
}

void
pmt_bar_set_pagesize(pmt_t *pmt, const u_int32_t pagesize)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const u_int32_t pagesize_enc = pmr_pagesize_enc(pagesize);

    assert((pagesize & (pagesize - 1)) == 0);
    pmr->pagesize = pagesize_enc;
}

u_int32_t
pmt_bar_get_pagesize(const pmt_t *pmt)
{
    const pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    return pmr_pagesize_dec(pmr->pagesize);
}

void
pmt_bar_setr_prt(pmt_t *pmt, const int bitb, const int bitc)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmr->prtsize = bitb;
    pmr->vfstart = bitb + bitc;
}

void
pmt_bar_setr_lif(pmt_t *pmt, const int bitb, const int bitc)
{
    /* lif range in db bar is the same as prt range in res bar */
    pmt_bar_setr_prt(pmt, bitb, bitc);
}

void
pmt_bar_setr_vf(pmt_t *pmt, const int bitb, const int bitc)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmr->vfstart = bitb;
    pmr->vfend   = bitb + bitc;
}

void
pmt_bar_setr_qtype(pmt_t *pmt, const int bitb, const int bitc)
{
    const u_int8_t qtmask = (1 << (bitc + 1)) - 1;
    pmt_bar_set_qtype(pmt, bitb, qtmask);
}

void
pmt_bar_setr_qid(pmt_t *pmt, const int bitb, const int bitc)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    pmr->qidstart = bitb;
    pmr->qidend   = bitc;
}

void
pmt_bar_setaddr(pmt_t *pmt, const u_int64_t addr)
{
    pmt_datamask_t dm;

    pmt_entry_dec(&pmt->pmte, &dm);
    dm.data.bar.addrdw = addr >> 2;
    /* note mask stays the same, so size doesn't change */
    pmt_entry_enc(&pmt->pmte, &dm);
}

u_int64_t
pmt_bar_getaddr(const pmt_t *pmt)
{
    pmt_datamask_t dm;

    pmt_entry_dec(&pmt->pmte, &dm);
    return dm.data.bar.addrdw << 2;
}

u_int64_t
pmt_bar_getsize(const pmt_t *pmt)
{
    pmt_datamask_t dm;

    pmt_entry_dec(&pmt->pmte, &dm);
    return ~(dm.mask.bar.addrdw << 2) + 1;
}

static int
pmt_bar_allows_rw(const pmt_t *pmt, const int rw)
{
    pmt_datamask_t dm;

    pmt_entry_dec(&pmt->pmte, &dm);

    /*
     * Either don't care about rw,
     *     OR
     * rw matches what we are looking for.
     */
    return dm.mask.bar.rw == 0 || dm.data.bar.rw == rw;
}

int
pmt_bar_allows_rd(const pmt_t *pmt)
{
    return pmt_bar_allows_rw(pmt, 0);
}

int
pmt_bar_allows_wr(const pmt_t *pmt)
{
    return pmt_bar_allows_rw(pmt, 1);
}

void
pmt_cfg_enc(pmt_t *pmt,
            const u_int8_t port,
            const u_int16_t bdf,
            const u_int64_t cfgpa,
            const u_int16_t addr,
            const u_int16_t addrm,
            const u_int8_t romsksel,
            const u_int8_t vfstridesel,
            const u_int8_t notify,
            const u_int8_t indirect)
{
    const u_int64_t cfgpadw = cfgpa >> 2;
    pmt_datamask_t dm;
    pmr_cfg_entry_t *pmr = &pmt->pmre.cfg;

    memset(pmt, 0, sizeof(*pmt));

    dm.data.all = 0;
    dm.mask.all = 0;

#define DM_SET_CFG(dm, field, dval, mval)       \
    do {                                        \
        dm.data.cfg.field = dval;               \
        dm.mask.cfg.field = mval;               \
    } while (0)

    DM_SET_CFG(dm, valid, 1, 0x1);
    DM_SET_CFG(dm, tblid, 0, 0x0); /* tblid don't care, for now */
    DM_SET_CFG(dm, type, PMT_TYPE_CFG, 0x7);
    DM_SET_CFG(dm, port, port, 0x7);
    DM_SET_CFG(dm, rw, 0, 0x0);
    DM_SET_CFG(dm, bdf, bdf, 0xffff);
    DM_SET_CFG(dm, addrdw, addr >> 2, addrm >> 2);

    pmt_entry_enc(&pmt->pmte, &dm);

    pmr->valid     = 1;
    pmr->type      = PMT_TYPE_CFG;
    pmr->vfbase    = 0;
    pmr->indirect  = indirect;
    pmr->notify    = notify;
    pmr->pstart    = port;
    pmr->bstart    = bdf_to_bus(bdf);
    pmr->dstart    = bdf_to_dev(bdf);
    pmr->fstart    = bdf_to_fnc(bdf);
    pmr->plimit    = port;
    pmr->blimit    = bdf_to_bus(bdf);
    pmr->dlimit    = bdf_to_dev(bdf);
    pmr->flimit    = bdf_to_fnc(bdf);
    pmr->vfstridesel = vfstridesel;
    pmr->td        = 0;
    pmr->addrdw    = cfgpadw;
    pmr->aspace    = 0;    /* cfgpadw is local not host */
    pmr->romsksel  = romsksel;
}

void
pmt_bar_enc(pmt_t *pmt,
            const u_int8_t port,
            const u_int8_t type,
            const u_int64_t pmtsize,
            const u_int32_t prtsize,
            const u_int32_t pmtflags)
{
    pmr_bar_entry_t *pmr = &pmt->pmre.bar;
    const u_int64_t addrm = ~((1ULL << (ffsll(pmtsize) - 1)) - 1);
    const u_int8_t prtbitb = ffs(prtsize) - 1;
    u_int8_t rw, rwm;
    pmt_datamask_t dm;

    /* must be power2 */
    assert((pmtsize & (pmtsize - 1)) == 0);
    assert((prtsize & (prtsize - 1)) == 0);

    /*
     * Read/write settings.  Default rw mask = 0 so "don't care"
     * matches both reads and writes.
     * If read-only  is desired set rw mask = 1 and match rw=0.
     * If write-only is desired set rw mask = 1 and match rw=1.
     */
    rw = 0; rwm = 0;
    if ((pmtflags & PMT_BARF_RW) == PMT_BARF_RD) {
        rw = 0; rwm = 1;
    } else if ((pmtflags & PMT_BARF_RW) == PMT_BARF_WR) {
        rw = 1; rwm = 1;
    }

    memset(pmt, 0, sizeof(*pmt));

    dm.data.all = 0;
    dm.mask.all = 0;

#define DM_SET_BAR(dm, field, dval, mval)       \
    do {                                        \
        dm.data.bar.field = dval;               \
        dm.mask.bar.field = mval;               \
    } while (0)

    DM_SET_BAR(dm, valid, 1, 0x1);
    DM_SET_BAR(dm, tblid, 0, 0x0); /* don't care, for now */
    DM_SET_BAR(dm, type, type, 0x7);
    DM_SET_BAR(dm, port, port, 0x7);
    DM_SET_BAR(dm, rw, rw, rwm);
    DM_SET_BAR(dm, addrdw, 0, addrm >> 2); /* addr val set via setaddr() */

    pmt_entry_enc(&pmt->pmte, &dm);

    pmr->valid     = 1;
    pmr->type      = type;
    pmr->vfbase    = 0;
    pmr->indirect  = 0;
    pmr->notify    = 0;
    pmr->prtb      = 0;
    pmr->prtc      = 0;
    pmr->prtsize   = prtbitb;
    pmr->vfstart   = pmr->prtsize;
    pmr->vfend     = pmr->vfstart;
    pmr->vflimit   = 0;
    pmr->bdf       = 0;
    pmr->td        = 0;
    pmr->pagesize  = pmr_pagesize_enc(0x1000);  /* default 4k page size */
    pmr->qtypestart= 0;
    pmr->qtypemask = 0;
    pmr->qidstart  = 0;
    pmr->qidend    = 0;
}
