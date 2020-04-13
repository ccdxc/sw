/*
 * Copyright (c) 2017-2020, Pensando Systems Inc.
 */

#ifndef __PCIEMGR_H__
#define __PCIEMGR_H__

typedef enum pciemgr_initmode_e {
    INITMODE_NONE = 0,          /* uninitialized init mode */
    INHERIT_ONLY,               /* passive clients: pcieutil, debug, etc */
    INHERIT_OK,                 /* pciemgrd restart */
    FORCE_INIT,                 /* pciemgrd must init */
} pciemgr_initmode_t;

#define PCIEMGR_STRSZ   80      /* size of vpd string params */

typedef struct pciemgr_params_s {
    u_int8_t cap_gen;           /* default GenX capability (1,2,3,4) */
    u_int8_t cap_width;         /* xX lane width (1,2,4,8,16) */
    u_int16_t vendorid;         /* default vendorid */
    u_int16_t subvendorid;      /* default subvendorid */
    u_int16_t subdeviceid;      /* default subdeviceid */
    u_int32_t clock_freq;       /* core clock frequency */
    u_int8_t long_lived;        /* aux powered swm card */
    u_int8_t first_bus;         /* first bus number for virtual devices bdf */
    pciemgr_initmode_t initmode;/* how to initialize hw,mem/shmem */
    u_int32_t fake_bios_scan:1; /* scan finalized topology, assign bus #'s */
    u_int32_t noexttag:1;       /* no extended tags capable */
    u_int32_t noexttag_en:1;    /* no extended tags enabled */
    u_int32_t nomsixcap:1;      /* no msix cap */
    u_int32_t sris:1;           /* enable spread spectrum clk */
    u_int32_t compliance:1;     /* compliance test mode */
    u_int32_t strict_crs:1;     /* enable crs=1 at open */
    u_int32_t restart:1;        /* restarting */
    u_int32_t single_pnd:1;     /* single pending mode */
    char id       [PCIEMGR_STRSZ]; /* product id */
    char partnum  [PCIEMGR_STRSZ]; /* part number */
    char serialnum[PCIEMGR_STRSZ]; /* serial number */
    char mfgdate  [PCIEMGR_STRSZ]; /* manufacturing date code */
    char engdate  [PCIEMGR_STRSZ]; /* engineering date code */
    char pcarev   [PCIEMGR_STRSZ]; /* pca revision */
    char misc     [PCIEMGR_STRSZ]; /* frequency,power, etc */
    char mac      [PCIEMGR_STRSZ]; /* factory mac address */
    char fwvers   [PCIEMGR_STRSZ]; /* fw version */
} pciemgr_params_t;

#include "pciehw.h"
#include "pciehw_dev.h"
#include "pciehsvc.h"

#endif /* __PCIEMGR_H__ */
