/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/src/lib/misc/include/misc.h"
#include "pmt.h"
#include "prt.h"
#include "pciesys.h"
#include "pciehbar.h"
#include "pciehbar_impl.h"

static void
pciehbarreg_delete(pciehbarreg_t *preg)
{
    if (preg == NULL) return;
    pciesys_free(preg->prts);
}

/*****************************************************************
 * bar - single bar
 */

void
pciehbar_delete(pciehbar_t *pbar)
{
    int i;
    pciehbarreg_t *preg;

    if (pbar == NULL) return;

    for (preg = pbar->regs, i = 0; i < pbar->nregs; i++, preg++) {
        pciehbarreg_delete(preg);
    }
    pciesys_free(pbar->regs);
}

void
pciehbar_add_reg(pciehbar_t *pbar, const pciehbarreg_t *preg)
{
    pbar->regs = pciesys_realloc(pbar->regs,
                                 (pbar->nregs + 1) * sizeof(*preg));
    pbar->regs[pbar->nregs++] = *preg;
}

void
pciehbarreg_add_prt(pciehbarreg_t *preg, const prt_t *prt)
{
    preg->prts = pciesys_realloc(preg->prts, (preg->nprts + 1) * sizeof(*prt));
    preg->prts[preg->nprts++] = *prt;
}

/*****************************************************************
 * bars
 */

pciehbars_t *
pciehbars_new(void)
{
    pciehbars_t *pbars = pciesys_zalloc(sizeof(pciehbars_t));

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

    pciesys_free(pbars->bars);
    pciesys_free(pbars);
}

void
pciehbars_add_bar(pciehbars_t *pbars, const pciehbar_t *pnewbar)
{
    pciehbar_t *pbar;

    assert(pbars->nbars < 6);

    pbars->bars = pciesys_realloc(pbars->bars,
                                  (pbars->nbars + 1) * sizeof(*pbar));
    pbar = &pbars->bars[pbars->nbars++];
    *pbar = *pnewbar;
}

void
pciehbars_finalize(pciehbars_t *pbars)
{
}

void
pciehbars_set_msix_tbl(pciehbars_t *pbars, const int bir, const int off)
{
    if (pbars) {
        pbars->msix_tblbir = bir;
        pbars->msix_tbloff = off;
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

void
pciehbars_set_msix_pba(pciehbars_t *pbars, const int bir, const int off)
{
    if (pbars) {
        pbars->msix_pbabir = bir;
        pbars->msix_pbaoff = off;
    }
}

u_int8_t
pciehbars_get_msix_pbabir(pciehbars_t *pbars)
{
    return pbars ? pbars->msix_pbabir : 0;
}

void
pciehbars_set_msix_pbaoff(pciehbars_t *pbars, const int off)
{
    if (pbars) pbars->msix_pbaoff = off;
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
    const int baridx = pbar - pbars->bars;
    assert(pbars->bars <= pbar && pbar < &pbars->bars[pbars->nbars]);
    return baridx + 1 < pbars->nbars ? pbar + 1 : NULL;
}
