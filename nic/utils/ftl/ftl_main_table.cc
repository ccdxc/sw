//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "gen/p4gen/p4/include/ftl.h"
#include "ftl_includes.hpp"

#define MASK(_nbits) ((1 << (_nbits)) - 1)

//---------------------------------------------------------------------------
// Factory method to instantiate the FTL_MAKE_AFTYPE(main_table) class
//---------------------------------------------------------------------------
FTL_MAKE_AFTYPE(main_table) *
FTL_MAKE_AFTYPE(main_table)::factory(sdk::table::properties_t *props) {
    void *mem = NULL;
    FTL_MAKE_AFTYPE(main_table) *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (FTL_MAKE_AFTYPE(main_table) *) SDK_CALLOC(SDK_MEM_ALLOC_FTL_MAIN_TABLE,
                                          sizeof(FTL_MAKE_AFTYPE(main_table)));
    if (!mem) {
        return NULL;
    }

    table = new (mem) FTL_MAKE_AFTYPE(main_table)();

    ret = table->init_(props);
    if (ret != SDK_RET_OK) {
        table->~FTL_MAKE_AFTYPE(main_table)();
        SDK_FREE(SDK_MEM_ALLOC_FTL_MAIN_TABLE, mem);
    }

    return table;
}

//---------------------------------------------------------------------------
// FTL_MAKE_AFTYPE(main_table) init_()
//---------------------------------------------------------------------------
sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::init_(sdk::table::properties_t *props) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = FTL_MAKE_AFTYPE(base_table)::init_(props->ptable_id, props->ptable_size);

    num_hash_bits_ = 32 - num_table_index_bits_;
    FTL_TRACE_VERBOSE("MainTable: Created FTL_MAKE_AFTYPE(main_table) "
                      "TableID:%d TableSize:%d NumTableIndexBits:%d NumHashBits:%d",
                      table_id_, table_size_, num_table_index_bits_, num_hash_bits_);

    hint_table_ = FTL_MAKE_AFTYPE(hint_table)::factory(props);
    SDK_ASSERT_RETURN(hint_table_, SDK_RET_OOM);

    return ret;
}

void
FTL_MAKE_AFTYPE(main_table)::destroy_(FTL_MAKE_AFTYPE(main_table) *table) {
    FTL_MAKE_AFTYPE(hint_table)::destroy_(table->hint_table_);
    FTL_MAKE_AFTYPE(base_table)::destroy_(table);
}

inline void 
FTL_MAKE_AFTYPE(main_table)::lock_(FTL_MAKE_AFTYPE(apictx) *ctx) {
    buckets_[ctx->table_index].lock_();
}

inline void
FTL_MAKE_AFTYPE(main_table)::unlock_(FTL_MAKE_AFTYPE(apictx) *ctx) {
    buckets_[ctx->table_index].unlock_();
}


sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::initctx_(FTL_MAKE_AFTYPE(apictx) *ctx) {
    // By now, we should have a valid hash value.
    SDK_ASSERT(ctx->params->hash_valid);

    ctx->table_id = table_id_;

    // Derive the table_index
    ctx->table_index = ctx->params->hash_32b % table_size_;
    ctx->hash_msbits = (ctx->params->hash_32b >> num_table_index_bits_) & MASK(num_hash_bits_);
    FTL_TRACE_VERBOSE("M: TID:%d Idx:%d", ctx->table_id, ctx->table_index);
    ctx->bucket = &buckets_[ctx->table_index];

    return ctx->bucket->read_(ctx);
}

//---------------------------------------------------------------------------
// FTL_MAKE_AFTYPE(main_table) insert_: Insert entry to main table
//---------------------------------------------------------------------------
sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::insert_(FTL_MAKE_AFTYPE(apictx) *ctx) {
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

    lock_(ctx);
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

    unlock_(ctx);
    return ret;
}

//---------------------------------------------------------------------------
// FTL_MAKE_AFTYPE(main_table) remove_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::remove_(FTL_MAKE_AFTYPE(apictx) *ctx) {
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
// FTL_MAKE_AFTYPE(main_table) update_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::find_(FTL_MAKE_AFTYPE(apictx) *ctx,
                      FTL_MAKE_AFTYPE(apictx) **match_ctx) {
__label__ done;
    if (!ctx->inited) {
        // If entry_valid, then context is already initialized.
        SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);
    }

    auto ret = ctx->bucket->find_(ctx);
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

sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::update_(FTL_MAKE_AFTYPE(apictx) *ctx) {
__label__ done;
    FTL_MAKE_AFTYPE(apictx) *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    auto ret = find_(ctx, &match_ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "find r:%d", ret);

    ret = match_ctx->bucket->update_(match_ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket update r:%d", ret);

done:
    return ret;
}

sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::get_(FTL_MAKE_AFTYPE(apictx) *ctx) {
__label__ done;
    FTL_MAKE_AFTYPE(apictx) *match_ctx = NULL;

    SDK_ASSERT(initctx_(ctx) == SDK_RET_OK);

    auto ret = find_(ctx, &match_ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "find r:%d", ret);

    // Set the Handle
    if (match_ctx->is_main()) {
        match_ctx->params->handle.pindex(match_ctx->table_index);
        ctx->params->handle.pindex(match_ctx->table_index);
    } else {
        match_ctx->params->handle.sindex(match_ctx->table_index);
        ctx->params->handle.sindex(match_ctx->table_index);
    }

    ctx->entry.copy_key_data((FTL_MAKE_AFTYPE(entry_t)*)match_ctx->params->entry);

done:
    return ret;
}

//---------------------------------------------------------------------------
// FTL_MAKE_AFTYPE(main_table) iterate_: Iterate entries from main table
//---------------------------------------------------------------------------
sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::iterate_(FTL_MAKE_AFTYPE(apictx) *ctx) {
    auto ret = FTL_MAKE_AFTYPE(base_table)::iterate_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "main table iterate r:%d", ret);

    ret = hint_table_->iterate_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "hint table iterate r:%d", ret);
done:
    return SDK_RET_OK;
}

sdk_ret_t
FTL_MAKE_AFTYPE(main_table)::clear_(FTL_MAKE_AFTYPE(apictx) *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (ctx->clear_global_state) {
        ret = FTL_MAKE_AFTYPE(base_table)::clear_(ctx);
        FTL_RET_CHECK_AND_GOTO(ret, done, "main table clear r:%d", ret);

        memclr(ctx->props->ptable_base_mem_va,
               ctx->props->ptable_base_mem_pa,
               ctx->props->ptable_size);
    }

    ret = hint_table_->clear_(ctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "hint table iterate r:%d", ret);

done:
    return SDK_RET_OK;
}
