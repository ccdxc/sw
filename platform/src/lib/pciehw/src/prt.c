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
#include <sys/param.h>

#include "misc.h"
#include "bdf.h"
#include "pal.h"
#include "pciehsys.h"
#include "pciehost.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "prt.h"

static int
prt_size(void)
{
    return PRT_COUNT;
}

static char *
prt_type_str(int type)
{
    static char *typestr[] = {
        "res", "db64", "db32", "db16"
    };
    return typestr[type & 0x3];
}

static int
prt_alloc(pciehw_t *phw, const pciehwdevh_t hwdevh, const int n)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    int prti;

    if (phwmem->allocprt + n >= prt_size()) {
        return -1;
    }
    prti = phwmem->allocprt;
    phwmem->allocprt += n;
    return prti;
}

static void
prt_free(pciehw_t *phw, const int prtbase, const int prtsize)
{
    /* XXX */
}

static u_int64_t
prt_addr(const int prti)
{
    return PRT_BASE + (prti * PRT_STRIDE);
}

static void
prt_get(pciehw_t *phw, const int prti, prt_t prt)
{
    pal_reg_rd32w(prt_addr(prti), prt, PRT_NWORDS);
}

static void
prt_set(pciehw_t *phw, const int prti, const prt_t prt)
{
    pal_reg_wr32w(prt_addr(prti), prt, PRT_NWORDS);
}

/*
 * Sizes are encoded in the PRT entry using this format.
 *
 * If bit10 == 0, then
 *
 *     10 9          0
 *     +-+-----+-----+
 *     |0|   sizedw  |
 *     +-+-----+-----+
 *
 *     effective size = sizedw;
 *
 * If bit11 == 1, then
 *
 *     10 9   5 4   0
 *     +-+-----+-----+
 *     |1| rv  | ru  |
 *     +-+-----+-----+
 *
 * ru is a power-of-2 "resource units", and
 * rv is the "resource value" multiplier.
 * so effective size is:
 *
 *     effective size = rv << ru;
 *
 * All sizes here are 4-byte "dwords".
 */
static u_int32_t
prt_size_encode(const u_int64_t size)
{
    const u_int64_t sizedw = size >> 2;
    u_int32_t rv, ru;

    /* multiple of dword */
    assert((size & 0x3) == 0);

    /*
     * Any size value < (1 << 10) is encoded
     * as simple value;
     */
    if (sizedw < (1 << 10)) {
        return sizedw;
    }

    /*
     * Size >= (1 << 10) are encoded such that
     * ru is a power-of-2 "resource units", and
     * rv is the "resource value" multiplier.
     * so effective size is:
     *
     *     effective size = rv << ru;
     */
    ru = MIN(ffsll(sizedw) - 1, 31);
    rv = sizedw >> ru;

    assert(rv < (1 << 5));
    return (1 << 10) | (rv << 5) | ru;
}

static u_int64_t
prt_size_decode(const u_int32_t size_enc)
{
    u_int64_t sizedw;

    if (size_enc & (1 << 10)) {
        const u_int64_t rv = (size_enc >> 5) & 0x1f;
        const u_int64_t ru = (size_enc     ) & 0x1f;

        sizedw = rv << ru;
    } else {
        sizedw = size_enc;
    }
    return sizedw << 2;
}

static void
prt_set_res(pciehw_t *phw,
            const int prti,
            const u_int64_t addr,
            const u_int64_t size)
{
    prt_t prt = { 0 };
    prt_res_t *p = (prt_res_t *)prt;
    const u_int32_t size_enc = prt_size_encode(size);

    p->valid     = 1;
    p->type      = PRT_TYPE_RES;
    p->indirect  = 0;
    p->notify    = phw->hwparams.force_notify_bar;
    p->vfstride  = 0;
    p->aspace    = 0;    /* local addr */
    p->addrdw    = addr >> 2;
    p->sizedw    = size_enc;
    p->pmvdis    = 0;

    prt_set(phw, prti, prt);
}

static void
prt_set_db(pciehw_t *phw,
           const int prti,
           const int dbtype,
           const u_int32_t lif,
           const u_int64_t updvec,
           const u_int8_t stridesel,
           const u_int8_t idxshift,
           const u_int8_t idxwidth,
           const u_int8_t qidshift,
           const u_int8_t qidwidth,
           const u_int8_t qidsel)
{
    prt_t prt = { 0 };
    prt_db_t *p = (prt_db_t *)prt;

    p->valid     = 1;
    p->type      = dbtype;
    p->indirect  = 0;
    p->notify    = phw->hwparams.force_notify_bar;
    p->vfstride  = 0;
    p->lif       = lif;
    p->updvec    = updvec;
    p->stridesel = stridesel;
    p->idxshift  = idxshift;
    p->idxwidth  = idxwidth;
    p->qidshift  = qidshift;
    p->qidwidth  = qidwidth;
    p->qidsel    = qidsel;

    prt_set(phw, prti, prt);
}

