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

typedef struct pciehdevice_resources_s {
    u_int16_t vendorid;         /* default vendorid */
    u_int16_t subvendorid;      /* default subvendorid */
    u_int16_t subdeviceid;      /* default subdeviceid */
    u_int32_t fnn:1;
    u_int8_t port;              /* pcie port id */
    u_int32_t lifb;             /* lif id base */
    u_int32_t lifc;             /* lif id count */
    u_int32_t intrb;            /* interrupt base */
    u_int32_t intrc;            /* interrupt count */
    u_int32_t npids;            /* number of rdma pids */
    u_int64_t devcmdpa;         /* devcmd region physical address */
    u_int64_t devcmddbpa;       /* devcmd doorbell physical address */
    u_int32_t cmbsz;            /* controller memory buffer bar size */
    u_int64_t cmbpa;            /* controller memory buffer physical address */
    u_int32_t cmbprefetch:1;    /* controller memory buffer prefetch enable */
    u_int32_t romsz;            /* option rom region size */
    u_int64_t rompa;            /* option rom memory buffer physical address */
    u_int64_t nvmeregspa;       /* nvme 4k register region */
    u_int64_t dsn;              /* device serial number */
    struct {
        u_int64_t barpa;        /* bar physical address */
        u_int64_t barsz;        /* bar size */
        u_int32_t prefetch;     /* bar prefetch enable */
    } debugbar[3];              /* debug vnic bar config */
} pciehdevice_resources_t;

pciehdev_t *pciehdev_eth_new(const char *name,
                             const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_mgmteth_new(const char *name,
                                 const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_nvme_new(const char *name,
                              const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_accel_new(const char *name,
                               const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_virtio_new(const char *name,
                                const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_pciestress_new(const char *name,
                                    const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_rcdev_new(const char *name,
                               const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_debug_new(const char *name,
                               const pciehdevice_resources_t *pres);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHDEVICES_H__ */
