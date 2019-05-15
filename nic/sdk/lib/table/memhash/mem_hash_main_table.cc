//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>
#include "include/sdk/base.hpp"

#include "mem_hash_table.hpp"
#include "mem_hash_api_context.hpp"
#include "mem_hash_utils.hpp"

using sdk::table::memhash::mem_hash_base_table;
using sdk::table::memhash::mem_hash_main_table;
using sdk::table::memhash::mem_hash_api_context;

#define MASK(_nbits) ((1 << (_nbits)) - 1)

//---------------------------------------------------------------------------
// Factory method to instantiate the mem_hash_main_table class
//---------------------------------------------------------------------------
mem_hash_main_table *
mem_hash_main_table::factory(sdk::table::properties_t *props) {
    void *mem = NULL;
    mem_hash_main_table *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (mem_hash_main_table *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_MAIN_TABLE,
                                          sizeof(mem_hash_main_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) mem_hash_main_table();

    ret = table->init_(props);
    if (ret != SDK_RET_OK) {
        table->~mem_hash_main_table();
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_MAIN_TABLE, mem);
    }

    return table;
}

//---------------------------------------------------------------------------
// mem_hash_main_table init_()
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::init_(sdk::table::properties_t *props) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = mem_hash_base_table::init_(props->ptable_id,
                                     props->ptable_size);

    num_hash_bits_ = 32 - num_table_index_bits_;
    MEMHASH_TRACE_VERBOSE("MainTable: Created mem_hash_main_table "
                          "TableID:%d TableSize:%d NumTableIndexBits:%d NumHashBits:%d",
                          table_id_, table_size_, num_table_index_bits_, num_hash_bits_);

    hint_table_ = mem_hash_hint_table::factory(props);
    SDK_ASSERT_RETURN(hint_table_, SDK_RET_OOM);

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table destroy_
//---------------------------------------------------------------------------
void
mem_hash_main_table::destroy_(mem_hash_main_table *table) {
    mem_hash_hint_table::destroy_(table->hint_table_);
    mem_hash_base_table::destroy_(table);
    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_MAIN_TABLE, table);
}

