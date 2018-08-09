#include <linux/slab.h>
#include "osal_mem.h"

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
void* osal_rmem_alloc(size_t size) 
{
	return NULL;
}

void* osal_rmem_aligned_alloc(size_t alignment, size_t size) 
{
	return NULL;
}

void osal_rmem_free(void* ptr) 
{
	return;
}

