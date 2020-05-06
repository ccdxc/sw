/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __PCIEHDEV_EVENT_H__
#define __PCIEHDEV_EVENT_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

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

int pciehdev_register_event_handler(pciehdev_evhandler_t evhandler);

/* events in to pciemgr */
void pciehw_event_hostup(const int port, const int gen, const int width,
                         const u_int16_t lnksta2);
void pciehw_event_hostdn(const int port);
void pciehw_event_buschg(const int port, const u_int8_t secbus);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHDEV_EVENT_H__ */
