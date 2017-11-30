/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "pciehbar.h"
#include "pciehbar_impl.h"
#include "pciehsys.h"

/*****************************************************************
 * bar - single bar
 */

void
pciehbar_delete(pciehbar_t *pbar)
{
    if (pbar == NULL) return;
    pciehsys_free(pbar->regs);
}

void
pciehbar_add_reg(pciehbar_t *pbar, const pciehbarreg_t *preg)
{
    pbar->regs = pciehsys_realloc(pbar->regs,
                                  (pbar->nregs + 1) * sizeof(*preg));
    pbar->regs[pbar->nregs++] = *preg;
}

static void
pciehbar_finalize(pciehbar_t *pbar)
{
    pciehbarreg_t *preg;
    u_int32_t baroff;
    int i;

    baroff = 0;
    for (preg = pbar->regs, i = 0; i < pbar->nregs; i++, preg++) {
        preg->baroff = baroff;
        if (preg->flags & PCIEHBARREGF_MSIX_TBL) {
            pbar->msix_tbl = 1;
        }
        if (preg->flags & PCIEHBARREGF_MSIX_PBA) {
            pbar->msix_pba = 1;
        }
        baroff += preg->size;
    }
    pbar->size = roundup_power2(baroff);
}

/*****************************************************************
 * bars
 */

pciehbars_t *
pciehbars_new(void)
{
    pciehbars_t *pbars = pciehsys_zalloc(sizeof(pciehbars_t));

    return pbars;
}

void
pciehbars_delete(pciehbars_t *pbars)
{
    pciehbar_t *b;
    int i;

    if (pbars == NULL) return;

    for (b = pbars->bars, i = 0; i < pbars->nbars; i++, b++) {
        pciehbar_delete(b);
    }

    pciehsys_free(pbars->bars);
    pciehsys_free(pbars);
}

void
pciehbars_add_bar(pciehbars_t *pbars, const pciehbar_t *pnewbar)
{
    pciehbar_t *pbar;

    assert(pbars->nbars < 6);

    pbars->bars = pciehsys_realloc(pbars->bars,
                                   (pbars->nbars + 1) * sizeof(*pbar));
    pbar = &pbars->bars[pbars->nbars++];
    *pbar = *pnewbar;

    pciehbar_finalize(pbar);
}

void
pciehbars_finalize(pciehbars_t *pbars)
{
    pciehbar_t *b;
    pciehbarreg_t *r;
    u_int8_t cfgidx;
    int bi, i;

    if (pbars == NULL) {
        return;
    }

    cfgidx = 0;
    for (b = pbars->bars, bi = 0; bi < pbars->nbars; bi++, b++) {

        /* If this bar has msix_tbl/pba, find region offset. */
        if (b->msix_tbl || b->msix_pba) {
            for (r = b->regs, i = 0; i < b->nregs; i++, r++) {
                if (r->flags & PCIEHBARREGF_MSIX_TBL) {
                    pbars->msix_tblbir = b->cfgidx;
                    pbars->msix_tbloff = r->baroff;
                }
                if (r->flags & PCIEHBARREGF_MSIX_PBA) {
                    pbars->msix_pbabir = b->cfgidx;
                    pbars->msix_pbaoff = r->baroff;
                }
            }
        }

        /* Config space index 0-5 of this bar. */
        assert(cfgidx < 6);
        b->cfgidx = cfgidx;

        switch (b->type) {
        case PCIEHBARTYPE_IO:
        case PCIEHBARTYPE_MEM:   cfgidx += 1; break;
        case PCIEHBARTYPE_MEM64: cfgidx += 2; break;
        default: break;
        }
    }
}

u_int8_t
pciehbars_get_msix_tblbir(pciehbars_t *pbars)
{
    return pbars ? pbars->msix_tblbir : 0;
}

u_int32_t
pciehbars_get_msix_tbloff(pciehbars_t *pbars)
{
    return pbars ? pbars->msix_tbloff : 0;
}

u_int8_t
pciehbars_get_msix_pbabir(pciehbars_t *pbars)
{
    return pbars ? pbars->msix_pbabir : 0;
}

u_int32_t
pciehbars_get_msix_pbaoff(pciehbars_t *pbars)
{
    return pbars ? pbars->msix_pbaoff : 0;
}

pciehbar_t *
pciehbars_get_first(pciehbars_t *pbars)
{
    return (pbars && pbars->nbars > 0) ? pbars->bars : NULL;
}

pciehbar_t *
pciehbars_get_next(pciehbars_t *pbars, pciehbar_t *pbar)
{
    int baridx = pbar - pbars->bars;
    assert(pbars->bars <= pbar && pbar < &pbars->bars[pbars->nbars]);
    return baridx + 1 < pbars->nbars ? pbar + 1 : NULL;
}
