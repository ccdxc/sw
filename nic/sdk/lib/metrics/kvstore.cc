// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

// this file contains shared memory based kvstore API

#include <memory>
#include <string.h>

#include "kvstore.hpp"
#include "shm.hpp"

namespace sdk {
namespace metrics {

// KvstoreMgr constructor
KvstoreMgr::KvstoreMgr(int32_t *kvstore_root,  ShmPtr shm_ptr) {
    // save shm ptr for later use
    shm_ptr_      = shm_ptr;
    kvstore_root_ = kvstore_root;

    // get the kvstore root
    assert(*kvstore_root != 0);
    htable_t *root_ht = (htable_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), *kvstore_root);
    root_table_ = std::unique_ptr<TableMgr>(new TableMgr(root_ht, shm_ptr));
}

// Init initializes kvstore root
error KvstoreMgr::Init(int32_t *kvstore_root, ShmPtr shm_ptr) {
    // allocate memory for base hash table
    int mem_size = (MAX_SEQ_BUCKETS * sizeof(ht_bucket_t)) + sizeof(htable_t);
    void *tptr = shm_ptr->Alloc(mem_size);
    if (tptr == NULL) {
        //LogError("Error allocating space in shared memory");
        return nullptr;
    }

    // initialize the table
    htable_t *ht = (htable_t *)tptr;
    strncpy(ht->tbl_name, "root", TABLE_NAME_LEN);
    ht->num_buckets    = MAX_SEQ_BUCKETS;
    ht->ht_flags       = 0;
    ht->num_entries    = 0;
    ht->num_tablets    = 0;
    ht->num_buckets_per_tablet = 0;
    ht->num_used_buckets       = 0;

    // initialize the buckets to empty
    memset((void *)&ht->buckets[0], 0, (MAX_SEQ_BUCKETS * sizeof(ht_bucket_t)));
    for (int i = 0; i < MAX_SEQ_BUCKETS; i++) {
        spin_lock_init(&ht->buckets[i].rw_lock);
    }

    // write the root offset
    *kvstore_root = OFFSET_FROM_PTR(shm_ptr->GetBase(), ht);

    return error::OK();
}

// Table gets a table for a kind, it creates one if it doesnt exist
TableMgrUptr KvstoreMgr::Table(std::string kind) {
    // some error checks
    if (kind == "") {
        //LogError("kind can not be empty");
        return nullptr;
    }

    // check if the kind already exists in the base ht
    void *tptr = root_table_->Find(kind.c_str(), kind.length());
    if (tptr == NULL) {
        //LogError("Error finding the table for kind {}", kind);
        return nullptr;
    }

    // table already exists, just return it
    htable_t *ht = (htable_t *)tptr;
    return std::unique_ptr<TableMgr>(new TableMgr(ht, shm_ptr_));
}

// CreateTable creates a table of specified size
TableMgrUptr KvstoreMgr::CreateTable(std::string kind, int32_t size) {
    return createTable(kind, size);
}

// DumpInfo prints information about kvstore
void KvstoreMgr::DumpInfo() {
    htable_t *root_ht = (htable_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), *kvstore_root_);

    printf("Kvstore info:\n");
    printf("Num root buckets: %d, Num tables: %d, used buckets: %d\n",
        root_ht->num_buckets, root_ht->num_entries, root_ht->num_used_buckets);

    // dump each table
    if (root_ht->num_entries > 0) {
        printf("Tables:\n");
        for (int i = 0; i < root_ht->num_buckets; i++) {
            if (root_ht->buckets[i].ht_entry != 0) {
                ht_entry_t *entry = (ht_entry_t *)PTR_FROM_OFFSET(shm_ptr_->GetBase(), root_ht->buckets[i].ht_entry);
                htable_t *ht = (htable_t *)VAL_PTR_FROM_HASH_ENTRY(entry);

                printf("  %d/0x%x. Kind: %s, buckets: %d, flags: %x, tablets: %d, entries: %d, used buckets: %d\n",
                    i, root_ht->buckets[i].ht_entry, ht->tbl_name, ht->num_buckets,
                    ht->ht_flags, ht->num_tablets, ht->num_entries, ht->num_used_buckets);
            }
        }
    }
}

