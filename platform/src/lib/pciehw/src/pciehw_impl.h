/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHW_IMPL_H__
#define __PCIEHW_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "pmt.h"
#include "notify.h"

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

#define PCIEHW_NPORTS   8
#define PCIEHW_NBUS     16
#define PCIEHW_NDEVS    16
#define PCIEHW_CFGSZ    1024

#define PCIEHW_NROMSK   128
#define PCIEHW_ROMSKSZ  (PCIEHW_CFGSZ / sizeof (u_int32_t))

#define PCIEHW_NPMT     PMT_COUNT
#define PCIEHW_NPRT     CAP_PXB_CSR_DHS_TGT_PRT_ENTRIES

#define PCIEHW_NBAR     6               /* 6 cfgspace BARs */

#define PCIEHW_NOTIFYSZ NOTIFYSZ

typedef u_int32_t pciehwdevh_t;

typedef enum pciehwbartype_e {
    PCIEHWBARTYPE_NONE,                 /* invalid bar type */
    PCIEHWBARTYPE_MEM,                  /* 32-bit memory bar */
    PCIEHWBARTYPE_MEM64,                /* 64-bit memory bar */
    PCIEHWBARTYPE_IO,                   /* 32-bit I/O bar */
} pciehwbartype_t;

typedef struct pciehwbar_s {
    u_int32_t valid:1;
    pciehwbartype_t type;               /* bar type (mem, mem64, io) */
    u_int32_t pmti;
    u_int32_t prtbase;
    u_int32_t prtcount;
    u_int32_t prtsize;
} pciehwbar_t;

typedef struct pciehwdev_s {
    char name[32];                      /* device name */
    int port;                           /* pcie port */
    u_int16_t bdf;                      /* bdf of this dev */
    u_int16_t lif_valid:1;              /* lif is valid */
    u_int32_t lif;                      /* lif for this dev */
    u_int32_t intrb;                    /* intr resource base */
    u_int32_t intrc;                    /* intr resource count */
    pciehwdevh_t parenth;               /* handle to parent */
    pciehwdevh_t childh;                /* handle to child */
    pciehwdevh_t peerh;                 /* handle to peer */
    u_int8_t vfstridesel;               /* vfstride selector */
    u_int8_t romsksel[PCIEHW_ROMSKSZ];  /* read-only mask selectors */
    pciehwbar_t bar[PCIEHW_NBAR];
} pciehwdev_t;

typedef struct pciehw_port_s {
} pciehw_port_t;

typedef struct pciehw_sprt_s {
    pciehwdevh_t owner;
    u_int64_t resaddr;                  /* mapped resource address */
    u_int32_t ressize;                  /* size of mapping */
} pciehw_sprt_t;

typedef struct pciehw_spmt_s {
    pciehwdevh_t owner;
    u_int32_t loaded:1;
    u_int64_t baraddr;
    u_int32_t barsize;
    u_int8_t qtypestart;
    u_int8_t qtypemask;
} pciehw_spmt_t;

typedef struct pciehw_sromsk_s {
    u_int32_t entry;
    u_int32_t count;
} pciehw_sromsk_t;

#define PCIEHW_MAGIC    0x706d656d      /* 'pmem' */
#define PCIEHW_VERSION  0x1

typedef struct pciehw_mem_s {
    u_int32_t magic;                    /* PCIEHW_MAGIC when initialized */
    u_int32_t version;                  /* PCIEHW_VERSION when initialized */
    u_int32_t allocdev;
    u_int32_t allocprt;
    pciehwdevh_t rooth;
    pciehwdev_t dev[PCIEHW_NDEVS];
    pciehw_port_t port[PCIEHW_NPORTS];
    pciehw_sromsk_t sromsk[PCIEHW_NROMSK];
    pciehw_spmt_t spmt[PCIEHW_NPMT];
    pciehw_sprt_t sprt[PCIEHW_NPRT];
    u_int8_t cfgcur[PCIEHW_NDEVS][PCIEHW_CFGSZ] __attribute__((aligned(4096)));
    u_int8_t cfgrst[PCIEHW_NDEVS][PCIEHW_CFGSZ] __attribute__((aligned(4096)));
    u_int8_t cfgmsk[PCIEHW_NDEVS][PCIEHW_CFGSZ] __attribute__((aligned(4096)));
    u_int8_t notify_area[PCIEHW_NPORTS][PCIEHW_NOTIFYSZ]
                                     __attribute__((aligned(PCIEHW_NOTIFYSZ)));
    u_int32_t notify_intr_dest;         /* temporary notify intr dest */
} pciehw_mem_t;

typedef struct pciehw_s {
    u_int32_t open:1;                   /* hw is in use */
    u_int32_t is_asic:1;                /* running on asic platform */
    u_int16_t clients;                  /* number of clients using us */
    u_int32_t nports;                   /* number of ports available */
    pciehw_params_t hwparams;
    pciehw_mem_t *pciehwmem;
} pciehw_t;

pciehw_t *pciehw_get(void);
pciehw_mem_t *pciehw_get_hwmem(pciehw_t *phw);
pciehwdev_t *pciehwdev_get(pciehwdevh_t hwdevh);
pciehwdevh_t pciehwdev_geth(pciehwdev_t *phwdev);
void pciehwdev_get_cfgspace(pciehwdev_t *phwdev, cfgspace_t *cs);
char *pciehwdev_get_name(pciehwdev_t *phwdev);
pciehwdev_t *pciehwdev_find_by_name(const char *name);

#include "hdrt.h"
#include "portmap.h"
#include "intr.h"
#include "reset.h"

int pciehw_nports(void);

int pciehw_cfg_init(pciehw_t *phw);
int pciehw_cfg_finalize(pciehdev_t *pdev);

int pciehw_bar_init(pciehw_t *phw);
int pciehw_bar_finalize(pciehdev_t *pdev);
void pciehw_bar_dbg(int argc, char *argv[]);

void pciehw_prt_init(pciehw_t *phw);
int pciehw_prt_load(pciehwdev_t *phwdev, pciehbar_t *bar);
void pciehw_prt_unload(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_prt_dbg(int argc, char *argv[]);

#define ROMSK_RDONLY 1

void pciehw_romsk_init(pciehw_t *phw);
int pciehw_romsk_load(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_romsk_unload(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_romsk_dbg(int argc, char *argv[]);

void pciehw_vfstride_init(pciehw_t *phw);
int pciehw_vfstride_load(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_vfstride_unload(pciehw_t *phw, pciehwdev_t *phwdev);

void *pciehw_memset(void *s, int c, size_t n);
void *pciehw_memcpy(void *dst, const void *src, size_t n);

#endif /* __PCIEHW_IMPL_H__ */
