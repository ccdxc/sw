// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#ifndef _SDK_METRICS_SHM_SHM_TYPES_H_
#define _SDK_METRICS_SHM_SHM_TYPES_H_

#include <memory>
#include <stdint.h>

#include "error.hpp"

namespace sdk {
namespace metrics {

// pack the structs
#define PACKED __attribute__((__packed__))

#define NUM_SLAB_LAYERS 10 // number of slab layers (one layer for each chunk size)

// magic string in shared memory
const int64_t kShmMagic = 0x70656e73616e646fll;

// shared memory metadata version
const int32_t kShmVersion = 0x1;

// ShmHdr_t is the shared memory metadata header format
typedef struct ShmHdr_ {
    int64_t    magic;      // magic bytes identifying this region
    int32_t    version;    // metadata version number
    int32_t    size;       // size of this shared memory region in bytes
} PACKED ShmHdr_t;

// Slab_t contains metadata about each slab (or page)
typedef struct Slab_ {
    int32_t    page_size;      // size of this page
    int32_t    chunk_size;     // size of each chunk
    int32_t    page_idx;       // index of current slab
    int32_t    layer_idx;      // index of slab list that owns this slab
    int32_t    next_page_idx;  // next slab in the linked list
    int32_t    prev_page_idx;  // previous slab in the linked list
    int32_t    num_chunks;     // number of chunks in this slab
    int32_t    num_free_chunks;   // number of free chunks
    int32_t    free_chunk_head;   // head of free chunk linked list
} PACKED Slab_t;

// SlabChunk_t is used to maintain a free list of chunks in a slab
typedef struct SlabChunk_ {
    int32_t     chunk_idx;         // current chunk's idx (starts from 1)
    int32_t     next_chunk_idx;    // next chunk in the linked list
    int32_t     prev_chunk_idx;    // previous chunk in the linked list
} PACKED SlabChunk_t;

// SlabList_t is linked list of slabs per chunk size (or for freelist mgmt)
typedef struct SlabList_ {
    int32_t    chunk_size;       // chunk size of slabs. (0 means its the free list)
    int32_t    layer_idx;        // layer owning this slab list
    int32_t    head_page_idx;    // head of the slab linked list
    int32_t    tail_page_idx;    // tail of the slab linked list
    int32_t    num_slabs;        // number of slabs in the list
    int32_t    num_free_chunks;  // number of free chunks in existing slabs
} PACKED SlabList_t;

// SlabPool_t contains information about slab memory allocator
typedef struct SlabPool_ {
    int32_t      lock;              // lock to make sure multiple processes are not working on slab pool at the same time
    int32_t      size;              // size of the total memory
    int32_t      page_size;         // page size
    int32_t      num_pages;         // total number of pages in this pool
    int32_t      num_free_pages;    // number of free pages
    SlabList_t   layers[NUM_SLAB_LAYERS + 1]; // slab layers. (layer-0 is free list)
} PACKED SlabPool_t;

// ShmMeta_t contains the metadata about the shared memory region.
// this metadata is stored at the head of the shared memory
typedef struct ShmCtx_ {
    ShmHdr_t   hdr;           // metadata hdr
    SlabPool_t mempool;       // slab memory allocator state
    int32_t    kvstore_root;  // offset of kvstore root table
} PACKED ShmMeta_t;

// forward declaration
class Shm;
typedef std::shared_ptr<Shm> ShmPtr;

} // namespace metrics
} // namespace sdk

#endif // _SDK_METRICS_SHM_SHM_TYPES_H_
