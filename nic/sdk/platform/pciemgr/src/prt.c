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
#include <sys/param.h>

#include "platform/misc/include/misc.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

#define PRT_BASE        PXB_(DHS_TGT_PRT)
#define PRT_STRIDE      ASIC_(PXB_CSR_DHS_TGT_PRT_ENTRY_BYTE_SIZE)

static int
prt_count(void)
{
    return PRT_COUNT;
}

static void
assert_prti_in_range(const int prti)
{
    assert(prti >= 0 && prti < prt_count());
}

static void
assert_prts_in_range(const int prtb, const int prtc)
{
    if (prtc > 0) {
        assert_prti_in_range(prtb);
        assert_prti_in_range(prtb + prtc - 1);
    }
}

static u_int64_t
prt_addr(const int prti)
{
    assert_prti_in_range(prti);
    return PRT_BASE + (prti * PRT_STRIDE);
}

int
prt_alloc(const int n)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    int prti;

    if (pshmem->allocprt + n >= prt_count()) {
        return -1;
    }
    prti = pshmem->allocprt;
    pshmem->allocprt += n;
    return prti;
}

void
prt_free(const int prtb, const int prtc)
{
    assert_prts_in_range(prtb, prtc);
    /* XXX */
}

void
prt_get(const int prti, prt_t *prt)
{
    pal_reg_rd32w(prt_addr(prti), prt->w, PRT_NWORDS);
}

void
prt_set(const int prti, const prt_t *prt)
{
    pal_reg_wr32w(prt_addr(prti), prt->w, PRT_NWORDS);
}

static void
prt_clear_all(void)
{
    const prt_t prt0 = { 0 };
    int prti;

    for (prti = 0; prti < prt_count(); prti++) {
        prt_set(prti, &prt0);
    }
}

/******************************************************************
 * apis
 */

#if 0
static void
prt_load_rcdev(const int prtbase, const int prtcount)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const int prtend = prtbase + prtcount;
    pciehw_sprt_t *sprt;
    int prti;

    sprt = &phwmem->sprt[prtbase];
    for (prti = prtbase; prti < prtend; prti++, sprt++) {
        switch (sprt->type) {
        case PRT_TYPE_RES: {
            prt_t prt = { 0 };

            prt_make_res(&prt,
                         sprt->resaddr, sprt->ressize,
                         sprt->notify, sprt->indirect,
                         sprt->pmvdis);
            prt.res.aspace = 1;
            prt_set(prti, &prt);
            break;
        }
        default:
            assert(0);
            break;
        }
    }
}
#endif

int
pciehw_prt_load(const int prtbase, const int prtcount)
{
    const int prtend = prtbase + prtcount;
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_sprt_t *sprt;
    int prti;

    assert_prts_in_range(prtbase, prtcount);

    sprt = &pshmem->sprt[prtbase];

#if 0
    pciehwdev_t *phwdev;

    phwdev = pciehwdev_get(sprt->owner);
    if (strncmp(pciehwdev_get_name(phwdev), "rcdev", 5) == 0) {
        prt_load_rcdev(prtbase, prtcount);
        return 0;
    }
#endif

    for (prti = prtbase; prti < prtend; prti++, sprt++) {
        prt_set(prti, &sprt->prt);
    }
    return 0;
}

void
pciehw_prt_unload(const int prtbase, const int prtcount)
{
    const prt_t prt0 = { 0 };
    const int prtend = prtbase + prtcount;
    int prti;

    assert_prts_in_range(prtbase, prtcount);

    for (prti = prtbase; prti < prtend; prti++) {
        prt_set(prti, &prt0);
    }
}

void
prt_init(void)
{
    STATIC_ASSERT(PRT_COUNT == ASIC_(PXB_CSR_DHS_TGT_PRT_ENTRIES));
    prt_clear_all();
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
    pciesys_loginfo("%-4s %-4s %-4s %-11s %-4s %-5s\n",
                    "idx", "type", "vfst", "address", "size", "flags");
}

static void
prt_show_res_entry(const int prti, prt_t *prt)
{
    const prt_res_t *r = &prt->res;

    pciesys_loginfo("%-4d %-4s %-4d 0x%09"PRIx64" %-4s %c%c%c%c\n",
                    prti,
                    prt_type_str(r->type),
                    r->vfstride,
                    (u_int64_t)r->addrdw << 2,
                    human_readable(prt_size_decode(r->sizedw)),
                    r->indirect ? 'i' : '-',
                    r->notify ? 'n' : '-',
                    r->aspace ? 'h' : '-',
                    r->pmvdis ? 'p' : '-');
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
    pciesys_loginfo("%-4s %-4s %-4s %-4s "
                    "%-4s %-4s "
                    "%-5s %-31s\n",
                    "idx", "type", "vfst", "lif",
                    "idx", "qid",
                    "flags", "upd vector");
}

static void
prt_show_db_entry(const int prti, prt_t *prt, const int raw)
{
    const prt_db_t *db = &prt->db;

    pciesys_loginfo("%-4d %-4s %-4d %-4d %1d:%-2d %1d:%-2d %c%c%c   %-31s\n",
                    prti,
                    prt_type_str(db->type),
                    db->vfstride,
                    db->lif,
                    db->idxshift * 8,
                    db->idxwidth ? db->idxshift * 8 + db->idxwidth - 1 : 0,
                    db->qidshift * 8,
                    db->qidwidth ? db->qidshift * 8 + db->qidwidth - 1 : 0,
                    db->indirect ? 'i' : '-',
                    db->notify   ? 'n' : '-',
                    db->qidsel   ? 'a' : 'd',
                    updvec_str(db->updvec, raw));
}

static int last_hdr_displayed = -1;

static void
prt_show_entry(const int prti, prt_t *prt, const int raw)
{
    const u_int32_t prttype = prt_type(prt);

    switch (prttype) {
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
    last_hdr_displayed = prttype;
}

static void
prt_show(const int raw)
{
    prt_t prt;
    int prti;

    last_hdr_displayed = -1;
    for (prti = 0; prti < prt_count(); prti++) {
        prt_get(prti, &prt);
        if (prt_is_valid(&prt)) {
            prt_show_entry(prti, &prt, raw);
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
