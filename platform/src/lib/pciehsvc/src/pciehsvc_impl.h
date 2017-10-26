/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHSVC_IMPL_H__
#define __PCIEHSVC_IMPL_H__

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct pciehsvc_params_s;
typedef struct pciehsvc_params_s pciehsvc_params_t;

typedef struct pciehsvc_s {
    pciehsvc_params_t svcparams;
    pciehdev_t *proot;
} pciehsvc_t;

pciehsvc_params_t *pciehsvc_get_params(void);

#endif /* __PCIEHSVC_IMPL_H__ */