// createTable creates a table in kv-store
TableMgrUptr KvstoreMgr::createTable(std::string kind, int32_t size) {
    TableMgrUptr table_ptr;

    // some error checks
    if (kind == "") {
        //LogError("kind can not be empty");
        return nullptr;
    }

    // check if the kind already exists in the base ht
    void *tptr = root_table_->Find(kind.c_str(), kind.length());
    if (tptr == NULL) {
        // determine if this is one level table or two level table
        if (size <= MAX_SEQ_BUCKETS) {
            // allocate buckets inline with hash table
            int mem_size = (size * sizeof(ht_bucket_t)) + sizeof(htable_t);
            tptr = root_table_->Create(kind.c_str(), kind.length(), mem_size);
            if (tptr == NULL) {
                //LogError("Error allocating space in shared memory");
                return nullptr;
            }

            // initialize the table
            htable_t *ht = (htable_t *)tptr;
            strncpy(ht->tbl_name, kind.c_str(), TABLE_NAME_LEN);
            spin_lock_init(&ht->rw_lock);
            ht->num_buckets    = size;
            ht->ht_flags       = 0;
            ht->num_entries    = 0;
            ht->num_tablets    = 0;
            ht->num_buckets_per_tablet = 0;

            // initialize the buckets to empty
            memset((void *)&ht->buckets[0], 0, (size * sizeof(ht_bucket_t)));
            for (int i = 0; i < size; i++) {
                spin_lock_init(&ht->buckets[i].rw_lock);
            }

            //LogInfo("Created hash table for kind {} at 0x{}", kind, OFFSET_FROM_PTR(shm_ptr_->GetBase(), ht));

            // create a table mgr instance
            table_ptr = std::unique_ptr<TableMgr>(new TableMgr(ht, shm_ptr_));
        } else {
            // calculate number of tablets required
            int num_tablets = size / MAX_SEQ_BUCKETS;
            if ((size % MAX_SEQ_BUCKETS) != 0) {
                num_tablets++;
            }

            // allocate memory for hash table
            int mem_size = (num_tablets * sizeof(ht_bucket_t)) + sizeof(htable_t);
            tptr = root_table_->Create(kind.c_str(), kind.length(), mem_size);
            if (tptr == NULL) {
                //LogError("Error allocating space in shared memory");
                return nullptr;
            }

            // initialize the hash table
            htable_t *ht = (htable_t *)tptr;
            strncpy(ht->tbl_name, kind.c_str(), TABLE_NAME_LEN);
            spin_lock_init(&ht->rw_lock);
            ht->num_buckets    = num_tablets * MAX_SEQ_BUCKETS;
            ht->ht_flags       = HTABLE_FLAG_TWO_LEVEL;
            ht->num_entries    = 0;
            ht->num_tablets    = num_tablets;
            ht->num_buckets_per_tablet = MAX_SEQ_BUCKETS;

            // initialize each tablet
            for (int i = 0; i < num_tablets; i++) {
                tptr = shm_ptr_->Alloc(sizeof(ht_tablet_t));
                if (tptr == NULL) {
                    //LogError("Error allocating space in shared memory");
                    shm_ptr_->Free((void *)ht);  // free the hash table
                    // FIXME: free all the other tablets we've allocated too
                    return nullptr;
                }

                // add the tablet to hash table
                ht->buckets[i].ht_entry = OFFSET_FROM_PTR(shm_ptr_->GetBase(), tptr);

                // initialize all  buckets in the tablet to empty
                memset(tptr, 0, (MAX_SEQ_BUCKETS * sizeof(ht_bucket_t)));
            }

            //LogInfo("Created two level hash table for kind {} at 0x{}", kind, OFFSET_FROM_PTR(shm_ptr_->GetBase(), ht));

            // create a table mgr instance
            table_ptr = std::unique_ptr<TableMgr>(new TableMgr(ht, shm_ptr_));
        }
    } else {
        // table already exists, just return it
        htable_t *ht = (htable_t *)tptr;
        table_ptr = std::unique_ptr<TableMgr>(new TableMgr(ht, shm_ptr_));
    }

    return table_ptr;
}

} // namespace metrics
} // namespace sdk
