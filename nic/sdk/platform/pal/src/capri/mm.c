/*
 * Copyright (c) 2018, Pensando Systems Inc.
 *
 * pal_mm.c : This file contains all the memory management functions
 *               for HBM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/mman.h>
#include "internal.h"
#include "mm_int.h"
#include "pal.h"

static void
print_region(pal_mem_region_t *region)
{
#if 0
    printf("\nREGION : %s\n", region->region_name);
    printf("ALLOCATING APPLICATION :%s\n", region->UUID);
    printf("SHAREABLE = %d\n", ISSHAREABLE(region));
    printf("START = 0x%lx\n", region->start_addr);
    printf("SIZE = %ld bytes\n", GETREGIONSIZE(region));
    printf("NEXT_IDX = %d\n", region->next_idx);
    printf("PREV_IDX = %d\n", region->prev_idx);
    printf("ALLOC = %d\n", ISALLOCATED(region));
    printf("OCCUPIED = %d\n", ISOCCUPIED(region));
    printf("CACHEABLE = %d\n", ISCACHEABLE(region));
    printf("COHERENT = %d\n", ISCOHERENT(region));
#endif
}

static void
print_metadata(pal_mem_metadata_t *metadata)
{
    int i = 0;

    printf("\n== METADATA - Start ==\n");
    printf("MAGIC = %.8s\n", metadata->magic);
    printf("Major = %x\n", metadata->major_ver);
    printf("Minor = %x\n", metadata->minor_ver);

    for (i = 0; i < MAXARENAS; i++) {
       if (metadata->arenas[i].size != 0) {
            printf("\nARENA[%d]\n", i);
            printf("START = 0x%lx\n", metadata->arenas[i].start);
            printf("SIZE = 0x%lx\n", metadata->arenas[i].size);
            printf("Top = 0x%lx\n", metadata->arenas[i].top);
            printf("Free = 0x%lx\n", metadata->arenas[i].free_space);
            printf("Flags = 0x%lx\n", metadata->arenas[i].flags);
            printf("Free IDX = %d\n", metadata->arenas[i].free_region_idx);
            printf("Alloc IDX = %d\n", metadata->arenas[i].allocated_region_idx);
        }
    }

    for (i = 0; i < MAXREGIONS; i++) {
        if (ISOCCUPIED(&metadata->regions[i])) {
            print_region(&metadata->regions[i]);
        }
    }

    printf("\n== METADATA - End ==\n\n\n");
}

/* copy_int : Internal function to copy data byte by byte.
 * This is added to avoid circular dependency between pal.h
 * and pal_mm.h. (We could have potentially used the memcpy
 * from pal.h)
 *
 * This is primarily used for metadata management.
 */
static void *
copy_int(void *dst, const void *src, size_t n)
{
    volatile u_int8_t *d = dst;
    const u_int8_t *s = src;
    int i;

    for (i = 0; i < n; i++) {
        *d++ = *s++;
    }
    return dst;
}

/* PAL_MM : Memory Alloc and Frees */
static u_int64_t
pal_get_aligned_size_int(u_int64_t size)
{
    if ((size & 0xFFF) != 0) {
        return ((size & ~0xFFF) + 4096);
    } else {
        return size;
    }
}

/* pal_clear_region_data :
 * Clears up the data within the region, so that it becomes
 * useable for later allocations by some other arena.
 *
 * It must be ensured that the indices to next/prev are 
 * initialized appropriately.
 */
static void
pal_clear_region_data(pal_mem_region_t *region)
{
    memset(region, 0, sizeof(pal_mem_region_t));
    region->next_idx = MAXREGIONS;
    region->prev_idx = MAXREGIONS;
    region->arena_idx = MAXREGIONS;
}

