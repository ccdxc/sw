//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>
#include "include/sdk/base.hpp"

#include "ftl_table.hpp"
#include "ftl_apictx.hpp"
#include "ftl_utils.hpp"

using sdk::table::ftlint::ftl_base_table;
using sdk::table::ftlint::ftl_main_table;
using sdk::table::ftlint::ftl_apictx;

#define MASK(_nbits) ((1 << (_nbits)) - 1)

//---------------------------------------------------------------------------
// Factory method to instantiate the ftl_main_table class
//---------------------------------------------------------------------------
ftl_main_table *
ftl_main_table::factory(sdk::table::properties_t *props) {
    void *mem = NULL;
    ftl_main_table *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (ftl_main_table *) SDK_CALLOC(SDK_MEM_ALLOC_FTL_MAIN_TABLE,
                                          sizeof(ftl_main_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) ftl_main_table();

    ret = table->init_(props);
    if (ret != SDK_RET_OK) {
        table->~ftl_main_table();
        SDK_FREE(SDK_MEM_ALLOC_FTL_MAIN_TABLE, mem);
    }

    return table;
}

//---------------------------------------------------------------------------
// ftl_main_table init_()
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::init_(sdk::table::properties_t *props) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = ftl_base_table::init_(props->ptable_id,
                                     props->ptable_size);

    num_hash_bits_ = 32 - num_table_index_bits_;
    FTL_TRACE_VERBOSE("MainTable: Created ftl_main_table "
                      "TableID:%d TableSize:%d NumTableIndexBits:%d NumHashBits:%d",
                      table_id_, table_size_, num_table_index_bits_, num_hash_bits_);

    hint_table_ = ftl_hint_table::factory(props);
    SDK_ASSERT_RETURN(hint_table_, SDK_RET_OOM);

    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table destroy_
//---------------------------------------------------------------------------
void
ftl_main_table::destroy_(ftl_main_table *table) {
    ftl_hint_table::destroy_(table->hint_table_);
    ftl_base_table::destroy_(table);
}

//---------------------------------------------------------------------------
// ftl_main_table initctx_ : Initialize API context
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::initctx_(ftl_apictx *ctx) {
    // By now, we should have a valid hash value.
    SDK_ASSERT(ctx->params->hash_valid);

    ctx->table_id = table_id_;

    // Derive the table_index
    ctx->table_index = ctx->params->hash_32b % table_size_;
    ctx->hash_msbits = (ctx->params->hash_32b >> num_table_index_bits_) & MASK(num_hash_bits_);
    FTL_TRACE_VERBOSE("M: TID:%d Idx:%d", ctx->table_id, ctx->table_index);

    return buckets_[ctx->table_index].read_(ctx);
}

//---------------------------------------------------------------------------
// ftl_main_table insert_: Insert entry to main table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::insert_(ftl_apictx *ctx) {
    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

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

    auto ret = buckets_[ctx->table_index].insert_(ctx);
    if (unlikely(ret == SDK_RET_COLLISION)) {
        // COLLISION case
        ret = hint_table_->insert_(ctx);
    }

    if (likely(ret == SDK_RET_OK)) {
        // 2 CASES:
        // CASE 1: Write to HW only if this is a terminal node.
        //
        // CASE 2: In case of collision, after the downstream nodes are
        //         written. we can update the main entry. This will ensure
        //         make before break for any downstream changes.
        ret = buckets_[ctx->table_index].write_(ctx);
        ctx->params->handle.pindex(ctx->table_index);
    } else {
        FTL_TRACE_ERR("MainTable: insert failed: ret:%d", ret);
    }

    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table remove_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::remove_(ftl_apictx *ctx) {
__label__ done;
    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    auto ret = buckets_[ctx->table_index].remove_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket remove r:%d", ret);

    SDK_ASSERT(ctx->match);
    if (ctx->exmatch) {
        // This means there was an exact match in the main table and
        // it was removed. Check and defragment the hints if required.
        if (ctx->hint) {
            ret = hint_table_->defragment_(ctx);
            FTL_RET_CHECK_AND_GOTO(ret, done, "defragment r:%d", ret);
        }
    } else {
        // We have a hint match, traverse the hints to remove the entry.
        ret = hint_table_->remove_(ctx);
        FTL_RET_CHECK_AND_GOTO(ret, done, "hint table remove r:%d", ret);
    }

done:
    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::find_(ftl_apictx *ctx,
                      ftl_apictx **match_ctx) {
__label__ done;
    if (!ctx->inited) {
        // If entry_valid, then context is already initialized.
        SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);
    }

    auto ret = buckets_[ctx->table_index].find_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket find r:%d", ret);

    if (ctx->exmatch) {
        // This means there was an exact match in the main table
        *match_ctx = ctx;
        return SDK_RET_OK;
    } else {
        // We have a hint match, traverse the hints to find a the entry.
        ret = hint_table_->find_(ctx, match_ctx);
        FTL_RET_CHECK_AND_GOTO(ret, done, "hint table find r:%d", ret);
    }

done:
    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::update_(ftl_apictx *ctx) {
__label__ done;
    ftl_apictx *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    auto ret = find_(ctx, &match_ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "find r:%d", ret);

    ret = buckets_[match_ctx->table_index].update_(match_ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket update r:%d", ret);

done:
    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::get_(ftl_apictx *ctx) {
__label__ done;
    ftl_apictx *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    auto ret = find_(ctx, &match_ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "find r:%d", ret);

    // Set the Handle
    if (FTL_API_CONTEXT_IS_MAIN(match_ctx)) {
        match_ctx->params->handle.pindex(match_ctx->table_index);
    } else {
        match_ctx->params->handle.sindex(match_ctx->table_index);
    }

    FTL_ENTRY_COPY_KEY_DATA((ftl_entry_t*)ctx->params->entry, &ctx->entry);

done:
    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table iterate_: Iterate entries from main table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_main_table::iterate_(ftl_apictx *ctx) {
    auto ret = ftl_base_table::iterate_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "main table iterate r:%d", ret);

    ret = hint_table_->iterate_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "hint table iterate r:%d", ret);
done:
    return SDK_RET_OK;
}
