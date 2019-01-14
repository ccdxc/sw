//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <cmath>
#include "include/sdk/base.hpp"

#include "mem_hash_table.hpp"
#include "mem_hash_table_bucket.hpp"

using sdk::table::memhash::mem_hash_base_table;

//---------------------------------------------------------------------------
// mem_hash_base_table init()
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_base_table::init_(uint32_t id, uint32_t size) {
    void *mem;
    uint32_t bucket_size = sizeof(mem_hash_table_bucket);

    table_id_ = id;
    table_size_ = size;
    num_table_index_bits_ = log2(size);
    assert(floor(num_table_index_bits_) == ceil(num_table_index_bits_));

    mem = SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_TABLE_BUCKETS,
                     table_size_ * bucket_size);
    if (mem == NULL) {
        return SDK_RET_OOM;
    }

    buckets_ = (mem_hash_table_bucket *) new(mem) mem_hash_table_bucket[table_size_];
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_base_table Destructor
//---------------------------------------------------------------------------
void
mem_hash_base_table::destroy_(mem_hash_base_table *table) {
    // Free the Hash table entries.
    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_TABLE_ENTRIES, table->buckets_);
    table->buckets_ = NULL;
}