static void
pal_mem_split(pal_mem_metadata_t *metadata, u_int16_t region_idx, u_int64_t split_size)
{
    u_int16_t arena_idx = metadata->regions[region_idx].arena_idx;
    u_int16_t insert_idx = MAXREGIONS;
    u_int16_t i = 0;

    /* Search within the regions to see if any region is unoccupied.
     * This helps us determine if the region which will be created 
     * as a result of the split can be stored within the regions.
     */  
    for (i = 0; i < MAXREGIONS; i++) {
     
        if (!ISOCCUPIED(&metadata->regions[i])) {
            insert_idx = i;
            break;
        }
    }

    if (insert_idx == MAXREGIONS) {
        printf("Could not find an unoccupied region. Cannot split.\n");
        return;
    }

    pal_clear_region_data(&metadata->regions[insert_idx]);    

    metadata->regions[insert_idx].start_addr = metadata->regions[region_idx].start_addr
                                                 + split_size;
    SETREGIONSIZE(&metadata->regions[insert_idx],
                  GETREGIONSIZE(&metadata->regions[region_idx]) - split_size);
    SETREGIONSIZE(&metadata->regions[region_idx], split_size);

    metadata->regions[insert_idx].next_idx = metadata->arenas[arena_idx].free_region_idx;
    metadata->regions[insert_idx].prev_idx = MAXREGIONS;    
    metadata->regions[insert_idx].arena_idx = arena_idx;    

    metadata->regions[metadata->arenas[arena_idx].free_region_idx].prev_idx = insert_idx;
    metadata->arenas[arena_idx].free_region_idx = insert_idx;

}

static void
pal_merge_regions_int(pal_mem_metadata_t *metadata, u_int16_t idx1, u_int16_t idx2)
{
    assert(metadata->regions[idx1].arena_idx == metadata->regions[idx2].arena_idx);
    u_int64_t region2_size = GETREGIONSIZE(&(metadata->regions[idx2]));
    u_int64_t region1_size = GETREGIONSIZE(&(metadata->regions[idx1])) + region2_size;

    if (metadata->regions[idx2].next_idx == MAXREGIONS) {
        metadata->regions[metadata->regions[idx2].next_idx].prev_idx = metadata->regions[idx2].prev_idx;
    }

    if (metadata->regions[idx2].prev_idx != MAXREGIONS) {
        metadata->regions[metadata->regions[idx2].prev_idx].next_idx = metadata->regions[idx2].next_idx;
    }

    SETREGIONSIZE(&metadata->regions[idx1], region1_size);
    pal_clear_region_data(&(metadata->regions[idx2]));
}

static void
pal_coalesce_int(pal_mem_metadata_t *metadata, u_int16_t arena_idx, u_int16_t idx)
{
    u_int16_t idx2 = metadata->arenas[arena_idx].free_region_idx;

    for (; idx2 != MAXREGIONS; idx2 = metadata->regions[idx2].next_idx) {
        assert(idx2 != metadata->regions[idx2].next_idx);
        assert(!ISALLOCATED(&metadata->regions[idx2]));

        if ((metadata->regions[idx].start_addr + GETREGIONSIZE(&metadata->regions[idx]) ==
            (metadata->regions[idx2].start_addr))) {
            pal_merge_regions_int(metadata, idx, idx2);
            break;
        }
    }
}

static void
pal_unmap_metadata()
{
    void *va = pr_ptov(MEMSTART, sizeof(pal_mem_metadata_t), FREEACCESS);
    pr_mem_unmap(va);
}

/* TODO : Add a pal memory spec which has information about
 *        the start address and size of each
 *        coherent/non-coherent regions.
 */
