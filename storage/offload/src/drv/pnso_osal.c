/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include "pnso_global.h"
#include "pnso_logger.h"
#include "pnso_osal_ops.h"

/* run on dol/model or linux user space */
#if PNSO_API_ON_MODEL
const struct osal_ops *g_osal_ops = &dol_osal_ops; 	
#else
const struct osal_ops *g_osal_ops = &lus_osal_ops; 	
#endif

void *
pnso_malloc(size_t size)
{
	return g_osal_ops->malloc(size);
}

void
pnso_free(void *ptr)
{
	return g_osal_ops->free(ptr);
}

pnso_error_t
pnso_memalign(size_t alignment, size_t size, void **ptr)
{
	return g_osal_ops->alloc_memalign(alignment, size, ptr);
}

uint64_t
pnso_virt_to_phys(void *ptr)
{
	return g_osal_ops->virt_to_phys(ptr);
}

