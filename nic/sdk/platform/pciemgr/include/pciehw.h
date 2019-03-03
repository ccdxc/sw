/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIEHW_H__
#define __PCIEHW_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

#include <stdint.h>

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"

#include "pmt.h"
#include "prt.h"
#include "pciemgr_stats.h"

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct cfgspace_s;
typedef struct cfgspace_s cfgspace_t;
struct pciemgr_params_s;
typedef struct pciemgr_params_s pciemgr_params_t;

#define PCIEHW_NPORTS   8
#define PCIEHW_NDEVS    48
#define PCIEHW_CFGSZ    1024
#define PCIEHW_NROMSK   128
#define PCIEHW_ROMSKSZ  (PCIEHW_CFGSZ / sizeof (u_int32_t))
#define PCIEHW_CFGHNDSZ (PCIEHW_CFGSZ / sizeof (u_int32_t))

#define PCIEHW_NPMT     PMT_COUNT
#define PCIEHW_NPRT     PRT_COUNT
#define PCIEHW_NBAR     6               /* 6 cfgspace BARs */

enum pciehw_cfghnd_e {
    PCIEHW_CFGHND_NONE,
    PCIEHW_CFGHND_CMD,
    PCIEHW_CFGHND_BARS,
    PCIEHW_CFGHND_ROMBAR,
    PCIEHW_CFGHND_MSIX,
};
typedef enum pciehw_cfghnd_e pciehw_cfghnd_t;

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
    u_int32_t pmtb;                     /* pmt base  for bar */
    u_int32_t pmtc;                     /* pmt count for bar */
} pciehwbar_t;

typedef struct pciehwdev_s {
    char name[32];                      /* device name */
    int port;                           /* pcie port */
    void *pdev;                         /* pciehdev */
    u_int16_t bdf;                      /* bdf of this dev */
    u_int32_t lifb;                     /* lif base  for this dev */
    u_int32_t lifc;                     /* lif count for this dev */
    u_int32_t intrb;                    /* intr resource base */
    u_int32_t intrc;                    /* intr resource count */
    pciehwdevh_t parenth;               /* handle to parent */
    pciehwdevh_t childh;                /* handle to child */
    pciehwdevh_t peerh;                 /* handle to peer */
    u_int8_t intpin;                    /* legacy int pin */
    u_int8_t romsksel[PCIEHW_ROMSKSZ];  /* cfg read-only mask selectors */
    u_int8_t cfgpmtf[PCIEHW_CFGHNDSZ];  /* cfg pmt flags */
    u_int8_t cfghnd[PCIEHW_CFGHNDSZ];   /* cfg indirect/notify handlers */
    pciehwbar_t bar[PCIEHW_NBAR];       /* bar info */
    pciehwbar_t rombar;                 /* option rom bar */
} pciehwdev_t;

typedef struct pciehw_port_s {
    u_int8_t secbus;                    /* bridge secondary bus */
    pciemgr_stats_t stats;
} pciehw_port_t;

typedef struct pciehw_sprt_s {
    prt_t prt;                          /* shadow copy of prt */
} pciehw_sprt_t;

typedef struct pciehw_spmt_s {
    u_int64_t baroff;                   /* bar addr offset */
    u_int64_t swrd;                     /* reads  handled by sw (not/ind) */
    u_int64_t swwr;                     /* writes handled by sw (not/ind) */
    pciehwdevh_t owner;                 /* current owner of this entry */
    u_int8_t loaded:1;                  /* is loaded into hw */
    u_int8_t cfgidx;                    /* cfgidx for bar we belong to */
    pmt_t pmt;                          /* shadow copy of pmt */
} pciehw_spmt_t;

typedef struct pciehw_sromsk_s {
    u_int32_t entry;
    u_int32_t count;
} pciehw_sromsk_t;

#define PCIEHW_MAGIC    0x706d656d      /* 'pmem' */
#define PCIEHW_VERSION  0x1

