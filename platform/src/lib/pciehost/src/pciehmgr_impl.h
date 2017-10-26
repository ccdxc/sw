/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHMGR_IMPL_H__
#define __PCIEHMGR_IMPL_H__

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct pciehmgr_params_s;
typedef struct pciehmgr_params_s pciehmgr_params_t;

typedef struct pciehmgr_s {
    pciehmgr_params_t mgrparams;
} pciehmgr_t;

pciehmgr_params_t *pciehmgr_get_params(void);

#endif /* __PCIEHMGR_IMPL_H__ */