void
pal_mm_init(char *application_name)
{
     pal_mem_metadata_t metadata;
     u_int64_t mem_start = MEMSTART;
     char buf[8];
     u_int16_t i;
     pal_data_t *pd = pal_get_data(); 

     strncpy(pd->app_uuid, application_name, MAXUUID);

     if (!pal_wr_lock(MEMLOCK)) {
         printf("Could not lock pal.lck\n");
         return;
     }

     /* Check if pal has already been init-ed */
     copy_int(buf, pr_ptov(mem_start, 8, FREEACCESS), 8);

     if (strncmp(buf, MAGIC, strlen(MAGIC)) == 0) {
        pd->init_done = 1;
        return;
     }

     memset(&metadata, 0, sizeof(metadata));

     strncpy(metadata.magic, MAGIC, strlen(MAGIC));
     metadata.major_ver = MAJORVERSION;
     metadata.minor_ver = MINORVERSION;

     /* Hardcode the memory spec for now */
     metadata.arenas[0].start = pal_get_aligned_size_int(mem_start + (sizeof(metadata)));
     metadata.arenas[0].size = 1024 * 1024 * 16; // 16MB
     metadata.arenas[0].free_space = metadata.arenas[0].size; // 16MB
     metadata.arenas[0].flags |= COHERENTFLAG;
     metadata.arenas[0].free_region_idx = MAXREGIONS;
     metadata.arenas[0].allocated_region_idx = MAXREGIONS;
     metadata.arenas[0].top = metadata.arenas[0].start;

     metadata.arenas[1].start = pal_get_aligned_size_int(metadata.arenas[0].start + metadata.arenas[0].size);
     metadata.arenas[1].size = 1024 * 1024 * 4;
     metadata.arenas[1].free_space = metadata.arenas[1].size;
     metadata.arenas[1].flags = 0;
     metadata.arenas[1].free_region_idx = MAXREGIONS;
     metadata.arenas[1].allocated_region_idx = MAXREGIONS;
     metadata.arenas[1].top = metadata.arenas[1].start;

     metadata.arenas[2].start = pal_get_aligned_size_int(metadata.arenas[1].start + metadata.arenas[1].size); 
     metadata.arenas[2].size = 1024 * 1024 * 4;
     metadata.arenas[2].free_space = metadata.arenas[1].size;
     metadata.arenas[2].flags |= COHERENTFLAG;
     metadata.arenas[2].flags |= ALLOC32BITFLAG;
     metadata.arenas[2].free_region_idx = MAXREGIONS;
     metadata.arenas[2].allocated_region_idx = MAXREGIONS;
     metadata.arenas[2].top = metadata.arenas[2].start;

     for (i = 0; i < MAXREGIONS; i++) {
        metadata.regions[i].next_idx = MAXREGIONS;
        metadata.regions[i].prev_idx = MAXREGIONS;
        metadata.regions[i].arena_idx = MAXREGIONS;
     }
     
     copy_int(pr_ptov(mem_start, sizeof(metadata), FREEACCESS),
         (const void*)&metadata, sizeof(metadata));
     pal_unmap_metadata();
     if (pal_wr_unlock(MEMLOCK) == LCK_FAIL) {
        printf("Failed to unlock.\n");
     }
     pd->init_done = 1;
}

static pal_mem_metadata_t*
pal_map_metadata_int()
{
     u_int64_t mem_start = MEMSTART;
     pal_mem_metadata_t *metadata = NULL;
     
     metadata = pr_ptov(mem_start, sizeof(pal_mem_metadata_t), FREEACCESS);
     return metadata;
}

/* TODO : Refactor the following "find" functions */
static u_int16_t
pal_mm_find_coherent_int(pal_mem_metadata_t *metadata,
                         uint64_t size,
                         u_int16_t *insert_idx)
{
    u_int16_t arena_idx = MAXARENAS;
    u_int16_t i = 0;
    u_int16_t j = 0;
    u_int64_t cumulative_free_region = 0;

    for (i = 0; i < MAXARENAS; i++) {
        if (((metadata->arenas[i].flags & COHERENTFLAG) != 0) &&
              metadata->arenas[i].free_space >= size) {

               cumulative_free_region = 0;
            *insert_idx = MAXREGIONS;
            /* walk the free list */
            for (j = metadata->arenas[i].free_region_idx;
                 j < MAXREGIONS;
                 j = metadata->regions[j].next_idx) {
                    assert(j != metadata->regions[j].next_idx);
                    if (GETREGIONSIZE(&metadata->regions[j]) >= size) {
                        *insert_idx = j;
                        break;
                    }
                    
                    cumulative_free_region += GETREGIONSIZE(&metadata->regions[j]);
                }

            /* After walking through all the regions in the free list
              * the total of free blocks contributing to the hole
              * was found to be smaller than the requested size, thus
              * allowing for a new allocation in the arena_idx = i.
              */         
            if (metadata->arenas[i].free_space - cumulative_free_region >= size) {
                    arena_idx = i;
                    break;
            }
        }
    }

    return arena_idx;
}

