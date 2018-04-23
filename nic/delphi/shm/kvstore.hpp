// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_SHM_KVSTORE_H_
#define _DELPHI_SHM_KVSTORE_H_

#include "nic/delphi/utils/utils.hpp"
#include "shm_types.hpp"
#include "htable.hpp"

namespace delphi {
namespace shm {

// KvstoreMgr manages a shared memory key-value store.
// kvstore is maintained in shared memory as two levels of hash table.
// first level hash table is indexed by 'kind' and each entry is another hash table
// second level hash table is indexed by the 'key' and each entry stores the 'value'
class KvstoreMgr {
public:
    KvstoreMgr(int32_t *kvstore_root, DelphiShmPtr shm_ptr);    // constructor
    TableMgrUptr Table(string kind);               // returns the table mgr for a kind
    TableMgrUptr CreateTable(string kind, int32_t size); // creates a hash table fo specific size
    static error Init(int32_t *kvstore_root, DelphiShmPtr shm_ptr);      // initializes the shared memory for kv-store
    void         DumpInfo();                       // prints information about the kvstore
private:
    DelphiShmPtr  shm_ptr_;      // reference to slab mgr to allocate memory
    TableMgrUptr  root_table_;   // root table. i.e. hash table index by 'kind'
    int32_t *     kvstore_root_; // pointer to root hash table

    // private methods
    TableMgrUptr createTable(string kind, int32_t size);
};
typedef std::shared_ptr<KvstoreMgr> KvstoreMgrPtr;
typedef std::unique_ptr<KvstoreMgr> KvstoreMgrUptr;

} // namespace shm
} // namespace delphi

#endif // _DELPHI_SHM_KVSTORE_H_
