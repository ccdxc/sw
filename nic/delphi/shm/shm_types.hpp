// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SHM_SHM_TYPES_H_
#define _DELPHI_SHM_SHM_TYPES_H_

namespace delphi {
namespace shm {

// pack the structs
#define PACKED __attribute__((__packed__))

#define NUM_SLAB_LAYERS    10    // number of slab layers (one layer for each chunk size)

// delphi magic string in shared memory
const int64_t kDelphiShmMagic = 0x000A4948504C4544ll; // 'DELPHI\n\0' in hex in network order

// delphi shared memory metadata version
const int32_t kDelphiShmVersion = 0x1;

// DelphiShmHdr_t is the shared memory metadata header format
typedef struct DelphiShmHdr_ {
    int64_t    magic;      // magic bytes identifying this region
    int32_t    version;    // metadata version number
    int32_t    size;       // size of this shared memory region in bytes
} PACKED DelphiShmHdr_t;

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

// DelphiShmMeta_t contains the metadata about the shared memory region.
// this metadata is stored at the head of the shared memory
typedef struct DelphiShmCtx_ {
    DelphiShmHdr_t    hdr;           // metadata hdr
    SlabPool_t        mempool;       // slab memory allocator state
    int32_t           kvstore_root;  // offset of kvstore root table
} PACKED DelphiShmMeta_t;

// forward declaration
class DelphiShm;
typedef std::shared_ptr<DelphiShm> DelphiShmPtr;

} // namespace shm
} // namespace delphi

#endif // _DELPHI_SHM_SHM_TYPES_H_