//---------------------------------------------------------------------------
// mem_hash_main_table initctx_ : Initialize API context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::initctx_(mem_hash_api_context *ctx) {
    // By now, we should have a valid hash value.
    SDK_ASSERT(ctx->params->hash_valid);

    ctx->table_id = table_id_;

    // Derive the table_index
    ctx->table_index = ctx->params->hash_32b % table_size_;
    ctx->hash_msbits = (ctx->params->hash_32b >> num_table_index_bits_) & MASK(num_hash_bits_);
    ctx->bucket = &buckets_[ctx->table_index];
    SDK_ASSERT(ctx->bucket);

    MEMHASH_TRACE_VERBOSE("M: TID:%d Idx:%d", ctx->table_id, ctx->table_index);

    return static_cast<mem_hash_table_bucket*>(ctx->bucket)->read_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_main_table insert_with_handle_: Insert entry to main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::insert_with_handle_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    SDK_ASSERT(ctx->handle->pvalid());
    SDK_ASSERT(ctx->table_index == ctx->handle->pindex());
    if (!ctx->handle->svalid()) {
        // This handle is for the main table.
        // Write key and data to the hardware.
        ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->insert_with_handle_(ctx);
    } else {
        ctx->hint = ctx->handle->sindex();
        ret = hint_table_->insert_(ctx);
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table insert_: Insert entry to main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::insert_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    // If handle is valid, insert directly using the handle.
    if (ctx->handle->valid()) {
        return insert_with_handle_(ctx);
    }

    // INSERT SEQUENCE:
    // 1) Insert to Main Table
    // 2) If COLLISION:
    //      2.1) Call Hint Table insert api
    //          2.1.1) Insert to Hint Table
    //          2.1.2) If COLLISION:
    //              2.1.2.1) Recursive call to (2.1)
    //          2.1.3) If SUCCESS, write Hint Table bucket to HW
    //      2.2) If Hint Table insert is Successful,
    //           Write the Main Table bucket to HW
    // 3) Else if SUCCESS, insert is complete.

    ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->insert_(ctx);
    if (ret == SDK_RET_COLLISION) {
        // COLLISION case
        ret = hint_table_->insert_(ctx);
    }

    if (ret == SDK_RET_OK) {
        // 2 CASES:
        // CASE 1: Write to HW only if this is a terminal node.
        //
        // CASE 2: In case of collision, after the downstream nodes are
        //         written. we can update the main entry. This will ensure
        //         make before break for any downstream changes.
        ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->write_(ctx);
        ctx->handle->pindex(ctx->table_index);
    } else {
        MEMHASH_TRACE_ERR("MainTable: insert failed: ret:%d", ret);
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table remove_with_handle_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::remove_with_handle_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    SDK_ASSERT(ctx->handle->pvalid());
    SDK_ASSERT(ctx->table_index == ctx->handle->pindex());
    if (!ctx->handle->svalid()) {
        // This handle is for the main table.
        // Write key and data to the hardware.
        MEMHASH_TRACE_VERBOSE("writing to main table.");
        ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->remove_with_handle_(ctx);
    } else {
        ctx->hint = ctx->handle->sindex();
        MEMHASH_TRACE_VERBOSE("removing from hint table, hint=%d", ctx->hint);
        ret = hint_table_->remove_(ctx);
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table remove_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::remove_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    // If handle is valid, insert directly using the handle.
    if (ctx->handle->valid()) {
        return remove_with_handle_(ctx);
    }

    ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->remove_(ctx);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    SDK_ASSERT(ctx->match_type);

    if (ctx->is_exact_match()) {
        // This means there was an exact match in the main table and
        // it was removed. Check and defragment the hints if required.
        if (ctx->is_hint_valid()) {
            ret = hint_table_->defragment_(ctx);
            if (ret != SDK_RET_OK) {
                MEMHASH_TRACE_ERR("defragment_ failed, ret:%d", ret);
            }
        }
    } else {
        // We have a hint match, traverse the hints to remove the entry.
        ret = hint_table_->remove_(ctx);
        if (ret != SDK_RET_OK) {
            MEMHASH_TRACE_ERR("hint table remove_ failed, ret:%d", ret);
        }
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::find_(mem_hash_api_context *ctx,
                           mem_hash_api_context **match_ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (!ctx->sw_valid) {
        // If sw_valid, then context is already initialized.
        SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);
    }

    ret = static_cast<mem_hash_table_bucket*>(ctx->bucket)->find_(ctx);
    if (ret != SDK_RET_OK) {
         MEMHASH_TRACE_ERR("find_ failed, ret:%d", ret);
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
            MEMHASH_TRACE_ERR("find_ failed, ret:%d", ret);
        }
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::update_(mem_hash_api_context *ctx) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    ret = find_(ctx, &match_ctx);
    if (ret != SDK_RET_OK) {
        goto done;
    }

    ret = static_cast<mem_hash_table_bucket*>(match_ctx->bucket)->update_(match_ctx);
    if (ret != SDK_RET_OK) {
        goto done;
    }

done:
    if (match_ctx && match_ctx != ctx) {
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::get_(mem_hash_api_context *ctx) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    ret = find_(ctx, &match_ctx);
    if (ret != SDK_RET_OK) {
        goto done;
    }

    // Set the Handle
    if (MEMHASH_API_CONTEXT_IS_MAIN(match_ctx)) {
        match_ctx->handle->pindex(match_ctx->table_index);
        ctx->params->handle.pindex(match_ctx->table_index);
    } else {
        match_ctx->handle->sindex(match_ctx->table_index);
        ctx->params->handle.sindex(match_ctx->table_index);
    }

    memcpy(ctx->params->appdata, match_ctx->sw_appdata, ctx->props->swappdata_len);

done:
    if (match_ctx && match_ctx != ctx) {
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table iterate_: Iterate entries from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_main_table::iterate_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = mem_hash_base_table::iterate_(ctx);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("main table iteration failed. ret:%d", ret);
        return ret;
    }

    ret = hint_table_->iterate_(ctx);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("hint table iteration failed. ret:%d", ret);
        return ret;
    }

    return SDK_RET_OK;
}
