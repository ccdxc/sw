/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#ifndef __PCIEHW_DEV_H__
#define __PCIEHW_DEV_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct pciehcfg_s;
typedef struct pciehcfg_s pciehcfg_t;
struct pciehbars_s;
typedef struct pciehbars_s pciehbars_t;

typedef enum pciehdev_event_e {
    PCIEHDEV_EV_NONE,
    PCIEHDEV_EV_MEMRD_NOTIFY,
    PCIEHDEV_EV_MEMWR_NOTIFY,
    PCIEHDEV_EV_SRIOV_NUMVFS,
    PCIEHDEV_EV_RESET,
} pciehdev_event_t;

typedef struct pciehdev_memrw_notify_s {
    u_int64_t baraddr;          /* PCIe bar address */
    u_int64_t baroffset;        /* bar-local offset */
    u_int8_t cfgidx;            /* bar cfgidx */
    u_int32_t size;             /* i/o size */
    u_int64_t localpa;          /* local physical address */
    u_int64_t data;             /* data, if write */
} pciehdev_memrw_notify_t;

typedef struct pciehdev_sriov_numvfs_s {
    u_int16_t numvfs;           /* number of vfs enabled */
} pciehdev_sriov_numvfs_t;

typedef enum pciehdev_rsttype_e {
    PCIEHDEV_RSTTYPE_NONE,
    PCIEHDEV_RSTTYPE_BUS,       /* bus reset */
    PCIEHDEV_RSTTYPE_FLR,       /* function level reset */
    PCIEHDEV_RSTTYPE_VF,        /* vf reset from sriov ctrl vfe */
} pciehdev_rsttype_t;

typedef struct pciehdev_reset_s {
    pciehdev_rsttype_t rsttype; /* RSTTYPE_* */
    u_int32_t lifb;             /* lif base */
    u_int32_t lifc;             /* lif count */
} pciehdev_reset_t;

typedef struct pciehdev_eventdata_s {
    pciehdev_event_t evtype;    /* PCIEHDEV_EV_* */
    u_int8_t port;              /* PCIe port */
    u_int32_t lif;              /* lif if event for lifs */
    union {
        pciehdev_memrw_notify_t memrw_notify;   /* EV_MEMRD/WR_NOTIFY */
        pciehdev_sriov_numvfs_t sriov_numvfs;   /* EV_SRIOV_NUMVFS */
        pciehdev_reset_t reset;                 /* EV_RESET */
    };
} pciehdev_eventdata_t;

typedef void (*pciehdev_evhandler_t)(const pciehdev_eventdata_t *evdata);

int pciehdev_open(pciemgr_params_t *params);
void pciehdev_close(void);

pciehdev_t *pciehdev_bridgeup_new(void);
pciehdev_t *pciehdev_bridgedn_new(const int port, const int memtun_en);

int pciehdev_initialize(const int port);
int pciehdev_finalize(const int port);

pciehcfg_t *pciehdev_get_cfg(pciehdev_t *pdev);
pciehbars_t *pciehdev_get_bars(pciehdev_t *pdev);

int pciehdev_add(pciehdev_t *pdev);
int pciehdev_addfn(pciehdev_t *pdev, pciehdev_t *pfn, const int fnc);
int pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild);

pciehdev_t *pciehdev_get_root(const u_int8_t port);
pciehdev_t *pciehdev_get_parent(pciehdev_t *pdev);
pciehdev_t *pciehdev_get_peer(pciehdev_t *pdev);
pciehdev_t *pciehdev_get_child(pciehdev_t *pdev);

pciehdev_t *pciehdev_get_by_bdf(const u_int8_t port, const u_int16_t bdf);
pciehdev_t *pciehdev_get_by_name(const char *name);

void *pciehdev_get_hwdev(pciehdev_t *pdev);
void pciehdev_set_hwdev(pciehdev_t *pdev, void *phwdev);
u_int16_t pciehdev_get_bdf(pciehdev_t *pdev);

int pciehdev_register_event_handler(pciehdev_evhandler_t evhandler);
void pciehdev_event(const pciehdev_eventdata_t *evd);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHW_DEV_H__ */
