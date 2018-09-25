#include <linux/kernel.h>
#include <linux/slab.h>
#include "sonic_api_int.h"
#include "osal_assert.h"
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

uint64_t osal_rmem_calloc(size_t size)
{
	if(size % PAGE_SIZE != 0)
	{
		OSAL_LOG_ERROR("rmem calloc request failed - size not multiple of page size");
		return 0;
	}
	
	return sonic_rmem_calloc(size);
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

uint64_t osal_rmem_aligned_calloc(size_t alignment, size_t size)
{
	if(size % PAGE_SIZE != 0 || alignment % PAGE_SIZE != 0) 
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

uint64_t osal_virt_to_phy(void *ptr)
{
	uint64_t pa;

	OSAL_ASSERT(ptr);

	pa = virt_to_phys(ptr);

	return osal_hostpa_to_devpa(pa);
}

void *osal_phy_to_virt(uint64_t phy)
{
	uint64_t pa;

	OSAL_ASSERT(phy);

	pa = osal_devpa_to_hostpa(phy);

	return phys_to_virt((phys_addr_t) pa);
}

uint64_t osal_hostpa_to_devpa(uint64_t hostpa)
{
	return sonic_hostpa_to_devpa(hostpa);
}

uint64_t osal_devpa_to_hostpa(uint64_t devpa)
{
	return sonic_devpa_to_hostpa(devpa);
}
