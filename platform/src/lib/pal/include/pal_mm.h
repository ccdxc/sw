#ifndef _PAL_MM_H_
#define _PAL_MM_H_
#include <inttypes.h>

/* Allocs a region region_name of size size, we specify in flags,
 * if a moveable/non-moveable region is required - returns PA*/
u_int64_t pal_mem_alloc(char *region_name,
                        uint32_t size,
                        u_int32_t alloc_flags);

void pal_mem_free(char *region_name);

void *pal_mem_map_region(char *region_name);

void pal_mem_unmap_region(char *region_name);

u_int64_t pal_mem_region_pa(char *region_name);

/* Total space available for growth of moveable region */
u_int64_t pal_get_max_available(void);

void pal_print_application_regions(char *application_name);

void pal_print_metadata();

#endif

