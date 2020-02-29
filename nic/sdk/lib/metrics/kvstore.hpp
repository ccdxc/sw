// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#ifndef _SDK_METRICS_SHM_KVSTORE_H_
#define _SDK_METRICS_SHM_KVSTORE_H_

#include <string>

#include "error.hpp"
#include "htable.hpp"
#include "shm_types.hpp"

namespace sdk {
namespace metrics {

// KvstoreMgr manages a shared memory key-value store.
// kvstore is maintained in shared memory as two levels of hash table.
// first level hash table is indexed by 'kind' and each entry is another hash table
// second level hash table is indexed by the 'key' and each entry stores the 'value'
class KvstoreMgr {
public:
    KvstoreMgr(int32_t *kvstore_root, ShmPtr shm_ptr);    // constructor
    TableMgrUptr Table(std::string kind);               // returns the table mgr for a kind
    TableMgrUptr CreateTable(std::string kind, int32_t size); // creates a hash table of specific size
    static error Init(int32_t *kvstore_root, ShmPtr shm_ptr);      // initializes the shared memory for kv-store
    void         DumpInfo();                       // prints information about the kvstore
private:
    ShmPtr       shm_ptr_;      // reference to slab mgr to allocate memory
    TableMgrUptr root_table_;   // root table. i.e. hash table index by 'kind'
    int32_t *    kvstore_root_; // pointer to root hash table

    // private methods
    TableMgrUptr createTable(std::string kind, int32_t size);
};
typedef std::shared_ptr<KvstoreMgr> KvstoreMgrPtr;
typedef std::unique_ptr<KvstoreMgr> KvstoreMgrUptr;

} // namespace metrics
} // namespace sdk

#endif // _SDK_METRICS_SHM_KVSTORE_H_