typedef struct pciehw_shmem_s {
    u_int32_t magic;                    /* PCIEHW_MAGIC when initialized */
    u_int32_t version;                  /* PCIEHW_VERSION when initialized */
    u_int32_t hwinit:1;                 /* hw is initialized */
    u_int32_t notify_verbose:1;         /* notify logs all */
    u_int32_t skip_notify:1;            /* notify skips if ring full */
    u_int32_t allocdev;
    u_int32_t allocpmt;
    u_int32_t allocprt;
    pciehwdevh_t rooth[PCIEHW_NPORTS];
    pciehwdev_t dev[PCIEHW_NDEVS];
    pciehw_port_t port[PCIEHW_NPORTS];
    pciehw_sromsk_t sromsk[PCIEHW_NROMSK];
    pciehw_spmt_t spmt[PCIEHW_NPMT];
    pciehw_sprt_t sprt[PCIEHW_NPRT];
    u_int8_t cfgrst[PCIEHW_NDEVS][PCIEHW_CFGSZ];
    u_int8_t cfgmsk[PCIEHW_NDEVS][PCIEHW_CFGSZ];
    u_int32_t notify_ring_mask;
} pciehw_shmem_t;

int pciehw_open(pciemgr_params_t *params);
void pciehw_close(void);
pciehw_shmem_t *pciehw_get_shmem(void);
void pciehw_initialize_topology(const u_int8_t port);
void pciehw_finalize_topology(pciehdev_t *proot);
void pciehw_dev_show(int argc, char *argv[]);
void pciehw_pmt_show(int argc, char *argv[]);
void pciehw_bar_show(void);

/* flags for stats_show() */
#define PMGRSF_NONE     0x0
#define PMGRSF_ALL      0x1
void pciehw_stats_show(const int port, const unsigned int flags);
void pciehw_stats_clear(const int port, const unsigned int flags);
pciemgr_stats_t *pciehw_stats_get(const int port);

void pciehw_event_hostup(const int port, const int gen, const int width);
void pciehw_event_hostdn(const int port);
void pciehw_event_buschg(const int port, const u_int8_t secbus);

int pciehw_notify_poll_init(void);
int pciehw_notify_poll(void);
int pciehw_notify_intr_init(const int port, u_int64_t msgaddr, u_int32_t msgdata);
int pciehw_notify_intr(const int port);

int pciehw_indirect_poll_init(void);
int pciehw_indirect_poll(void);
int pciehw_indirect_intr_init(const int port, u_int64_t msgaddr, u_int32_t msgdata);
int pciehw_indirect_intr(const int port);

#define AXIMSTF_TLP     0x1
#define AXIMSTF_IND     0x2
#define AXIMSTF_RAW     0x4

void pciehw_aximst_show(const unsigned int port,
                        const unsigned int entry,
                        const int flags);

void pciehw_dbg(int argc, char *argv[]);

int pciehw_cfgrd(const u_int8_t port, const u_int16_t bdf,
                 const u_int16_t offset, const u_int8_t size, u_int32_t *valp);
int pciehw_cfgwr(const u_int8_t port, const u_int16_t bdf,
                 const u_int16_t offset, const u_int8_t size, u_int32_t val);

int pciehw_memrd(const u_int8_t port,
                 const u_int64_t addr, const u_int8_t size, u_int64_t *valp);
int pciehw_memwr(const u_int8_t port,
                 const u_int64_t addr, const u_int8_t size, u_int64_t val);

int pciehw_iord(const u_int8_t port,
                const u_int32_t addr, const u_int8_t size, u_int32_t *valp);
int pciehw_iowr(const u_int8_t port,
                const u_int32_t addr, const u_int8_t size, u_int32_t val);

unsigned long long pciehw_barsz(const u_int8_t port,
                                const u_int16_t bdf, const int i);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHW_H__ */
