/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include "osal_assert.h"
#include "osal_rmem.h"
#include "osal_logger.h"
#include "sonic_api_int.h"

/*
 * TODO:
 * 1. Kernel doesnt provide an elegant way to allocate aligned memory. This may need to be moved to upper layer 
 *    or serviced via memory manager within osal. 
 * 2. kmalloc has a limitation that allocated size should be less than KMALLOC_MAXSIZE. This again may need to 
 *    be moved to upper layer or serviced via memory manager within osal . 
 */

/* TODO - rmem functions need to be filled */
uint64_t osal_rmem_alloc(size_t size) 
{
	return sonic_rmem_alloc(size);
}

uint64_t osal_rmem_calloc(size_t size)
{
	return sonic_rmem_calloc(size);
}

uint64_t osal_rmem_aligned_alloc(size_t alignment, size_t size)
{
	if(!is_power_of_2(alignment) || (alignment > sonic_rmem_page_size_get())) 
	{
		OSAL_LOG_ERROR("rmem alloc request failed - invalid alignment");
		return 0;
	}
	
	return sonic_rmem_alloc(size);
}

uint64_t osal_rmem_aligned_calloc(size_t alignment, size_t size)
{
	if(!is_power_of_2(alignment) || (alignment > sonic_rmem_page_size_get())) 
	{
		OSAL_LOG_ERROR("rmem calloc request failed - size or alignment not multiple of page size");
		return 0;
	}
	
	return sonic_rmem_calloc(size);
}

void osal_rmem_free(uint64_t ptr, size_t size)
{
	return sonic_rmem_free(ptr, size);
}

void osal_rmem_set(uint64_t ptr, uint8_t val, size_t size)
{
	sonic_rmem_set(ptr, val, size);
}

void osal_rmem_read(void *dst, uint64_t ptr, size_t size)
{
	sonic_rmem_read(dst, ptr, size);
}

void osal_rmem_write(uint64_t ptr, const void *src, size_t size)
{
	sonic_rmem_write(ptr, src, size);
}

uint32_t osal_rmem_total_pages_get(void)
{
	return sonic_rmem_total_pages_get();
}

uint32_t osal_rmem_avail_pages_get(void)
{
	return sonic_rmem_avail_pages_get();
}

uint32_t osal_rmem_page_size_get(void)
{
	return sonic_rmem_page_size_get();
}

uint64_t osal_rmem_addr_invalid_def(void)
{
	return sonic_rmem_addr_invalid_def();
}

bool osal_rmem_addr_valid(uint64_t addr)
{
	return sonic_rmem_addr_valid(addr);
}

