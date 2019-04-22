/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#ifndef __PCIEHDEVICES_H__
#define __PCIEHDEVICES_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;

typedef struct debugdev_res_s {
    u_int16_t vendorid;         /* override vendorid */
    u_int16_t deviceid;         /* override deviceid */
    u_int32_t classcode;        /* override classcode */
    struct {
        u_int64_t barpa;        /* bar physical address */
        u_int64_t barsz;        /* bar size */
        u_int32_t prefetch;     /* bar prefetch enable */
    } bar[3];                   /* debug device bar config */
} debugdev_res_t;

/*
 * PCIe resources for device (pf or vf)
 */
typedef struct pciehdev_res_s {
    u_int8_t port;              /* pcie port id */
    u_int16_t vendorid;         /* default vendorid */
    u_int16_t subvendorid;      /* default subvendorid */
    u_int16_t subdeviceid;      /* default subdeviceid */
    u_int32_t fnn:1;
    u_int8_t is_vf:1;           /* these resources are for vfs (in vfres) */
    u_int16_t totalvfs;         /* PF: total number of sriov vfs */
    u_int32_t lifb;             /* lif id base */
    u_int32_t lifc;             /* lif id count */
    u_int32_t intrb;            /* interrupt base */
    u_int32_t intrc;            /* interrupt count */
    u_int32_t intrdmask:1;      /* reset val for drvcfg.mask */
    u_int32_t npids;            /* number of rdma pids */
    u_int64_t devcmdpa;         /* devcmd region physical address */
    u_int32_t devcmd_stride;    /* VF: stride between devcmdpa regions */
    u_int64_t devcmddbpa;       /* devcmd doorbell physical address */
    u_int32_t devcmddb_stride;  /* VF: stride between devcmddbpa regions */
    u_int64_t cmbpa;            /* controller mem buf physical address */
    u_int32_t cmbsz;            /* controller mem buf bar size */
    u_int32_t cmbprefetch:1;    /* controller mem buf prefetch enable */
    u_int32_t cmb_stride;       /* VF: stride between cmdpa regions */
    u_int32_t romsz;            /* PF: option rom region size */
    u_int64_t rompa;            /* PF: option rom mem buf physical address */
    u_int64_t nvmeregspa;       /* nvme 4k register region */
    u_int32_t nvmeqidc;         /* nvme qid count */
    u_int64_t dsn;              /* device serial number */
    u_int32_t dsn_stride;       /* VF: stride between dsn values */
    union {                     /* aux per-device-type resources */
        debugdev_res_t debug;   /* debug device resources */
    };
} pciehdev_res_t;

typedef struct pciehdevice_resources_s {
    pciehdev_res_t pfres;       /* pf/endpoint resources */
    pciehdev_res_t vfres;       /* if pfres.nvfs > 0, vfres valid */
} pciehdevice_resources_t;

pciehdev_t *pciehdevice_new(const char *type,
                            const char *name,
                            const pciehdevice_resources_t *pres);

void pciehdevice_delete(pciehdev_t *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHDEVICES_H__ */
