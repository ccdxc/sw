#ifndef _PAL_MEM_H_
#define _PAL_MEM_H_
#include <inttypes.h>

/*
 * PAL memory read/write APIs.
 */
void *pal_mem_map(const u_int64_t pa, const size_t sz, u_int32_t flags);
void pal_mem_unmap(void *va);
void *pal_mem_map_region(char *region_name);
void pal_mem_unmap_region(char *region_name);
u_int64_t pal_mem_region_pa(char *region_name);

u_int64_t pal_mem_vtop(const void *va);
void *pal_mem_ptov(const u_int64_t pa);
int pal_mem_rd(const u_int64_t pa,       void *buf, const size_t sz, u_int32_t flags);
int pal_mem_wr(const u_int64_t pa, const void *buf, const size_t sz, u_int32_t flags);
int pal_memset(const u_int64_t pa, u_int8_t c, const size_t sz, u_int32_t flags);

/* Move memory from source to destination */
u_int32_t pal_mem_move(uint64_t source_address,
                       uint64_t dest_address,
                       u_int32_t size);

#endif

