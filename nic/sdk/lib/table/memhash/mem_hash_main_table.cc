//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"

#include "mem_hash_table.hpp"
#include "mem_hash_api_context.hpp"

using sdk::table::memhash::mem_hash_base_table;
using sdk::table::memhash::mem_hash_main_table;
using sdk::table::memhash::mem_hash_api_context;

#define MASK(_nbits) ((1 << (_nbits)) - 1)

//---------------------------------------------------------------------------
// Factory method to instantiate the mem_hash_main_table class
//---------------------------------------------------------------------------
mem_hash_main_table *
mem_hash_main_table::factory(uint32_t id, uint32_t size, void *hint_table) {
    void *mem = NULL;
    mem_hash_main_table *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (mem_hash_main_table *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_MAIN_TABLE,
                                          sizeof(mem_hash_main_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) mem_hash_main_table();

    ret = table->init_(id, size, hint_table);
    if (ret != SDK_RET_OK) {
        table->~mem_hash_main_table();
        SDK_FREE(SDK_MEM_ALLOC_mem_HASH_MAIN_TABLE, mem);
    }

    return table;
}

//---------------------------------------------------------------------------
// mem_hash_main_table init_()
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::init_(uint32_t id, uint32_t size, void *hint_table) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = mem_hash_base_table::init_(id, size);

    num_hash_bits_ = 32 - num_table_index_bits_;
    SDK_TRACE_DEBUG("MainTable: Created mem_hash_main_table "
                    "TableID:%d TableSize:%d NumTableIndexBits:%d NumHashBits:%d",
                    table_id_, table_size_, num_table_index_bits_, num_hash_bits_);

    hint_table_ = static_cast<mem_hash_hint_table*>(hint_table);

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table destroy_
//---------------------------------------------------------------------------
void
mem_hash_main_table::destroy_(mem_hash_main_table *table) {
    mem_hash_base_table::destroy_(table);
}

//---------------------------------------------------------------------------
// mem_hash_main_table initctx_ : Initialize API context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::initctx_(mem_hash_api_context *ctx) {
    // By now, we should have a valid hash value.
    SDK_ASSERT(ctx->hash_valid);

    ctx->table_id = table_id_;

    // Derive the table_index
    ctx->table_index = ctx->hash_32b % table_size_;
    ctx->hash_msbits = (ctx->hash_32b >> num_table_index_bits_) & MASK(num_hash_bits_);
    ctx->bucket = &buckets_[ctx->table_index];
    SDK_ASSERT(ctx->bucket);

    SDK_TRACE_DEBUG("MainTable: TableID:%d Index:%d",
                    ctx->table_id, ctx->table_index);

    return static_cast<mem_hash_table_bucket*>(ctx->bucket)->read_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_main_table insert_: Insert entry to main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::insert_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);
    ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->insert_(ctx);

    if (ret == SDK_RET_OK) {
        // Write to HW only if this is a terminal node.
        // In case of collision, write will be called after
        // the downstream nodes are written.
        ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->write_(ctx);
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table remove_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::remove_(mem_hash_api_context *ctx) {
    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);
    return static_cast<mem_hash_table_bucket*>(ctx->bucket)->remove_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::find_(mem_hash_api_context *ctx,
                           mem_hash_api_context **match_ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (!ctx->hw_valid) {
        // If hw_valid, then context is already initialized.
        SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);
    }

    ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->find_(ctx);
    if (ret != SDK_RET_OK) {
         SDK_TRACE_DEBUG("find_ failed, ret:%d", ret);
         return ret;
    }

    if (ctx->is_exact_match()) {
        // This means there was an exact match in the main table
        *match_ctx = ctx;
        return SDK_RET_OK;
    } else {
        // We have a hint match, traverse the hints to find a the entry.
        ret = hint_table_->find_(ctx, match_ctx);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_DEBUG("find_ failed, ret:%d", ret);
        }
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::update_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    ret = find_(ctx, &match_ctx);
    if (ret != SDK_RET_OK) {
        goto update_return;
    }

    ret = static_cast<mem_hash_table_bucket*>(match_ctx->bucket)->update_(match_ctx);
    if (ret != SDK_RET_OK) {
        goto update_return;
    }

update_return:
    if (match_ctx && match_ctx != ctx) {
        mem_hash_api_context::destroy(match_ctx);
    }

    return ret;
}