static void
prt_set_db64(pciehw_t *phw,
             const int prti,
             const u_int32_t lif,
             const u_int64_t updvec)
{
    const u_int8_t dbtype    = PRT_TYPE_DB64;
    const u_int8_t stridesel = 0x1;
    /* these are unused for 64-bit doorbells */
    const u_int8_t idxshift  = 0;
    const u_int8_t idxwidth  = 0;
    const u_int8_t qidshift  = 0;
    const u_int8_t qidwidth  = 0;
    const u_int8_t qidsel    = 0;

    prt_set_db(phw, prti, dbtype, lif, updvec, stridesel,
               idxshift, idxwidth, qidshift, qidwidth, qidsel);
}

static void
prt_init(pciehw_t *phw)
{
    const prt_t prt = { 0 };
    int i;

    for (i = 0; i < prt_size(); i++) {
        prt_set(phw, i, prt);
    }
}

static u_int8_t
regupd2hwupd(const u_int8_t regupd)
{
    u_int8_t hwupd = 0;

    switch (regupd & PCIEHBARUPD_SCHED_BITS) {
    case PCIEHBARUPD_SCHED_EVAL:  hwupd |= 0x1; break;
    case PCIEHBARUPD_SCHED_CLEAR: hwupd |= 0x2; break;
    case PCIEHBARUPD_SCHED_SET:   hwupd |= 0x3; break;
    }
    switch (regupd & PCIEHBARUPD_PICI_BITS) {
    case PCIEHBARUPD_PICI_CISET:  hwupd |= 0x4; break;
    case PCIEHBARUPD_PICI_PISET:  hwupd |= 0x8; break;
    case PCIEHBARUPD_PICI_PIINC:  hwupd |= 0xc; break;
    }
    if (regupd & PCIEHBARUPD_PID_CHECK) {
        hwupd |= 0x10;
    }
    return hwupd;
}

static u_int64_t
mkupdvec(const u_int8_t upd[8])
{
    u_int64_t updvec = 0;
    int i;

    for (i = 0; i < 8; i++) {
        const u_int64_t upde = regupd2hwupd(upd[i]);
        updvec |= upde << (i * 5);
    }
    return updvec;
}

/******************************************************************
 * apis
 */

int
pciehw_prt_load(pciehwdev_t *phwdev, pciehbar_t *bar)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehwbar_t *phwbar;
    pciehw_sprt_t *sprt;
    pciehbarreg_t *reg;
    u_int64_t updvec;
    int i, prtbase;

    prtbase = prt_alloc(phw, hwdevh, bar->nregs);
    if (prtbase < 0) {
        pciehsys_error("prt_load: prt_alloc %d failed\n", bar->nregs);
        return -1;
    }

    phwbar = &phwdev->bar[bar->cfgidx];
    phwbar->prtbase = prtbase;
    phwbar->prtcount = bar->nregs;
    phwbar->prtsize = bar->size / bar->nregs;

    reg = bar->regs;
    sprt = &phwmem->sprt[prtbase];
    for (i = 0; i < bar->nregs; i++, sprt++, reg++) {
        sprt->owner = hwdevh;
        sprt->resaddr = reg->paddr;
        sprt->ressize = reg->size;

        switch (reg->regtype) {
        case PCIEHBARREGT_RES:
            prt_set_res(phw, prtbase + i, sprt->resaddr, sprt->ressize);
            break;
        case PCIEHBARREGT_DB64:
            updvec = mkupdvec(reg->upd);
            prt_set_db64(phw, prtbase + i, phwdev->lif, updvec);
            break;
        case PCIEHBARREGT_DB32:
        case PCIEHBARREGT_DB16:
        default:
            assert(0);
            break;
        }
    }
    return 0;
}

void
pciehw_prt_unload(pciehw_t *phw, pciehwdev_t *phwdev)
{
    /* XXX */
    if (0) prt_free(phw, 0, 0);
}

void
pciehw_prt_init(pciehw_t *phw)
{
    prt_init(phw);
}

/******************************************************************
 * debug
 */

/*
idx  typ vfst 0xaddr____ size INP
 */
