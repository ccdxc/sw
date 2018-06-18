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

uint64_t osal_virt_to_phy(void* ptr) 
{
  return (uint64_t)ptr;
}

void* osal_phy_to_virt(uint64_t phy) 
{
  return (void *)phy;
}

void* osal_rmem_alloc(size_t size) 
{
	return malloc(size);
}

void* osal_rmem_aligned_alloc(size_t alignment, size_t size) 
{
	return aligned_alloc(alignment, size);
}

void osal_rmem_free(void* ptr) 
{
	return free(ptr);
}

