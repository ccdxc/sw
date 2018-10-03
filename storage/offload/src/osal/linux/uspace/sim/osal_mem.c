#include <stdlib.h>
#include "osal_mem.h"

void* osal_alloc(size_t size) 
{
	return malloc(size);
}

void* osal_aligned_alloc(size_t alignment, size_t size) 
{
	return aligned_alloc(alignment, size);
}

void osal_free(void* ptr) 
{
	return free(ptr);
}

void* osal_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

char *osal_strdup(const char *str)
{
	return strdup(str);
}

uint64_t osal_virt_to_phy(void* ptr) 
{
	return (uint64_t)ptr;
}

void* osal_phy_to_virt(uint64_t phy) 
{
	return (void *)phy;
}

uint64_t osal_rmem_alloc(size_t size) 
{
	return (uint64_t) malloc(size);
}

uint64_t osal_rmem_calloc(size_t size) 
{
	return (uint64_t) calloc(1, size);
}

uint64_t osal_rmem_aligned_alloc(size_t alignment, size_t size) 
{
	return (uint64_t) aligned_alloc(alignment, size);
}

uint64_t osal_rmem_aligned_calloc(size_t alignment, size_t size) 
{
	void *ptr = aligned_alloc(alignment, size);
	if (ptr)
		memset(ptr, 0, size);
	return (uint64_t)ptr;
}

void osal_rmem_set(uint64_t ptr, uint8_t val, size_t size)
{
	memset((void *)ptr, val, size);
}

void osal_rmem_read(void *dst, uint64_t ptr, size_t size)
{
	memcpy(dst, (void *)ptr, size);
}

void osal_rmem_write(uint64_t ptr, const void *src, size_t size)
{
	memcpy((void *)ptr, src, size);
}

void osal_rmem_free(uint64_t ptr, size_t size) 
{
	return free((void*)ptr);
}
