/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <linux/kernel.h>
#include <linux/slab.h>
#include "osal_assert.h"
#include "osal_mem.h"

#ifdef __FreeBSD__
#include <machine/pmap.h>
#endif
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
	size_t block_count = (size + alignment - 1)/alignment;

	return kmalloc(block_count*alignment, GFP_KERNEL);
}

void *osal_contig_alloc(size_t alignment, size_t size)
{
	size_t block_count;

	alignment = alignment ? alignment : 1;
	block_count = (size + alignment - 1)/alignment;
#ifdef __FreeBSD__
	return contigmalloc(block_count*alignment, M_KMALLOC, GFP_KERNEL,
			0, ~0ull, alignment, 0);
#else
	return kmalloc(block_count*alignment, GFP_KERNEL);
#endif
}

void osal_contig_free(void *ptr, size_t size)
{
#ifdef __FreeBSD__
	if (ptr)
		contigfree(ptr, size, M_KMALLOC);
#else
	return kfree(ptr);
#endif
}

void osal_free(void* ptr) 
{
	return kfree(ptr);
}

void* osal_realloc(void *ptr, size_t size)
{
	return krealloc(ptr, size, GFP_KERNEL);
}

char *osal_strdup(const char *str)
{
	return kstrdup(str, GFP_KERNEL);
}

uint64_t osal_virt_to_phy(void *ptr)
{
	OSAL_ASSERT(ptr);
	return virt_to_phys(ptr);
}

void *osal_phy_to_virt(uint64_t phy)
{
	OSAL_ASSERT(phy);
	return phys_to_virt((phys_addr_t) phy);
}
