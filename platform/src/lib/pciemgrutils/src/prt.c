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

#include "platform/src/lib/misc/include/misc.h"
#include "prt.h"

int
prt_is_valid(const prt_t *prt)
{
    return prt->cmn.valid;
}

u_int32_t
prt_type(const prt_t *prt)
{
    return prt->cmn.type;
}

char *
prt_type_str(const int type)
{
    static char *typestr[4] = {
        "res", "db64", "db32", "db16"
    };
    return typestr[type & 0x3];
}

/*
 * Sizes are encoded in the PRT entry using this format.
 *
 * If bit10 == 0, then
 *
 *     10 9         0
 *     +-+-----+-----+
 *     |0|   sizedw  |
 *     +-+-----+-----+
 *
 *     effective size = sizedw;
 *
 * If bit10 == 1, then
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
 *     effective_size = rv << ru;
 *
 * All sizes here are 4-byte "dwords".
 */
u_int32_t
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
     *     effective_size = rv << ru;
     */
    ru = MIN(ffsll(sizedw) - 1, 31);
    rv = sizedw >> ru;

    assert(rv < (1 << 5));
    return (1 << 10) | (rv << 5) | ru;
}

u_int64_t
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

void
prt_res_enc(prt_t *prt,
            const u_int64_t addr,
            const u_int64_t size,
            const u_int32_t prtflags)
{
    prt_res_t *res = &prt->res;
    const u_int32_t size_enc = prt_size_encode(size);
    const u_int8_t notify   = (prtflags & PRT_RESF_NOTIFY) != 0;
    const u_int8_t indirect = (prtflags & PRT_RESF_INDIRECT) != 0;
    const u_int8_t pmvdis   = (prtflags & PRT_RESF_PMVDIS) != 0;

    memset(prt, 0, sizeof(*prt));

    res->valid     = 1;
    res->type      = PRT_TYPE_RES;
    res->indirect  = indirect;
    res->notify    = notify;
    res->vfstride  = 0;
    res->aspace    = 0;    /* local addr */
    res->addrdw    = addr >> 2;
    res->sizedw    = size_enc;
    res->pmvdis    = pmvdis;
}

static u_int64_t
prt_updvec_enc(const u_int8_t upd[8])
{
    u_int64_t updvec = 0;
    int i;

    for (i = 0; i < 8; i++) {
        updvec |= upd[i] << (i * 5);
    }
    return updvec;
}

static void
prt_db_enc(prt_t *prt,
           const int dbtype,
           const u_int32_t lif,
           const u_int8_t upd[8],
           const u_int8_t stridesel,
           const u_int8_t idxshift,
           const u_int8_t idxwidth,
           const u_int8_t qidshift,
           const u_int8_t qidwidth,
           const u_int8_t qidsel)
{
    prt_db_t *db = &prt->db;
    u_int64_t updvec = prt_updvec_enc(upd);

    memset(prt, 0, sizeof(*prt));

    db->valid     = 1;
    db->type      = dbtype;
    db->indirect  = 0;
    db->notify    = 0;
    db->vfstride  = 0;
    db->lif       = lif;
    db->updvec    = updvec;
    db->stridesel = stridesel;
    db->idxshift  = idxshift;
    db->idxwidth  = idxwidth;
    db->qidshift  = qidshift;
    db->qidwidth  = qidwidth;
    db->qidsel    = qidsel;
}

void
prt_db64_enc(prt_t *prt,
             const u_int32_t lif,
             const u_int8_t upd[8])
{
    const u_int8_t dbtype    = PRT_TYPE_DB64;
    const u_int8_t stridesel = 0x1;
    /* these are unused for 64-bit doorbells */
    const u_int8_t idxshift  = 0;
    const u_int8_t idxwidth  = 0;
    const u_int8_t qidshift  = 0;
    const u_int8_t qidwidth  = 0;
    const u_int8_t qidsel    = 0;

    prt_db_enc(prt, dbtype, lif, upd, stridesel,
               idxshift, idxwidth, qidshift, qidwidth, qidsel);
}

void
prt_db32_enc(prt_t *prt,
             const u_int32_t lif,
             const u_int8_t upd[8])
{
    const u_int8_t dbtype    = PRT_TYPE_DB32;
    const u_int8_t stridesel = 0x1;

    const u_int8_t idxshift  = 0;
    const u_int8_t idxwidth  = 0;
    const u_int8_t qidshift  = 0;
    const u_int8_t qidwidth  = 0;
    const u_int8_t qidsel    = 0;

    prt_db_enc(prt, dbtype, lif, upd, stridesel,
               idxshift, idxwidth, qidshift, qidwidth, qidsel);
}

void
prt_db16_enc(prt_t *prt,
             const u_int32_t lif,
             const u_int8_t upd[8])
{
    const u_int8_t dbtype    = PRT_TYPE_DB16;
    const u_int8_t stridesel = 0x1;

    const u_int8_t idxshift  = 0;
    const u_int8_t idxwidth  = 0;
    const u_int8_t qidshift  = 0;
    const u_int8_t qidwidth  = 0;
    const u_int8_t qidsel    = 0;

    /* XXX this is just a placeholder copy of db32_enc above */
    assert(0);

    prt_db_enc(prt, dbtype, lif, upd, stridesel,
               idxshift, idxwidth, qidshift, qidwidth, qidsel);
}

static void
prt_set_vfstride(prt_t *prt, const u_int8_t vfstride)
{
    prt->cmn.vfstride = ffs(vfstride) - 1;
}

void
prt_res_set_vfstride(prt_t *prt, const u_int8_t vfstride)
{
    prt_set_vfstride(prt, vfstride);
}

void
prt_db_set_vfstride(prt_t *prt, const u_int8_t vfstride)
{
    prt_set_vfstride(prt, vfstride);
}
