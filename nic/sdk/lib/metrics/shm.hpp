// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#ifndef _SDK_METRICS_SHM_H_
#define _SDK_METRICS_SHM_H_

#include <memory>

#include "error.hpp"
#include "kvstore.hpp"
#include "slab_mgr.hpp"

namespace sdk {
namespace metrics {

#define SDK_METRICS_SHM_NAME  "metrics_shm"
#define SDK_METRICS_SHM_SIZE (32 * 1024 * 1024)

// Shm class represents a metrics shaed memory region
class Shm : public std::enable_shared_from_this<Shm> {
public:
    Shm();                                  // constructor
    error  MemMap(std::string name, int32_t size); // memory map the shared memory regior
    error  MemUnmap();                      // unmap the shared memory
    void * Alloc(int size);                 // allocate space from shared memory
    error  Free(void *ptr);                 // free space from shared memory
    void   DumpMeta();                      // dump contents of shm meta

    // IsMapped returns if shared memory is mapped
    inline bool IsMapped() { return mapped_; };

    // GetBase returns the base address of shared memory
    inline uint8_t * GetBase() { return (uint8_t *)shm_meta_; }

    // Kvstore returns kvstore instance
    inline KvstoreMgrPtr Kvstore() { return kvstore_; }

private:
    ShmMeta_t         *shm_meta_;    // pointer to shared memory memory-mapped region
    int32_t           my_id_;        // my identifier (typically my process id)
    std::string       shm_name_;     // name of the shared memory
    bool              mapped_;       // is shared memory mapped into memory already?
    SlabAllocatorUptr slab_mgr_;     // slab allocator
    KvstoreMgrPtr     kvstore_;      // key-value store

    // private methods
    error createMemMap(std::string name, int32_t size);
    error initSharedMemory(int32_t size);

};
typedef std::unique_ptr<Shm> ShmUptr;

} // namespace metrics
} // namespace sdk

#endif // _SDK_METRICS_SHM_H_
