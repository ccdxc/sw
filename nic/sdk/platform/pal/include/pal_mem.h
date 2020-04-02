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

/*
 * Physical memory ranges.
 *
 * These functions describe the PAL dataplane memory ranges.
 * e.g an 8GB HBM Capri with 2GB for Linux and 6GB for dataplane may
 * have an overall memory map as follows:
 *      240000000...27fffffff: 1GB Linux high memory
 *      0c4000000...23fffffff: ~6GB Dataplane noncoherent memory
 *      0c0000000...0c3ffffff: 64MB Dataplane coherent memory
 *      080000000...0bfffffff: 1GB Linux low memory
 *
 * These APIs report the dataplane memory only, distilling the following into:
 *      pal_mem_get_phys_totalsize() = 0x180000000 (6GB)
 *      pal_mem_get_phys_ranges() =
 *          nranges = 2
 *          range[0]: pa=0x0c000000, sz=0x004000000, flags=COHERENT
 *          range[1]: pa=0x0c400000, sz=0x17c000000, flags=0
 *
 * NOTE: The range[] array:
 *      - Will be sorted on pa in ascending order
 *      - Will be packed (i.e. contiguous entries will have different flags)
 *      - May be discontiguous (i.e. have holes)
 */
typedef struct pal_mem_phys_range_s {
    uint64_t pa;
    uint64_t sz;
    uint32_t flags;
} pal_mem_phys_range_t;
#define PAL_MEM_PHYS_COHERENT   0x1

typedef struct {
    int nranges;
    const pal_mem_phys_range_t *range;
} pal_mem_phys_rangetab_t;

int pal_mem_get_phys_ranges(pal_mem_phys_rangetab_t *tab);
uint64_t pal_mem_get_phys_totalsize(void);

#endif

