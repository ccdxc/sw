/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdlib.h>

#include "pnso_global.h"
#include "pnso_logger.h"

#include "pnso_osal_ops.h"

#include "../utils/host_mem/c_if.h"

static void *
__dol_malloc(size_t size)
{
	void *ptr;

	ptr = alloc_host_mem(size);
	if (!ptr) {
		PNSO_LOG_ERROR(errno,
			       "failed to allocate memory! size: %ju errno: %d",
			       size, errno);
		assert(0);
		goto done;
	}
	PNSO_LOG_DEBUG(PNSO_OK, "allocated %p", ptr);

done:
	return ptr;
}

static void
__dol_free(void *ptr)
{
	PNSO_LOG_DEBUG(PNSO_OK, "deallocated %p", ptr);
	free_host_mem(ptr);
}

static pnso_error_t
__dol_alloc_memalign(size_t alignment, size_t size, void **memptr)
{
	int err = PNSO_OK;
	void *ptr;

	ptr = alloc_page_aligned_host_mem(size);
	if (!ptr) {
		err = ENOMEM;
		PNSO_LOG_ERROR(errno,
			       "failed to allocate aligned memory! alignment: %ju, size: %ju err: %d",
			       alignment, size, err);
		assert(0);
		goto done;
	}
	*memptr = ptr;
	PNSO_LOG_DEBUG(PNSO_OK, "allocated %p", ptr);

done:
	return (pnso_error_t) err;
}

static uint64_t
__dol_virt_to_phys(void *ptr)
{
	return host_mem_v2p(ptr);
}

const struct osal_ops dol_osal_ops = {
	.malloc = __dol_malloc,
	.free = __dol_free,
	.alloc_memalign = __dol_alloc_memalign,
	.virt_to_phys = __dol_virt_to_phys,
};
