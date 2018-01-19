/*
 * Copyright (c) 2017, Pensando Systems Inc.
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
    u_int32_t fnn:1;
    u_int32_t lif_valid:1;      /* lif id is valid */
    u_int32_t lif;              /* lif id */
    u_int8_t port;              /* pcie port id */
    u_int32_t intrbase;         /* interrupt start */
    u_int32_t nintrs;           /* interrupt count */
} pciehdevice_resources_t;

pciehdev_t *pciehdev_enet_new(const char *name,
                              const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_nvme_new(const char *name,
                              const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_accel_new(const char *name,
                               const pciehdevice_resources_t *pres);

pciehdev_t *pciehdev_debug_new(const char *name,
                               const pciehdevice_resources_t *pres);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHDEVICES_H__ */
