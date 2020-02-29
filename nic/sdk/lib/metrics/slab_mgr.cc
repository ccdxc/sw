// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was fork from delphi

// this file contains slab allocator for shared memory

#include "slab_mgr.hpp"

namespace sdk {
namespace metrics {

// Slab allocator
// This is roughly how the slab allocator works
//Its laregly based on original slab allocator paper:
// https://www.usenix.org/legacy/publications/library/proceedings/bos94/full_papers/bonwick.a
//
// Entire shared memory space is broken down into equal sized pages typically
// of length 64KB. Page-0 is reserved for shared memory metadata and allocator state.
// Rest of the pages are used for allocating memory.
//
//  Shared memory:
//  +------------+------------+------------+------------+------------+---
//  |   Page 0   |   Page 1   |   Page 2   |   Page 3   |   Page 4   | ..
//  +------------+------------+------------+------------+------------+---
//
// Each page is broken up into multiple equal sized chunks. This is called a slab.
// At the end of the page, we maintain a slab info data structure that contains information
// about all the chunks in the page.
//
// Slab (or page):
//  +-----------+-----------+-----------+-----------+-----------+------------------+
//  |  Chunk 1  |  Chunk 2  |  Chunk 3  |  Chunk 4  |  Chunk 5  |        Slab info |
//  +-----------+-----------+-----------+-----------+-----------+------------------+
//
// slab info keeps track of the chunk size, number of free chunks and a linked list
// of currently available chunks. When an alloc request comes in, first chunk
// from the free list of chunks is allocated and the pointer is returned
//
// Shared memory metadata contains a memory pool datastructure. This datastructure
// contains a freelist which is a linked list of slabs(i.e pages). Allocator also maintains
// cache of slabs for various size of objects. These are called layers. Each layer contains
// a linked list of slabs that are allocated to that layer. Currently there is a layer for
// object sizes: 40, 80, 160, 320, 640, 1280, 2560, 5120, 10240, 20480.
// When an alloc request comes in, nearest layer thats bigger than the required length
// is picked and a chunk is allocated from that layer. If the layer has no free chunk,
// new page is allocated to that layer from the free list of slabs.
//
// Slab Pool info:
//  +------------------+
//  |  Pool info       |
//  +------------------+
//  |                  |     +--------+     +--------+
//  |   Freelist       +---->+ Slab 1 +---->+ Slab 2 |
//  |                  |     +--------+     +--------+
//  +------------------+
//  |                  |     +--------+     +--------+
//  |    Layer 1       +---->+ Slab 3 +---->+ Slab 4 |
//  |                  |     +--------+     +--------+
//  +------------------+
//  |                  |     +--------+     +--------+
//  |    Layer 2       +---->+ Slab 5 +---->+ Slab 6 |
//  |                  |     +--------+     +--------+
//  +------------------+
//  |    Layer ...     |
//  +------------------+
//

// SlabAllocator constructor
SlabAllocator::SlabAllocator(SlabPool_t *pool, uint8_t *mem_base, int32_t my_id) {
    pool_     = pool;
    mem_base_ = mem_base;
    my_id_    = my_id;
}

// InitMem initializes the memory for slab allocation
error  SlabAllocator::InitMem(int32_t mem_size) {
    // verify the mem size is multiple of page size
    if ((mem_size % SHM_PAGE_SIZE) != 0) {
        //LogError("Memory size needs to be multiple of page size 64kb");
        return error::New("Invalid memory size");
    }

    // calculate number of pages (we reserve page-0 for shm metadata)
    int32_t num_pages = (mem_size / SHM_PAGE_SIZE) - 1;

    // init the lock
    spin_lock_init(&pool_->lock);

    // init the pool metadata
    pool_->size      = mem_size;
    pool_->page_size = SHM_PAGE_SIZE;
    pool_->num_pages = num_pages;
    pool_->num_free_pages = num_pages;

    // init the layers
    for (int i = 0; i < (NUM_SLAB_LAYERS + 1); i++) {
        SlabList_t *layer = &pool_->layers[i];
        layer->layer_idx = i;
        if (i == 0) {
            layer->chunk_size = 0;
            layer->num_slabs  = num_pages;
            layer->head_page_idx = 1;
            layer->tail_page_idx = num_pages;
        } else {
            layer->chunk_size = BASE_CHUNK_SIZE * (1 << (i - 1));
            layer->num_slabs  = 0;
            layer->head_page_idx = 0;
            layer->tail_page_idx = 0;
        }
    }

    // init each page
    for (int sidx = 1; sidx < (num_pages + 1); sidx++) {
        // slab info is at the end of each page
        Slab_t *slab_info = SLAB_INFO_FROM_PAGE_ID(mem_base_, sidx);

        slab_info->page_size = SHM_PAGE_SIZE;
        slab_info->chunk_size = 0;
        slab_info->page_idx = sidx;
        slab_info->layer_idx = 0;
        if (sidx == num_pages) {
            slab_info->next_page_idx = 0; // last slab in the list point to null
        } else {
            slab_info->next_page_idx = sidx + 1;
        }
        slab_info->prev_page_idx = sidx - 1; // first slab in the list points to null
        slab_info->num_chunks = 0;
        slab_info->num_free_chunks = 0;
        slab_info->free_chunk_head = 0;
    }

    return error::OK();
}

// Alloc allocates memory of requested size
// Note slab allocator has small number of fixed chunk sizes, allocator finds the
// closest chunk size and allocates memory from that slab
uint8_t * SlabAllocator::Alloc(int32_t len) {
    Slab_t * slab;

    // we can not allocate bigger than largest chunk size
    if ((len > MAX_CHUNK_SIZE) || (len <= 0)) {
        return NULL;
    }

    // lock the allocator
    spin_lock(&pool_->lock);

    // determine the slab layer and chunk size
    int32_t layer_idx = layer_from_len(len);
    SlabList_t *layer = &pool_->layers[layer_idx];
    int32_t chunk_size = layer->chunk_size;

    // if the layer has no free chunk, get a page from free list
    if ((layer->head_page_idx == 0) || (layer->num_free_chunks == 0)) {
        slab = allocatePage(chunk_size);
        if (slab == NULL) {
            //LogError("Failed to allocate memory of len {}. No free pages", len);
            spin_unlock(&pool_->lock);
            return NULL;
        }

        // add the slab to the head of the list
        error err = insertSlabAtHead(slab, layer);
        assert(err.IsOK());
        layer->num_slabs++;
        layer->num_free_chunks += slab->num_free_chunks;
    } else {
        // find the first slab from the linked list
        // if we have any free space in this layer, its almost guaranteed that first slab will have
        // at least one chunk free
        slab = SLAB_INFO_FROM_PAGE_ID(mem_base_, layer->head_page_idx);
        if (slab->num_free_chunks == 0) {
            while ((slab->num_free_chunks == 0) && (slab->next_page_idx != 0)) {
                slab = SLAB_INFO_FROM_PAGE_ID(mem_base_, slab->next_page_idx);
            }
        }
    }
    assert(slab->num_free_chunks != 0);

    // allocate a chunk from the slab
    uint8_t *ptr = allocateChunk(slab, layer);
    assert(ptr != NULL);
    layer->num_free_chunks--;

    // unlock the allocator
    spin_unlock(&pool_->lock);

    return ptr;
}

// Free frees memory and moves the chunk back to the free list
error  SlabAllocator::Free(uint8_t *ptr) {
    if ((ptr < mem_base_) || (ptr > (mem_base_ + pool_->size))) {
        //LogError("Invalid address {:p} during free. memory Pool is {} - {}", ptr, mem_base_, (mem_base_ + pool_->size));
        return error::New("Invalid memory address to free");
    }

    // lock the allocator
    spin_lock(&pool_->lock);

    // get the page id from pointer address
    int32_t page_idx = PAGE_ID_FROM_PTR(mem_base_, ptr);

    // get hold of the slab info and layer
    Slab_t *slab = SLAB_INFO_FROM_PAGE_ID(mem_base_, page_idx);
    SlabList_t *layer = &pool_->layers[slab->layer_idx];

    // if the slab is in free list, we cant free it
    if (slab->layer_idx == 0) {
        spin_unlock(&pool_->lock);
        return error::New("Duplicate free");
    }

    // free the chunk in the slab
    error err = freeChunk(ptr, slab, layer);
    if (err.IsNotOK()) {
        //LogError("Error freeing chunk from page {} for layer {}. Err: {}", slab->page_idx, slab->layer_idx, err);
        spin_unlock(&pool_->lock);
        return err;
    }

    // if all the chunks in the slab are free, free the page
    if (slab->num_free_chunks == slab->num_chunks) {
        err = freePage(slab, layer);
        if (err.IsNotOK()) {
            spin_unlock(&pool_->lock);
            //LogError("Error freeing page {}. Err: {}", slab->page_idx, err);
            return err;
        }
    }

    // unlock the allocator
    spin_unlock(&pool_->lock);

    return error::OK();
}

void SlabAllocator::Print() {
    printf("Pool:\nsize: %d, page_size: %d, num_pages: %d, free_pages: %d\n",
            pool_->size, pool_->page_size, pool_->num_pages, pool_->num_free_pages);
    printf("Free list:\n");
    SlabList_t *freelist = &pool_->layers[0];
    printf("  num_slabs: %d, head_page_idx: %d, tail_page_idx: %d\n",
            freelist->num_slabs, freelist->head_page_idx, freelist->tail_page_idx);
    printf("Layers:\n");
    for (int i = 0; i < NUM_SLAB_LAYERS; i++) {
        SlabList_t *layer = &pool_->layers[i + 1];
        printf("  layer: %d, num_slabs: %d, chunk size: %d, chunks: %d, free_chunks: %d\n",
                layer->layer_idx, layer->num_slabs, layer->chunk_size,
                (layer->num_slabs * CHUNKS_PER_PAGE(layer->chunk_size)),
                layer->num_free_chunks);
        printf("  head_page_idx: %d, tail_page_idx: %d\n", layer->head_page_idx, layer->tail_page_idx);
    }
}

// allocatePage dequeues a page from free list and initializes it for a chunk size
Slab_t * SlabAllocator::allocatePage(int32_t chunk_size) {
    // if free list is empty we are out of memory
    if (pool_->num_free_pages <= 0) {
        //LogError("No free pages. Out of memory");
        return NULL;
    }

    // free list
    SlabList_t *freelist = &pool_->layers[0];
    assert(freelist->head_page_idx != 0);

    // dequeue the page from free list
    pool_->num_free_pages--;
    Slab_t *slab = SLAB_INFO_FROM_PAGE_ID(mem_base_, freelist->head_page_idx);
    assert(slab != NULL);
    error err = removeSlab(slab, freelist);
    assert(err.IsOK());

    // init slab info
    slab->chunk_size = chunk_size;
    slab->prev_page_idx = slab->next_page_idx = 0;

    // create chunks in the page
    int32_t num_chunks = (slab->page_size - sizeof(Slab_t)) / chunk_size;
    slab->num_chunks = num_chunks;
    slab->num_free_chunks = num_chunks;
    slab->free_chunk_head = 1; // chunk idx starts at 1

    // put all chunks in the free list
    for (int i = 0; i < num_chunks; i++) {
        SlabChunk_t *chunk = SLAB_CHUNK_FROM_IDX(slab, (i + 1));
        chunk->chunk_idx = i + 1;
        if (i == (num_chunks - 1)) {
            chunk->next_chunk_idx = 0;   // last chunk points at null
        } else {
            chunk->next_chunk_idx = (i + 2);
        }
        chunk->prev_chunk_idx = i; // first chunk points at null
    }

    return slab;
}

// freePage moves a page back to free list
error SlabAllocator::freePage(Slab_t *slab, SlabList_t *layer) {
    SlabList_t *freelist = &pool_->layers[0];

    // remove the slab from this layer
    error err = removeSlab(slab, layer);
    assert(err.IsOK());
    assert(layer->num_free_chunks >= slab->num_chunks);
    layer->num_free_chunks -= slab->num_chunks;
    layer->num_slabs--;

    // add it to the free list
    err = insertSlabAtHead(slab, freelist);
    assert(err.IsOK());
    pool_->num_free_pages++;

    return error::OK();
}

// allocateChunk allocates a chunk from a slab
// if the slab has no more free chunks left, it moves the slab to the tail of the slab list
uint8_t * SlabAllocator::allocateChunk(Slab_t *slab, SlabList_t *layer) {
    // some error checking
    assert(slab->num_free_chunks > 0);
    assert(slab->free_chunk_head != 0);

    // dequeue the head of the free chunks
    SlabChunk_t *chunk = SLAB_CHUNK_FROM_IDX(slab, slab->free_chunk_head);
    assert(chunk->chunk_idx == slab->free_chunk_head);
    slab->num_free_chunks--;
    if (chunk->next_chunk_idx == 0) {
        assert(slab->num_free_chunks == 0);
        slab->free_chunk_head = 0;

        // move this slab to the tail of the slab list
        error err = removeSlab(slab, layer);
        assert(err.IsOK());
        err = insertSlabAtTail(slab, layer);
        assert(err.IsOK());
    } else {
        SlabChunk_t *next_chunk = SLAB_CHUNK_FROM_IDX(slab, chunk->next_chunk_idx);
        if (next_chunk->chunk_idx != chunk->next_chunk_idx) {
            //LogError("Invalid chunk data, n->cidx: {}, c->nidx: {}, ", next_chunk->chunk_idx, chunk->next_chunk_idx);
        }
        assert(next_chunk->chunk_idx == chunk->next_chunk_idx);
        next_chunk->prev_chunk_idx = 0;
        slab->free_chunk_head = next_chunk->chunk_idx;
    }

    return (uint8_t *)chunk;
}

// freeChunk moves a chunk to free list
error SlabAllocator::freeChunk(uint8_t *ptr, Slab_t *slab, SlabList_t *layer) {
    // calculate the chunk idx from pointer
    int32_t chunk_idx = CHUNK_IDX_FROM_PTR(slab, ptr);
    assert(chunk_idx != 0);

    // get a pointer to the chunk
    SlabChunk_t *chunk = SLAB_CHUNK_FROM_IDX(slab, chunk_idx);
    assert(chunk != NULL);

    // add the chunk to the free list
    slab->num_free_chunks++;
    layer->num_free_chunks++;
    chunk->chunk_idx = chunk_idx;
    chunk->prev_chunk_idx = 0;
    chunk->next_chunk_idx = slab->free_chunk_head;
    slab->free_chunk_head = chunk_idx;

    // move the slab to the head of the slab list
    error err = removeSlab(slab, layer);
    assert(err.IsOK());
    err = insertSlabAtHead(slab, layer);
    assert(err.IsOK());

    return error::OK();
}

// insertSlabAtHead insterts the slab at the head of linked list
error SlabAllocator::insertSlabAtHead(Slab_t *slab, SlabList_t *layer) {
    slab->layer_idx = layer->layer_idx;

    // check if the slab is already at the head
    if (layer->head_page_idx == slab->page_idx) {
        return error::OK();
    }

    // insert at the head of linked list
    if (layer->head_page_idx == 0) {
        assert(layer->tail_page_idx == 0);
        layer->head_page_idx = slab->page_idx;
        layer->tail_page_idx = slab->page_idx;
        slab->next_page_idx = slab->prev_page_idx = 0;
    } else {
        assert(layer->tail_page_idx != 0);
        Slab_t *next_slab        = SLAB_INFO_FROM_PAGE_ID(mem_base_, layer->head_page_idx);
        slab->next_page_idx      = layer->head_page_idx;
        next_slab->prev_page_idx = slab->page_idx;
        layer->head_page_idx     = slab->page_idx;
    }

    return error::OK();
}

// insertSlabAtTail inserts the slab at the tail of the list
error SlabAllocator::insertSlabAtTail(Slab_t *slab, SlabList_t *layer) {
    slab->layer_idx = layer->layer_idx;

    // check if the slab is already at the tail
    if (layer->tail_page_idx == slab->page_idx) {
        return error::OK();
    }

    // insert at the tail of linked list
    if (layer->tail_page_idx == 0) {
        assert(layer->head_page_idx == 0);
        layer->head_page_idx = slab->page_idx;
        layer->tail_page_idx = slab->page_idx;
        slab->next_page_idx  = slab->prev_page_idx = 0;
    } else {
        Slab_t *tail_slab        = SLAB_INFO_FROM_PAGE_ID(mem_base_, layer->tail_page_idx);
        slab->prev_page_idx      = tail_slab->page_idx;
        tail_slab->next_page_idx = slab->page_idx;
        layer->tail_page_idx     = slab->page_idx;
        slab->next_page_idx      = 0;
    }

    return error::OK();
}

// removeSlab removes the slab from the list
error SlabAllocator::removeSlab(Slab_t *slab, SlabList_t *layer) {
    assert(layer->head_page_idx != 0);
    assert(layer->tail_page_idx != 0);

    // if both head and tail are pointing at the slab, just empty the list
    if ((layer->head_page_idx == slab->page_idx) && (layer->tail_page_idx == slab->page_idx)) {
        layer->head_page_idx = 0;
        layer->tail_page_idx = 0;
        return error::OK();
    }

    // remove the slab from linked list
    if (layer->head_page_idx == slab->page_idx) {
        Slab_t *next_slab    = SLAB_INFO_FROM_PAGE_ID(mem_base_, slab->next_page_idx);
        assert(next_slab->page_idx == slab->next_page_idx);
        layer->head_page_idx = next_slab->page_idx;
        next_slab->prev_page_idx = 0;
    } else if (layer->tail_page_idx == slab->page_idx) {
        Slab_t *prev_slab  = SLAB_INFO_FROM_PAGE_ID(mem_base_, slab->prev_page_idx);
        assert(prev_slab->page_idx == slab->prev_page_idx);
        layer->tail_page_idx = prev_slab->page_idx;
        prev_slab->next_page_idx = 0;
    } else {
        Slab_t *next_slab    = SLAB_INFO_FROM_PAGE_ID(mem_base_, slab->next_page_idx);
        Slab_t *prev_slab  = SLAB_INFO_FROM_PAGE_ID(mem_base_, slab->prev_page_idx);
        assert(next_slab->page_idx == slab->next_page_idx);
        assert(prev_slab->page_idx == slab->prev_page_idx);
        next_slab->prev_page_idx = prev_slab->page_idx;
        prev_slab->next_page_idx = next_slab->page_idx;
    }

    return error::OK();
}

} // namespace metrics
} // namespace sdk
