/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/cfgspace/include/cfgspace.h"
#include "cfgspace_bars.h"

static void
cfgspace_set_bar(cfgspace_t *cs,
                 const u_int16_t cfgbase,
                 const cfgspace_bar_t *b)
{
    const u_int16_t cfgoff = cfgbase + (b->cfgidx << 2);
    u_int64_t v, m;

    switch (b->type) {
    case CFGSPACE_BARTYPE_MEM:
        v = 0x0 << 1;   /* 32-bit */
        if (b->prefetch) v |= 0x8;
        m = ~(b->size - 1) & ~0xfUL;
        cfgspace_setdm(cs, cfgoff, v, m);
        break;
    case CFGSPACE_BARTYPE_MEM64:
        v = 0x2 << 1;   /* 64-bit */
        if (b->prefetch) v |= 0x8;
        m = ~(b->size - 1) & ~0xfULL;
        cfgspace_setdm(cs, cfgoff, v, m);
        cfgspace_setdm(cs, cfgoff + 4, v >> 32, m >> 32);
        break;
    case CFGSPACE_BARTYPE_IO:
        v = 0x1;        /* I/O */
        m = ~(b->size - 1) & ~0x3;
        cfgspace_setdm(cs, cfgoff, v, m);
        break;
    default:
        break;
    }
}

void
cfgspace_set_bars(cfgspace_t *cs, const cfgspace_bar_t *bars, const int nbars)
{
    const u_int8_t headertype = cfgspace_get_headertype(cs) & 0x7f;
    const u_int8_t maxidx = headertype == 0 ? 5 : 1;
    const cfgspace_bar_t *b;
    int i;

    for (b = bars, i = 0; i < nbars; i++, b++) {
        assert(b->cfgidx <= maxidx);
        cfgspace_set_bar(cs, 0x10, b);
    }
}

void
cfgspace_set_rombar(cfgspace_t *cs, const cfgspace_bar_t *rombar)
{
    u_int32_t m;

    if (rombar->size == 0) return;

    /* rombar must not be greater than 16 MB. */
    assert(rombar->size <= 16 * 1024 * 1024);

    /* min rombar size is 2048 */
    m = ~(MAX(rombar->size, 2048) - 1);
    m |= 0x1; /* rom enable */
    cfgspace_setdm(cs, 0x30, 0, m);
}

void
cfgspace_set_sriov_bars(cfgspace_t *cs,
                        const u_int16_t cfgbase,
                        const cfgspace_bar_t *bars,
                        const int nbars)
{
    const u_int8_t maxidx = 5;
    const cfgspace_bar_t *b;
    int i;

    for (b = bars, i = 0; i < nbars; i++, b++) {
        assert(b->cfgidx <= maxidx);
        /* no I/O bar for VFs */
        assert(b->type != CFGSPACE_BARTYPE_IO);
        cfgspace_set_bar(cs, cfgbase, b);
    }
}