static u_int16_t
pal_mm_find_non_coherent_int(pal_mem_metadata_t *metadata,
                         uint64_t size,
                         u_int16_t *insert_idx)
{
    u_int16_t arena_idx = MAXARENAS;
    u_int16_t i = 0;
    u_int16_t j = 0;
    u_int64_t cumulative_free_region = 0;

    for (i = 0; i < MAXARENAS; i++) {
        if (((metadata->arenas[i].flags & COHERENTFLAG) == 0) &&
             metadata->arenas[i].free_space >= size) {

            cumulative_free_region = 0;
            /* walk the free list */
            for (j = metadata->arenas[i].free_region_idx;
                j < MAXREGIONS;
                j = metadata->regions[j].next_idx) {
                    assert(j != metadata->regions[j].next_idx);
                    if (GETREGIONSIZE(&metadata->regions[j]) >= size) {
                        *insert_idx = j;
                        break;
                    }

                    cumulative_free_region += GETREGIONSIZE(&metadata->regions[j]);
             }

            /* After walking through all the regions in the free list
             * the total of free blocks contributing to the hole
             * was found to be smaller than the requested size, thus
             * allowing for a new allocation in the arena_idx = i.
             */
            if (metadata->arenas[i].free_space - cumulative_free_region >= size) {
                 arena_idx = i;
                break;
            }
        }
    }

    return arena_idx;
}

static u_int16_t
pal_mm_find_32bit(pal_mem_metadata_t *metadata,
                  uint64_t size,
                  u_int16_t *insert_idx)
{
    u_int16_t arena_idx = MAXARENAS;
    u_int16_t i = 0;
    u_int16_t j = 0;
    u_int64_t cumulative_free_region = 0;

    for (i = 0; i < MAXARENAS; i++) {
        if (((metadata->arenas[i].flags & ALLOC32BITFLAG) != 0) &&
           metadata->arenas[i].free_space >= size) {

            cumulative_free_region = 0;
            *insert_idx = MAXREGIONS;
            /* walk the free list */
            for (j = metadata->arenas[i].free_region_idx;
                j < MAXREGIONS;
                j = metadata->regions[j].next_idx) {
                    assert(j != metadata->regions[j].next_idx);
                    if (GETREGIONSIZE(&metadata->regions[j]) >= size) {
                        *insert_idx = j;
                        break;
                    }

                    cumulative_free_region += GETREGIONSIZE(&metadata->regions[j]);
                }

            /* After walking through all the regions in the free list
             * the total of free blocks contributing to the hole
             * was found to be smaller than the requested size, thus
             * allowing for a new allocation in the arena_idx = i.
             */
            if (metadata->arenas[i].free_space - cumulative_free_region >= size) {
                arena_idx = i;
                break;
            }
        }
    }

    return arena_idx;
}

void *
pal_mem_map_region(char *region_name)
{
    u_int16_t i = 0;
    void *va = NULL;
    pal_mem_metadata_t *metadata = NULL;
    pal_data_t *pd = pal_get_data();

    if (!pd->init_done) {
        printf("PAL has not been init-ed yet.\n");
        goto exit;
    }

    if (pal_rd_lock(MEMLOCK) == LCK_FAIL) {
        return NULL;
    }

    metadata = (pal_mem_metadata_t*) pal_map_metadata_int();

    for (i = 0; i < MAXREGIONS; i++) {
        if (strcmp(metadata->regions[i].region_name, region_name) ==  0 &&
           ISOCCUPIED(&metadata->regions[i])) {
            if (strncmp(metadata->regions[i].UUID, pd->app_uuid, MAXUUID) != 0 &&
               ISEXCL(&metadata->regions[i])) {
                printf("Application[%s] does not have permission to act on this region[%s].\n",
                                 pd->app_uuid, metadata->regions[i].region_name);
                goto exit;
            }
            break;
        }
    }

    if (i < MAXREGIONS) {
        va = pr_ptov(metadata->regions[i].start_addr,
                     GETREGIONSIZE(&metadata->regions[i]),
                     FREEACCESS);
        pr_mem_unmap(va);
    } else {
        printf("Could not find region\n");
    }

exit:
    pal_unmap_metadata();
    if (pal_rd_unlock(MEMLOCK) == LCK_FAIL) {
        printf("Failed to unlock.\n");
    }
    return va;
}

