/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_RMEM_H
#define OSAL_RMEM_H

#include "osal_stdtypes.h"

#ifndef __KERNEL__
#include <string.h>
#else
#include <linux/string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint64_t osal_rmem_alloc(size_t size);
uint64_t osal_rmem_calloc(size_t size);
uint64_t osal_rmem_aligned_alloc(size_t alignment, size_t size);
uint64_t osal_rmem_aligned_calloc(size_t alignment, size_t size);
void osal_rmem_free(uint64_t ptr, size_t size);
void osal_rmem_set(uint64_t ptr, uint8_t val, size_t size);
void osal_rmem_read(void *dst, uint64_t ptr, size_t size);
void osal_rmem_write(uint64_t ptr, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

#endif
