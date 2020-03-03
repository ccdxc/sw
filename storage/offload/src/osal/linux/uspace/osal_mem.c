/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
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

void *osal_contig_alloc(size_t alignment, size_t size)
{
	return aligned_alloc(alignment, size);
}

void osal_contig_free(void* ptr, size_t size)
{
	return free(ptr);
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
