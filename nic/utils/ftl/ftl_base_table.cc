//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "gen/p4gen/p4/include/ftl.h"
#include "ftl_includes.hpp"

sdk_ret_t
base_table::init_(uint32_t id, uint32_t size) {
    void *mem;
    uint32_t bucket_size = sizeof(Bucket);

    table_id_ = id;
    table_size_ = size;
    num_table_index_bits_ = log2(size);
    assert(floor(num_table_index_bits_) == ceil(num_table_index_bits_));

    mem = SDK_CALLOC(SDK_MEM_ALLOC_FTL_TABLE_BUCKETS,
                     table_size_ * bucket_size);
    if (mem == NULL) {
        return SDK_RET_OOM;
    }

    //buckets_ = (Bucket *) new(mem) Bucket[table_size_];
    buckets_ = (Bucket *)mem;
    return SDK_RET_OK;
}

void
base_table::destroy_(base_table *table) {
    // Free the Hash table entries.
    SDK_FREE(SDK_MEM_ALLOC_FTL_TABLE_ENTRIES, table->buckets_);
    table->buckets_ = NULL;
}

sdk_ret_t
base_table::iterate_(Apictx *ctx) {
    uint32_t i = (ctx->is_main()) ? 0 : 1;

    ctx->table_id = table_id_;
    for ( ; i < table_size_; i++) {
        ctx->table_index = i;
        buckets_[i].iterate_(ctx);
    }
    return SDK_RET_OK;
}

sdk_ret_t
base_table::clear_(Apictx *ctx) {
    memset(buckets_, 0, table_size_ * sizeof( Bucket));
    return SDK_RET_OK;
}

