/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_OSAL_H__
#define __PNSO_OSAL_H__

#include "pnso_types.h"

void *pnso_malloc(size_t size);

void pnso_free(void *ptr);

pnso_error_t pnso_memalign(size_t alignment, size_t size, void **memptr);

uint64_t pnso_virt_to_phys(void *ptr);

#endif /* __PNSO_OSAL_H__ */
