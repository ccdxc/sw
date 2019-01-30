#ifndef _PAL_MEM_H_
#define _PAL_MEM_H_
#include <inttypes.h>

/*
 * Memory write fence to ensure all pending writes are completed.
 */
#ifdef __aarch64__
#define PAL_barrier()  asm volatile("dsb sy" ::: "memory")
#else
#define PAL_barrier()  do { } while (0)
#endif

/*
 * PAL memory read/write APIs.
 */
void *pal_mem_map(const uint64_t pa, const size_t sz, uint32_t flags);
void pal_mem_unmap(void *va);
void *pal_mem_map_region(char *region_name);
void pal_mem_unmap_region(char *region_name);
uint64_t pal_mem_region_pa(char *region_name);

uint64_t pal_mem_vtop(const void *va);
void *pal_mem_ptov(const uint64_t pa);
int pal_mem_rd(const uint64_t pa,       void *buf, const size_t sz, uint32_t flags);
int pal_mem_wr(const uint64_t pa, const void *buf, const size_t sz, uint32_t flags);
int pal_memset(const uint64_t pa, uint8_t c, const size_t sz, uint32_t flags);

/* Move memory from source to destination */
uint32_t pal_mem_move(uint64_t source_address,
                       uint64_t dest_address,
                       uint32_t size);

#endif