static void
prt_show_res_entry_hdr(void)
{
    pciehsys_log("%-4s %-4s %-4s %-10s %-4s %-5s\n",
                 "idx", "type", "vfst", "address", "size", "flags");
}

static void
prt_show_res_entry(const int prti, prt_t prt)
{
    const prt_res_t *r = (prt_res_t *)prt;

    pciehsys_log("%4d %-4s %4d 0x%08"PRIx64" %-4s %c%c%c\n",
                 prti,
                 prt_type_str(r->type),
                 r->vfstride,
                 (u_int64_t)r->addrdw << 2,
                 human_readable(prt_size_decode(r->sizedw)),
                 r->indirect ? 'i' : '-',
                 r->notify ? 'n' : '-',
                 r->aspace ? 'h' : '-');
}

static char *
updvec_str(const u_int64_t updvec, const int raw)
{
    static char buf[32];
    char sched, pici, pidchk, *bp;
    int i;

    bp = buf;
    for (i = 0; i < 8; i++) {
        const u_int8_t upde = (updvec >> (i * 5)) & 0x1f;

        if (raw) {
            snprintf(bp, sizeof(buf) - i * 3,
                     "%02x%s", upde, i < 7 ? ":" : "");
            bp += 3;
            continue;
        }

        switch (upde & 0x3) { /* sched bits */
        case 0: sched = '-'; break;
        case 1: sched = 'e'; break;
        case 2: sched = 'c'; break;
        case 3: sched = 's'; break;
        }
        switch ((upde >> 2) & 0x3) { /* pici bits */
        case 0: pici = '-'; break;
        case 1: pici = 'c'; break;
        case 2: pici = 'p'; break;
        case 3: pici = 'P'; break;
        }
        pidchk = upde & 0x10 ? 'p' : '-';

        snprintf(bp, sizeof(buf) - i * 4,
                 "%c%c%c%s", sched, pici, pidchk, i < 7 ? ":" : "");
        bp += 4;
    }
    return buf;
}

static void
prt_show_db_entry_hdr(void)
{
    pciehsys_log("%-4s %-4s %-4s %-4s "
                 "%-4s %-4s "
                 "%-5s %-31s\n",
                 "idx", "type", "vfst", "lif",
                 "idx", "qid",
                 "flags", "upd vector");
}

static void
prt_show_db_entry(const int prti, prt_t prt, const int raw)
{
    const prt_db_t *r = (prt_db_t *)prt;

    pciehsys_log("%4d %-4s %4d %4d %1d:%-2d %1d:%-2d %c%c%c   %-31s\n",
                 prti,
                 prt_type_str(r->type),
                 r->vfstride,
                 r->lif,
                 r->idxshift, r->idxwidth,
                 r->qidshift, r->qidwidth,
                 r->indirect ? 'i' : '-',
                 r->notify   ? 'n' : '-',
                 r->qidsel   ? 'a' : 'd',
                 updvec_str(r->updvec, raw));
}

static int last_hdr_displayed = -1;

static void
prt_show_entry(const int prti, prt_t prt, const int raw)
{
    prt_common_t *prt_common = (prt_common_t *)prt;

    switch (prt_common->type) {
    case PRT_TYPE_RES:
        if (last_hdr_displayed != PRT_TYPE_RES) {
            prt_show_res_entry_hdr();
        }
        prt_show_res_entry(prti, prt);
        break;
    case PRT_TYPE_DB64:
    case PRT_TYPE_DB32:
    case PRT_TYPE_DB16:
        if (last_hdr_displayed != PRT_TYPE_DB64 &&
            last_hdr_displayed != PRT_TYPE_DB32 &&
            last_hdr_displayed != PRT_TYPE_DB16) {
            prt_show_db_entry_hdr();
        }
        prt_show_db_entry(prti, prt, raw);
        break;
    default:
        break;
    }
    last_hdr_displayed = prt_common->type;
}

static void
prt_show(const int raw)
{
    pciehw_t *phw = pciehw_get();
    prt_t prt;
    int i;

    last_hdr_displayed = -1;
    for (i = 0; i < prt_size(); i++) {
        prt_common_t *prt_common = (prt_common_t *)prt;
        prt_get(phw, i, prt);
        if (prt_common->valid) {
            prt_show_entry(i, prt, raw);
        }
    }
}

void
pciehw_prt_dbg(int argc, char *argv[])
{
    int raw = 0;
    int opt;

    optind = 0;
    while ((opt = getopt(argc, argv, "r")) != -1) {
        switch (opt) {
        case 'r': raw = 1; break;
        default:
            return;
        }
    }

    prt_show(raw);
}