void
pal_mem_unmap_region(char *region_name)
{
    u_int16_t i = 0;
    pal_data_t *pd = pal_get_data();

    pal_mem_metadata_t *metadata = NULL;

    if (!pd->init_done) {
        printf("PAL has not been init-ed yet.\n");
        goto exit;
    }

    if (pal_rd_lock(MEMLOCK) == LCK_FAIL) {
        return;
    }

    metadata = (pal_mem_metadata_t*) pal_map_metadata_int();

    for (i = 0; i < MAXREGIONS; i++) {
        if (strcmp(metadata->regions[i].region_name, region_name) ==  0 &&
            ISOCCUPIED(&metadata->regions[i])) {
                if (strncmp(metadata->regions[i].UUID, pd->app_uuid, MAXUUID) != 0 &&
                           ISEXCL(&metadata->regions[i])) {
                    printf("Application[%s] does not have permission to act on this region[%s].\n",
                                   pd->app_uuid, metadata->regions[i].region_name);
                   return;
                    }

                break;
        }
    }

    if (i < MAXREGIONS) {
        void *va = pr_ptov(metadata->regions[i].start_addr,
                           GETREGIONSIZE(&metadata->regions[i]),
                           FREEACCESS);
        pr_mem_unmap(va);
    } else {
        printf("Could not find region\n");
    }

exit:
    pal_unmap_metadata();
    if (pal_rd_unlock(MEMLOCK) == LCK_FAIL) {
        printf("Failed to unlock.");
    }
    return;
}

