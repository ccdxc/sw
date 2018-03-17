/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHBAR_IMPL_H__
#define __PCIEHBAR_IMPL_H__

typedef struct pciehbars_s {
    u_int16_t flags;            /* PCIEHBARSF_* flags below */
    u_int8_t msix_tblbir;       /* msix bar index 0-5 */
    u_int8_t msix_pbabir;       /* msix bar index 0-5 */
    u_int32_t msix_tbloff;      /* msix table offset */
    u_int32_t msix_pbaoff;      /* msix pending bit array offset */
    u_int8_t nbars;             /* number of entries in bars[] */
    pciehbar_t *bars;           /* bar table */
    pciehbar_t *rombar;         /* rombar */
} pciehbars_t;

#endif /* __PCIEHBAR_IMPL_H__ */
