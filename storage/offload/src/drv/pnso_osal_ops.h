/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_OSAL_OPS_H__
#define __PNSO_OSAL_OPS_H__

#include "pnso_global.h"

struct osal_ops {
	void * (*malloc)(size_t size);

	void (*free)(void *ptr);

	pnso_error_t (*alloc_memalign)(size_t alignment, size_t size,
			void **ptr);

	uint64_t (*virt_to_phys)(void *ptr);
};

extern const struct osal_ops dol_osal_ops;
extern const struct osal_ops lus_osal_ops;

#endif /* __PNSO_OSAL_OPS_H__ */