u_int64_t
pal_mem_alloc(char *region_name,
                  uint32_t size,
                  u_int32_t alloc_flags)
{
    u_int16_t insert_idx = MAXREGIONS;
    u_int16_t arena_idx = 0;
    u_int16_t i = 0;
    u_int64_t ret = 0;

    pal_mem_metadata_t *metadata = NULL;
    pal_data_t *pd = pal_get_data();
        
    if (!pd->init_done) {
        printf("PAL unitialized. Cannot alloc.\n");
        return 0;
    }

    if (pal_wr_lock(MEMLOCK) == LCK_FAIL) {
        return 0;
    }
    metadata = (pal_mem_metadata_t*) pal_map_metadata_int();

    if (size < MINALLOC) {
        printf("Allocation failed. Allocation requests must atleast be %d", MINALLOC);
        goto exit;
    }

    size = pal_get_aligned_size_int(size);

    for (i = 0; i < MAXREGIONS; i++) {
        if (metadata->regions[i].region_name != NULL &&
            strcmp(metadata->regions[i].region_name, region_name) ==  0 &&
            ISOCCUPIED(&metadata->regions[i]) &&
            ISALLOCATED(&metadata->regions[i])) {
               printf("Region name %s already exists.\n\n", region_name);
               goto exit;
          }
    }

    /* TODO : Refactor the following statements. */
    if ((alloc_flags & ALLOC32BITFLAG) == 0) {
        if ((alloc_flags & COHERENTFLAG) != 0) {
            arena_idx = pal_mm_find_coherent_int(metadata, size, &insert_idx);
        } else {
            arena_idx = pal_mm_find_non_coherent_int(metadata, size, &insert_idx);
        }
    } else {
        arena_idx = pal_mm_find_32bit(metadata, size, &insert_idx);
    }

    if (arena_idx == MAXARENAS) {
        printf("There is no arena available to satisfy the alloc request.\n");
        goto exit;
    }

   /* Search for a region which is large enough to accomodate the request is 
    * unavailable. Which means a new region has to be used and the arena has 
    * to be extended. This is equivalent to the sbrk() call.
    */ 
   if (insert_idx == MAXREGIONS) {
       for (i = 0; i < MAXREGIONS; i++) {
           if (!ISOCCUPIED(&metadata->regions[i])) {
               assert(metadata->arenas[arena_idx].top + size >
                      metadata->arenas[arena_idx].size);
               insert_idx = i;
               metadata->regions[insert_idx].start_addr = metadata->arenas[arena_idx].top;
               metadata->arenas[arena_idx].top += size;
                   SETREGIONSIZE(&metadata->regions[insert_idx], size);
               break;
           }
       }
    } else {
        /* Writing as a separate if condition inside the else for readability */
        if (GETREGIONSIZE(&metadata->regions[insert_idx]) - size > MINALLOC) {
             pal_mem_split(metadata, insert_idx, size);
        } 
    }

    if (insert_idx == MAXREGIONS) {
        goto exit;        
    }

    strncpy(metadata->regions[insert_idx].UUID, pd->app_uuid, MAXUUID);
    strcpy(metadata->regions[insert_idx].region_name, region_name);

    SETALLOCATED(&metadata->regions[insert_idx]);

    if ((alloc_flags & EXCLFLAG) != 0) {
        SETEXCL(&metadata->regions[insert_idx]);
    }

    if ((alloc_flags & COHERENTFLAG) != 0) {
        SETCOHERENT(&metadata->regions[insert_idx]);
    }

    if (metadata->regions[insert_idx].prev_idx != MAXREGIONS) {
        metadata->regions[metadata->regions[insert_idx].prev_idx].next_idx =
         metadata->regions[insert_idx].next_idx;
    } else if (metadata->arenas[arena_idx].free_region_idx == insert_idx){
        /* If it was already occupied, it must be part of a free list */
        metadata->arenas[arena_idx].free_region_idx =
         metadata->regions[insert_idx].next_idx;
    }

    if (metadata->regions[insert_idx].next_idx != MAXREGIONS) {
        metadata->regions[metadata->regions[insert_idx].next_idx].prev_idx =
         metadata->regions[insert_idx].prev_idx;
    } 

    SETOCCUPIED(&metadata->regions[insert_idx]);

    metadata->regions[insert_idx].next_idx = metadata->arenas[arena_idx].allocated_region_idx;
    metadata->regions[insert_idx].prev_idx = MAXREGIONS;
    metadata->regions[metadata->arenas[arena_idx].allocated_region_idx].prev_idx = insert_idx;
    metadata->regions[insert_idx].arena_idx = arena_idx;

    metadata->arenas[arena_idx].allocated_region_idx = insert_idx;
    metadata->arenas[arena_idx].free_space -= size;

    ret = metadata->regions[insert_idx].start_addr;

    print_region(&metadata->regions[insert_idx]);
exit:
    pal_unmap_metadata();

    if (pal_wr_unlock(MEMLOCK) == LCK_FAIL) {
        pal_mem_trace("Failed to unlock.\n");
    }

    return ret;
}

