// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SHM_SHM_H_
#define _DELPHI_SHM_SHM_H_

#include "nic/delphi/utils/utils.hpp"
#include "slab_mgr.hpp"
#include "kvstore.hpp"

namespace delphi {
namespace shm {

// DelphiShm class represents a delphi shaed memory region
class DelphiShm : public enable_shared_from_this<DelphiShm> {
public:
    DelphiShm();                            // constructor
    error  MemMap(string name, int32_t size, bool is_srv);   // memory map the shared memory regior
    error  MemUnmap(string name);           // unmap the shared memory
    void * Alloc(int size);                 // allocate space from shared memory
    error  Free(void *ptr);                 // free space from shared memory
    void   DumpMeta();                      // dump contents of shm meta

    // GetBase returns the base address of shared memory
    inline uint8_t * GetBase() { return (uint8_t *)shm_meta_; }

    // Kvstore returns kvstore instance
    inline KvstoreMgrPtr Kvstore() { return kvstore_; }

private:
    DelphiShmMeta_t    *shm_meta_;    // pointer to shared memory memory mapped region
    int32_t            my_id_;        // my identifier (typically my process id)
    bool               mapped_;
    SlabAllocatorUptr  slab_mgr_;      // slab allocator
    KvstoreMgrPtr      kvstore_;       // key-value store

    // private methods
    error initSharedMemory(int32_t size);

};
typedef std::unique_ptr<DelphiShm> DelphiShmUptr;

} // namespace shm
} // namespace delphi

#endif // _DELPHI_SHM_SHM_H_
