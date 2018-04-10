/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdlib.h>

#include "pnso_global.h"
#include "pnso_logger.h"

#include "pnso_osal_ops.h"

static void *
__lus_malloc(size_t size)
{
	void *ptr;

	ptr = malloc(size);
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
__lus_free(void *ptr)
{
	PNSO_LOG_DEBUG(PNSO_OK, "deallocated %p", ptr);
	free(ptr);
}

static pnso_error_t
__lus_alloc_memalign(size_t alignment, size_t size, void **memptr)
{
	int err = PNSO_OK;
	void *ptr;

	err = posix_memalign(&ptr, alignment, size);
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
__lus_virt_to_phys(void *ptr)
{
	/* Note: This request should be a no-op in userspace */
	return (uint64_t) ptr;
}

const struct osal_ops lus_osal_ops = {
	.malloc = __lus_malloc,
	.free = __lus_free,
	.alloc_memalign = __lus_alloc_memalign,
	.virt_to_phys = __lus_virt_to_phys,
};