void
pal_mem_free(char *region_name)
{
    u_int16_t arena_idx = 0;
    u_int16_t i = 0;

    pal_mem_metadata_t *metadata = NULL;
    pal_data_t *pd = pal_get_data();

    if (!pd->init_done) {
        printf("PAL has not been init-ed yet.\n");
        goto exit;
    }

    if (pal_wr_lock(MEMLOCK) == LCK_FAIL) {
        printf("Couldnot get write lock. Exiting.");
        return;
    }
    metadata = (pal_mem_metadata_t*) pal_map_metadata_int();

    for (i = 0; i < MAXREGIONS; i++) {
        if (strcmp(metadata->regions[i].region_name, region_name) == 0 &&
             ISOCCUPIED(&metadata->regions[i]) &&
             ISALLOCATED(&metadata->regions[i])) {
                printf("Region name %s exists.\n", region_name);
                arena_idx = metadata->regions[i].arena_idx;
                    SETOCCUPIED(&metadata->regions[i]);
                    SETFREE(&metadata->regions[i]);

                 /* Memory has been reclaimed, so reset the memory region to be default == shareable */
                    SETSHAREABLE(&metadata->regions[i]);

                assert(!ISALLOCATED(&metadata->regions[i]));

                    memset(metadata->regions[i].UUID, 0, MAXUUID);
                    memset(metadata->regions[i].region_name, 0, MAXREGIONNAME);

                if (metadata->regions[i].prev_idx != MAXREGIONS) {
                    metadata->regions[metadata->regions[i].prev_idx].next_idx = metadata->regions[i].next_idx;
                } else {
                    metadata->arenas[arena_idx].allocated_region_idx = metadata->regions[i].next_idx;
                }

                if (metadata->regions[i].next_idx != MAXREGIONS) {
                    metadata->regions[metadata->regions[i].next_idx].prev_idx = metadata->regions[i].prev_idx;
                }

                    metadata->regions[i].prev_idx = MAXREGIONS;
                    metadata->regions[i].next_idx = metadata->arenas[arena_idx].free_region_idx;
                    metadata->regions[metadata->arenas[arena_idx].free_region_idx].prev_idx = i;
                    metadata->arenas[arena_idx].free_region_idx = i;
                    metadata->arenas[arena_idx].free_space += GETREGIONSIZE(&metadata->regions[i]);
                    pal_coalesce_int(metadata, arena_idx, metadata->arenas[arena_idx].free_region_idx);

                break;
          }
    }

    if (i == MAXREGIONS) {
        printf("Region %s not found.\n", region_name);
    }

exit:
    pal_unmap_metadata();

    if (pal_wr_unlock(MEMLOCK) == LCK_FAIL) {
        pal_mem_trace("Failed to unlock.\n");
    }
}

pal_ret_t
pal_pa_access_allowed(u_int64_t pa, u_int64_t sz)
{
    pal_data_t *pd = pal_get_data();
    pal_mem_metadata_t *metadata;
    u_int16_t i = 0;
    u_int8_t needs_check = 0;

    if (pal_rd_lock(MEMLOCK) == LCK_FAIL) {
        return PAL_FAIL;
    }

    metadata = pal_map_metadata_int();
    assert(metadata);
    assert(pd);

    for (i = 0; i < MAXREGIONS; i++) {
       /* TODO : Should a region be occupied for it to be range checked? */
       if (ISOCCUPIED(&metadata->regions[i])) { 
           /* Current region is the start region */ 
           print_region(&metadata->regions[i]);

           if (pa >= metadata->regions[i].start_addr &&
              pa < metadata->regions[i].start_addr + GETREGIONSIZE(&metadata->regions[i])) {
               needs_check = 1;
           }

           /* Current region is the in-between region */
           if (!needs_check &&
              pa >= metadata->regions[i].start_addr &&
              (pa + sz) < metadata->regions[i].start_addr + GETREGIONSIZE(&metadata->regions[i])) {
                needs_check = 1;
           }

           /* Current region is the final region */
           if (!needs_check &&
              (pa + sz) < metadata->regions[i].start_addr + GETREGIONSIZE(&metadata->regions[i]) &&
              (pa + sz) >= metadata->regions[i].start_addr) {
               needs_check = 1;        
           }

           if (needs_check &&
               ISEXCL(&metadata->regions[i]) &&
               strncmp(metadata->regions[i].UUID, pd->app_uuid, MAXUUID) != 0) {
               printf("Application[%s] does not have access permission for this region[%s].\n",
                         pd->app_uuid, metadata->regions[i].region_name);
               return PAL_FAIL;
           }                  
       }

       needs_check = 0;
   }

   pal_unmap_metadata();
   if (pal_rd_unlock(MEMLOCK) == LCK_FAIL) {
        pal_mem_trace("Failed to unlock.\n");
   }
   return PAL_SUCCESS;
}

