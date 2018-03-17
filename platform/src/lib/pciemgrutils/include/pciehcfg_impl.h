/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHCFG_IMPL_H__
#define __PCIEHCFG_IMPL_H__

#define PCIEHCFGSZ 1024

typedef struct pciehcfg_s {
    u_int8_t cap_gen;           /* PCIe GenX (1,2,3,4) */
    u_int8_t cap_width;         /* lane width xX (1,2,4,8,16) */
    u_int16_t vendorid;         /* vendor id */
    u_int16_t deviceid;         /* device id */
    u_int16_t subvendorid;      /* subvendorid */
    u_int16_t subdeviceid;      /* subdeviceid */
    u_int32_t classcode;        /* device classcode */
    u_int16_t nintrs;           /* number of MSI/-X interrupts */
    u_int8_t revid;             /* device revision id */
    u_int8_t intpin;            /* legacy int pin IntA|B|C|D = 1|2|3|4 */
    u_int8_t msix_tblbir;       /* msix bar index 0-5 */
    u_int8_t msix_pbabir;       /* msix bar index 0-5 */
    u_int32_t msix_tbloff;      /* msix table offset */
    u_int32_t msix_pbaoff;      /* msix pending bit array offset */
    u_int64_t dsn;              /* device serial number */
    u_int32_t flr:1;            /* device supports Function Level Reset */
    u_int32_t exttag:1;         /* extended tag capable */
    u_int32_t exttag_en:1;      /* extended tag enabled */
    u_int32_t msicap:1;         /* msi  cap (if intrs) */
    u_int32_t msixcap:1;        /* msix cap (if intrs) */
    u_int32_t bridgeup:1;       /* bridge upstream port */
    u_int32_t bridgedn:1;       /* bridge downstream port */
    u_int32_t fnn:1;            /* multi-function device, not function 0 */
    u_int32_t vf:1;             /* sriov vf */
    u_int8_t cap_cursor;        /* current capabilities position */
    u_int16_t extcap_cursor;    /* current extended capabilities position */
    u_int8_t *cur;              /* config space contents */
    u_int8_t *msk;              /* config space write mask */
} pciehcfg_t;

#endif /* __PCIEHCFG_IMPL_H__ */
