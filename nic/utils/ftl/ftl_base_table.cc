//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <cmath>
#include "include/sdk/base.hpp"

#include "ftl_table.hpp"
#include "ftl_bucket.hpp"

using sdk::table::ftlint::ftl_base_table;

//---------------------------------------------------------------------------
// ftl_base_table init()
//---------------------------------------------------------------------------
sdk_ret_t
ftl_base_table::init_(uint32_t id, uint32_t size) {
    void *mem;
    uint32_t bucket_size = sizeof(ftl_bucket);

    table_id_ = id;
    table_size_ = size;
    num_table_index_bits_ = log2(size);
    assert(floor(num_table_index_bits_) == ceil(num_table_index_bits_));

    mem = SDK_CALLOC(SDK_MEM_ALLOC_FTL_TABLE_BUCKETS,
                     table_size_ * bucket_size);
    if (mem == NULL) {
        return SDK_RET_OOM;
    }

    //buckets_ = (ftl_bucket *) new(mem) ftl_bucket[table_size_];
    buckets_ = (ftl_bucket *)mem;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl_base_table Destructor
//---------------------------------------------------------------------------
void
ftl_base_table::destroy_(ftl_base_table *table) {
    // Free the Hash table entries.
    SDK_FREE(SDK_MEM_ALLOC_FTL_TABLE_ENTRIES, table->buckets_);
    table->buckets_ = NULL;
}

sdk_ret_t
ftl_base_table::iterate_(ftl_apictx *ctx) {
    ctx->table_id = table_id_;
    for (uint32_t i = 0; i < table_size_; i++) {
        ctx->table_index = i;
        buckets_[i].iterate_(ctx);
    }
    return SDK_RET_OK;
}
