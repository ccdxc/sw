// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#ifndef _SDK_METRICS_SHM_SLAB_MGR_H_
#define _SDK_METRICS_SHM_SLAB_MGR_H_

#include <math.h>

#include "atomic.hpp"
#include "error.hpp"
#include "shm_types.hpp"

namespace sdk {
namespace metrics {

#define BASE_CHUNK_SIZE    40    // chunk size starts from 40 and goes 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 20480
#define SHM_PAGE_SIZE      (64 * 1024) // page size. shared memory id divided into pages of 64KBs
#define MAX_CHUNK_SIZE     (BASE_CHUNK_SIZE * (1 << (NUM_SLAB_LAYERS - 1))) // we can not allocate objects bigger than this

// SLAB_INFO_FROM_PAGE_ID gets a pointer to slab info from page idx
#define SLAB_INFO_FROM_PAGE_ID(mbase, pidx) ((Slab_t *)(mbase + (pidx * SHM_PAGE_SIZE) + (SHM_PAGE_SIZE - sizeof(Slab_t))))

// SLAB_CHUNK_FROM_IDX returns a pointer to chunk from slab pointer and chunk idx
// note that chunk_idx start from 1 instead of 0
#define SLAB_CHUNK_FROM_IDX(slab, chunk_idx) ((SlabChunk_t *)((((uint8_t *)slab) - SHM_PAGE_SIZE) + sizeof(Slab_t) + ((chunk_idx - 1) * slab->chunk_size)))

// PAGE_ID_FROM_PTR returns page id from pointer address
#define PAGE_ID_FROM_PTR(mbase, ptr) ((ptr - mbase) / SHM_PAGE_SIZE)

// OFFSET_FROM_PTR calculates the offset from base of shared memory
#define OFFSET_FROM_PTR(mbase, ptr) ((uint8_t *)(ptr) - (uint8_t *)(mbase))

// PTR_FROM_OFFSET returns pointer from offset
#define PTR_FROM_OFFSET(mbase, offset) ((uint8_t *)(mbase) + offset)

// CHUNK_IDX_FROM_PTR returns chunk idx from pointer and slab info
#define CHUNK_IDX_FROM_PTR(slab, ptr) (((ptr - ((((uint8_t *)slab) - SHM_PAGE_SIZE) + sizeof(Slab_t))) / slab->chunk_size) + 1)

// CHUNKS_PER_PAGE is a macro to calculate number of chunk in a page, given chunk size
#define CHUNKS_PER_PAGE(chunk_size) (int32_t)((SHM_PAGE_SIZE - sizeof(Slab_t)) / chunk_size)

// SlabAllocator
class SlabAllocator {
public:
    SlabAllocator(SlabPool_t *pool, uint8_t *mem_base, int32_t my_id);  // constructor
    error     InitMem(int32_t mem_size);     // Initialize the memory for slab allocation
    uint8_t * Alloc(int32_t len);            // Allocate a chunk of memory
    error     Free(uint8_t *ptr);            // Free a chunk of memory
    void      Print();                       // print the current state of memory pool
private:
    SlabPool_t *pool_;
    uint8_t    *mem_base_;
    int32_t    my_id_;

    // private methods
    Slab_t *  allocatePage(int32_t chunk_size);                      // allocate a new page for a layer
    error     freePage(Slab_t *slab, SlabList_t *layer);             // free a page from layer
    uint8_t * allocateChunk(Slab_t *slab, SlabList_t *layer);        // allocate a chunk from a slab
    error     freeChunk(uint8_t *ptr, Slab_t *slab, SlabList_t *layer); // free the chunk from a slab
    error     insertSlabAtHead(Slab_t *slab, SlabList_t *layer);     // insert slab at the head of slab linked list
    error     insertSlabAtTail(Slab_t *slab, SlabList_t *layer);     // insert slab at the tail of slab linked list
    error     removeSlab(Slab_t *slab, SlabList_t *layer);           // remove slab from a slab linked list
};
typedef std::shared_ptr<SlabAllocator> SlabAllocatorPtr;
typedef std::unique_ptr<SlabAllocator> SlabAllocatorUptr;

// upper_power_of_two returns closest power of two for a given number
static inline int32_t upper_power_of_two(int32_t v) {
    if (v < 0) {
        return 0;
    }

    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;

}

// layer_from_len returns the slab layer number from allocation len
static inline int32_t layer_from_len(int32_t len) {
    if (len < 0) {
        return 0;
    }

    // if len is less than minimum size, use minimum chunk size
    if (len <= BASE_CHUNK_SIZE) {
        return 1;
    }

    // divide by base and round up to next integer
    int32_t base = len / BASE_CHUNK_SIZE;
    if ((len % BASE_CHUNK_SIZE) != 0) {
        base++;
    }

    // determine nearest power of two and calculate the log2 for it
    int32_t layer = int32_t(log2(upper_power_of_two(base))) + 1;
    return layer;
}

} // namespace metrics
} // namespace sdk

#endif // _SDK_METRICS_SHM_SLAB_MGR_H_
