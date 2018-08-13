#include <linux/slab.h>
#include "sonic_api_int.h"
#include "osal_mem.h"
#include "osal_logger.h"

/*
 * TODO:
 * 1. Kernel doesnt provide an elegant way to allocate aligned memory. This may need to be moved to upper layer 
 *    or serviced via memory manager within osal. 
 * 2. kmalloc has a limitation that allocated size should be less than KMALLOC_MAXSIZE. This again may need to 
 *    be moved to upper layer or serviced via memory manager within osal . 
 */

void* osal_alloc(size_t size) 
{
	return kmalloc(size, GFP_KERNEL);
}

void* osal_aligned_alloc(size_t alignment, size_t size) 
{
	return kmalloc(size, GFP_KERNEL);
}

void osal_free(void* ptr) 
{
	return kfree(ptr);
}

/* TODO - rmem functions need to be filled */
uint64_t osal_rmem_alloc(size_t size) 
{
	if(size % PAGE_SIZE != 0)
	{
		OSAL_LOG_ERROR("rmem alloc request failed - size not multiple of page size");
		return 0;
	}
	
	return sonic_rmem_alloc(size);
}

uint64_t osal_rmem_aligned_alloc(size_t alignment, size_t size) 
{
	if(size % PAGE_SIZE != 0 || alignment % PAGE_SIZE != 0) 
	{
		OSAL_LOG_ERROR("rmem alloc request failed - size or alignment not multiple of page size");
		return 0;
	}
	
	return sonic_rmem_alloc(size);
}

void osal_rmem_free(uint64_t ptr, size_t size) 
{
	return sonic_rmem_free(ptr, size);
}

