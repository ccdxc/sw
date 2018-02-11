/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHW_IMPL_H__
#define __PCIEHW_IMPL_H__

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "pmt.h"
#include "notify.h"
#include "event.h"

struct pciehw_s;
typedef struct pciehw_s pciehw_t;

#define PCIEHW_NPORTS   8
#define PCIEHW_NBUS     16
#define PCIEHW_NDEVS    16
#define PCIEHW_CFGSZ    1024

#define PCIEHW_NROMSK   128
#define PCIEHW_ROMSKSZ  (PCIEHW_CFGSZ / sizeof (u_int32_t))

#define PCIEHW_CFGHNDSZ (PCIEHW_CFGSZ / sizeof (u_int32_t))

enum pciehw_cfghnd_e {
    PCIEHW_CFGHND_NONE,
    PCIEHW_CFGHND_CMD,
    PCIEHW_CFGHND_BARS,
    PCIEHW_CFGHND_MSIX,
};
typedef enum pciehw_cfghnd_e pciehw_cfghnd_t;

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
    u_int32_t valid:1;                  /* valid bar for this dev */
    pciehwbartype_t type;               /* PCIEHWBARTYPE_* */
    u_int32_t pmti;                     /* pmt entry for bar */
} pciehwbar_t;

typedef struct pciehwdev_s {
    char name[32];                      /* device name */
    int port;                           /* pcie port */
    void *pdev;                         /* pciehdev */
    u_int16_t bdf;                      /* bdf of this dev */
    u_int16_t lif_valid:1;              /* lif is valid */
    u_int32_t lif;                      /* lif for this dev */
    u_int32_t intrb;                    /* intr resource base */
    u_int32_t intrc;                    /* intr resource count */
    pciehwdevh_t parenth;               /* handle to parent */
    pciehwdevh_t childh;                /* handle to child */
    pciehwdevh_t peerh;                 /* handle to peer */
    u_int8_t intpin;                    /* legacy int pin */
    u_int8_t vfstridesel;               /* vfstride selector */
    u_int8_t romsksel[PCIEHW_ROMSKSZ];  /* cfg read-only mask selectors */
    u_int8_t cfghnd[PCIEHW_CFGHNDSZ];   /* cfg indirect/notify handlers */
    pciehwbar_t bar[PCIEHW_NBAR];
} pciehwdev_t;

typedef struct pciehw_port_s {
    u_int32_t notify_max;               /* largest pending notify events */
} pciehw_port_t;

typedef struct pciehw_sprt_s {
    pciehwdevh_t owner;
    u_int8_t type;                      /* PRT_TYPE_* */
    u_int32_t notify:1;                 /* notify access */
    u_int32_t indirect:1;               /* indirect access */
    u_int32_t ressize;                  /* size of mapping */
    u_int64_t resaddr;                  /* mapped resource address */
    u_int64_t updvec;                   /* per-qtype UPD vector */
} pciehw_sprt_t;

typedef struct pciehw_spmt_s {
    pciehwdevh_t owner;                 /* current owner of this entry */
    u_int8_t type;                      /* PMT_TYPE_* */
    u_int32_t loaded:1;                 /* loaded into hw */
    u_int32_t notify:1;                 /* notify access */
    u_int32_t indirect:1;               /* indirect access */
    u_int64_t baraddr;                  /* bar address */
    u_int32_t barsize;                  /* bar size */
    u_int32_t prtbase;                  /* start of contiguous prt entries */
    u_int32_t prtcount;                 /* count of contiguous prt entries */
    u_int32_t prtsize;                  /* size of each prt */
    u_int8_t qtypestart;                /* prt db: qtype low bit */
    u_int8_t qtypemask;                 /* prt db: qtype mask */
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
    u_int32_t notify_ring_mask;
} pciehw_mem_t;

typedef struct pciehw_s {
    u_int32_t open:1;                   /* hw is in use */
    u_int32_t is_asic:1;                /* running on asic platform */
    u_int16_t clients;                  /* number of clients using us */
    u_int32_t nports;                   /* number of ports available */
    pciehw_params_t hwparams;
    pciehw_mem_t *pciehwmem;
} pciehw_t;

struct pcie_stlp_s;
typedef struct pcie_stlp_s pcie_stlp_t;

pciehw_t *pciehw_get(void);
pciehw_mem_t *pciehw_get_hwmem(pciehw_t *phw);
pciehwdev_t *pciehwdev_get(pciehwdevh_t hwdevh);
pciehwdevh_t pciehwdev_geth(const pciehwdev_t *phwdev);
void pciehwdev_get_cfgspace(const pciehwdev_t *phwdev, cfgspace_t *cs);
char *pciehwdev_get_name(const pciehwdev_t *phwdev);
pciehwdev_t *pciehwdev_find_by_name(const char *name);

#include "hdrt.h"
#include "portmap.h"
#include "intr.h"
#include "reset.h"

int pciehw_cfg_init(pciehw_t *phw);
int pciehw_cfg_finalize(pciehdev_t *pdev);
void pciehw_cfgrd_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const pciehw_spmt_t *spmt);
void pciehw_cfgwr_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const pciehw_spmt_t *spmt);

int pciehw_bar_init(pciehw_t *phw);
int pciehw_bar_finalize(pciehdev_t *pdev);
void pciehw_bar_setaddr(pciehwbar_t *phwbar, const u_int64_t addr);
void pciehw_bar_load(pciehwbar_t *phwbar);
void pciehw_bar_enable(pciehwbar_t *phwbar, const int on);
int pciehw_bar_getsz(pciehwbar_t *phwbar);
void pciehw_barrd_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const pciehw_spmt_t *spmt);
void pciehw_barwr_notify(pciehwdev_t *phwdev,
                         const pcie_stlp_t *stlp,
                         const pciehw_spmt_t *spmt);
void pciehw_bar_dbg(int argc, char *argv[]);

void pciehw_prt_init(pciehw_t *phw);
int pciehw_prt_alloc(pciehwdev_t *phwdev, const pciehbar_t *bar);
void pciehw_prt_free(const int prtbase, const int prtcount);
int pciehw_prt_load(const int prtbase, const int prtcount);
void pciehw_prt_unload(const int prtbase, const int prtcount);
void pciehw_prt_dbg(int argc, char *argv[]);

#define ROMSK_RDONLY 1

void pciehw_romsk_init(pciehw_t *phw);
int pciehw_romsk_load(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_romsk_unload(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_romsk_dbg(int argc, char *argv[]);

void pciehw_vfstride_init(pciehw_t *phw);
int pciehw_vfstride_load(pciehw_t *phw, pciehwdev_t *phwdev);
void pciehw_vfstride_unload(pciehw_t *phw, pciehwdev_t *phwdev);

void pciehw_port_init(pciehw_t *phw);
void pciehw_port_skip_notify(const int port, const int on);

void *pciehw_memset(void *s, int c, size_t n);
void *pciehw_memcpy(void *dst, const void *src, size_t n);

#endif /* __PCIEHW_IMPL_H__ */
