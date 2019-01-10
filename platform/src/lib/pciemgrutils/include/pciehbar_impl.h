/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#ifndef __PCIEHBAR_IMPL_H__
#define __PCIEHBAR_IMPL_H__

#define PCIEHBAR_NBARS  6

typedef struct pciehbars_s {
    u_int8_t msix_tblbir;       /* msix bar index 0-5 */
    u_int8_t msix_pbabir;       /* msix bar index 0-5 */
    u_int32_t msix_tbloff;      /* msix table offset */
    u_int32_t msix_pbaoff;      /* msix pending bit array offset */
    pciehbar_t bars[PCIEHBAR_NBARS]; /* bar table */
    pciehbar_t rombar;          /* rombar */
} pciehbars_t;

#endif /* __PCIEHBAR_IMPL_H__ */
