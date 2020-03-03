/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_MEM_H
#define OSAL_MEM_H

#include "osal_stdtypes.h"

#ifndef __KERNEL__
#include <string.h>
#else
#include <linux/string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void *osal_alloc(size_t size);
void *osal_aligned_alloc(size_t alignment, size_t size);
void *osal_contig_alloc(size_t alignment, size_t size);
void osal_contig_free(void *ptr, size_t size);
void osal_free(void *ptr);
void *osal_realloc(void *ptr, size_t size);
char *osal_strdup(const char *str);
uint64_t osal_virt_to_phy(void *ptr);
void *osal_phy_to_virt(uint64_t phy);

#ifdef __cplusplus
}
#endif

#ifdef __FreeBSD__
#define virt_to_phys(x)	vtophys(x)
/* XXX: we shouldn't be doing this. */
#define phys_to_virt(x)	(void *)PHYS_TO_DMAP(x)
#endif

#endif
