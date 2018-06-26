#include <linux/slab.h>
#include "osal_mem.h"

void* osal_alloc(size_t size) 
{
	return kmalloc(size, GFP_KERNEL|GFP_NOFS|GFP_NOIO);
}

void* osal_aligned_alloc(size_t alignment, size_t size) 
{
	return kmalloc(size, GFP_KERNEL|GFP_NOFS|GFP_NOIO);
}

void osal_free(void* ptr) 
{
	return kfree(ptr);
}

void* osal_rmem_alloc(size_t size) 
{
	return kmalloc(size, GFP_KERNEL|GFP_NOFS|GFP_NOIO);
}

void* osal_rmem_aligned_alloc(size_t alignment, size_t size) 
{
	return kmalloc(size, GFP_KERNEL|GFP_NOFS|GFP_NOIO);
}

void osal_rmem_free(void* ptr) 
{
	return kfree(ptr);
}