void
pal_print_application_regions(char *application_name)
{
    pal_mem_metadata_t *metadata = NULL;
    u_int16_t i = 0;
    u_int64_t total_regions = 0;
    u_int64_t total_allocated  = 0;
    u_int64_t total_shareable = 0;
    u_int64_t total_coherent_int = 0;

    if (pal_rd_lock(MEMLOCK) == LCK_FAIL) {
        return;
    }

    metadata = pal_map_metadata_int();
    assert(metadata);

    for (i = 0; i < MAXREGIONS; i++) {
        if (strncmp(metadata->regions[i].UUID, application_name, MAXUUID) == 0 &&
            ISALLOCATED(&metadata->regions[i])) {
            print_region(&metadata->regions[i]);
            total_regions++;
            total_allocated += GETREGIONSIZE(&metadata->regions[i]);
            
            if (ISSHAREABLE(&metadata->regions[i])) {
                total_shareable += GETREGIONSIZE(&metadata->regions[i]);
            }

            if (ISCOHERENT(&metadata->regions[i])) {
                total_coherent_int += GETREGIONSIZE(&metadata->regions[i]);
            }
        }
    }

    printf("\n\n======================================================================");
    printf("\nThe aggregate statistics of the application [%s] are", application_name);
    printf("\nTotal regions allocated : %ld", total_regions); 
    printf("\nTotal memory allocated (Bytes) : %ld", total_allocated); 
    printf("\nTotal shareable memory allocated (Bytes) : %ld", total_shareable); 
    printf("\nTotal exclusive memory allocated (Bytes) : %ld", total_allocated - total_shareable); 
    printf("\nTotal coherent memory allocated (Bytes) : %ld", total_coherent_int); 
    printf("\nTotal non-coherent memory allocated (Bytes) : %ld", total_allocated - total_coherent_int); 
    printf("\n======================================================================\n");

    pal_unmap_metadata();
    if (pal_rd_unlock(MEMLOCK) == LCK_FAIL) {
        pal_mem_trace("Failed to unlock.\n");
    }
}

void
pal_print_metadata(void) {
    pal_mem_metadata_t *metadata = NULL;

    if (pal_rd_lock(MEMLOCK) == LCK_FAIL) {
        return;
    }

    metadata = pal_map_metadata_int();
    assert(metadata);
    print_metadata(metadata);
    pal_unmap_metadata();

    if (pal_rd_unlock(MEMLOCK) == LCK_FAIL) {
        pal_mem_trace("Failed to unlock.\n");
    }
}

u_int64_t
pal_mem_region_pa(char *region_name)
{
    u_int16_t i = 0;
    u_int64_t pa = 0;
    pal_mem_metadata_t *metadata = NULL;
    pal_data_t *pd = pal_get_data();

    if (!pd->init_done) {
        printf("PAL has not been init-ed yet.\n");
        goto exit;
    }

    if (pal_rd_lock(MEMLOCK) == LCK_FAIL) {
            goto exit; 
    }

    metadata = (pal_mem_metadata_t*) pal_map_metadata_int();

    for (i = 0; i < MAXREGIONS; i++) {
        if (strcmp(metadata->regions[i].region_name, region_name) ==  0 &&
           ISOCCUPIED(&metadata->regions[i])) {
            if (strncmp(metadata->regions[i].UUID, pd->app_uuid, MAXUUID) != 0 &&
               ISEXCL(&metadata->regions[i])) {
                printf("Application[%s] does not have permission to act on this region[%s].\n",
                                 pd->app_uuid, metadata->regions[i].region_name);
                goto exit;
            }
            break;
        }
    }

    if (i < MAXREGIONS) {
        pa = metadata->regions[i].start_addr;
    }

exit:
    pal_unmap_metadata();
    if (pal_rd_unlock(MEMLOCK) == LCK_FAIL) {
        printf("Failed to unlock.\n");
    }
    return pa; 
}

