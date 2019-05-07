//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"

#include "ftl_table.hpp"
#include "ftl_apictx.hpp"
#include "ftl_bucket.hpp"
#include "ftl_utils.hpp"

using sdk::table::ftlint::ftl_base_table;
using sdk::table::ftlint::ftl_hint_table;
using sdk::table::ftlint::ftl_apictx;
using sdk::table::ftlint::ftl_bucket;
using sdk::table::ftlint::ftl_bucket;

//---------------------------------------------------------------------------
// Factory method to instantiate the ftl_main_table class
//---------------------------------------------------------------------------
ftl_hint_table *
ftl_hint_table::factory(sdk::table::properties_t *props) {
    void *mem = NULL;
    ftl_hint_table *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (ftl_hint_table *) SDK_CALLOC(SDK_MEM_ALLOC_FTL_HINT_TABLE,
                                          sizeof(ftl_hint_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) ftl_hint_table();

    ret = table->init_(props);
    if (ret != SDK_RET_OK) {
        table->~ftl_hint_table();
        SDK_FREE(SDK_MEM_ALLOC_FTL_HINT_TABLE, mem);
    }

    return table;
}

//---------------------------------------------------------------------------
// ftl_hint_table init()
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::init_(sdk::table::properties_t *props) {
    auto ret = ftl_base_table::init_(props->stable_id,
                                     props->stable_size);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = indexer_.init(table_size_);
    SDK_ASSERT(ret == SDK_RET_OK);

    // Index 0 cannot be used, so reserve it by doing a dummy alloc.
    uint32_t temp = 0;
    ret = indexer_.alloc(temp);
    SDK_ASSERT(ret == SDK_RET_OK);
    FTL_TRACE_VERBOSE("Created ftl_main_table TableID:%d TableSize:%d "
                      "NumTableIndexBits:%d",
                      table_id_, table_size_, num_table_index_bits_);
    return ret;
}

//---------------------------------------------------------------------------
// ftl_main_table destroy_
//---------------------------------------------------------------------------
void
ftl_hint_table::destroy_(ftl_hint_table *table)
{
    ftl_base_table::destroy_(table);
    table->indexer_.deinit();
}

//---------------------------------------------------------------------------
// ftl_main_table alloc_: Allocate a hint (bucket)
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::alloc_(ftl_apictx *ctx) {
#ifdef PERF
    static uint32_t hint_index = 1;
    ctx->hint = hint_index++;
    return SDK_RET_OK;
#endif
    uint32_t hint = 0;
    auto ret = indexer_.alloc(hint);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    ctx->hint = hint;
    FTL_TRACE_VERBOSE("%s: Allocated index:%d Meta:[%s]",
                      ctx->idstr(), ctx->hint, ctx->metastr());

    // Set write pending
    ctx->write_pending = true;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl_main_table dealloc_: De-Allocate a hint (bucket)
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::dealloc_(ftl_apictx *ctx) {
    indexer_.free(ctx->hint);
    FTL_TRACE_VERBOSE("HintTable: Freed index:%d", ctx->hint);

    // Clear the hint and set write pending
    HINT_SET_INVALID(ctx->hint);
    ctx->write_pending = true;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl_hint_table initctx_ : Initialize API context
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::initctx_(ftl_apictx *ctx) {
    // All HintTable api contexts must have parent api context.
    SDK_ASSERT(ctx->pctx);

    // Make the hint from parent context as table_index
    ctx->table_index = ctx->pctx->hint;
    // Save the table_id
    ctx->table_id = table_id_;

    FTL_TRACE_VERBOSE("%s: TID:%d Idx:%d", ctx->idstr(),
                      ctx->table_id, ctx->table_index);
    return buckets_[ctx->table_index].read_(ctx);
}

//---------------------------------------------------------------------------
// ftl_hint_table insert_: Recursive function
//                              level0 context is main table context
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::insert_(ftl_apictx *pctx) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;
    ftl_apictx *hctx = NULL;

    if (unlikely((uint32_t)(pctx->level+1) >= pctx->props->max_recircs)) {
        FTL_TRACE_ERR("Max Recirc levels reached.");
        return SDK_RET_MAX_RECIRC_EXCEED;
    }

    // If the hint is invalid, allocate a new hint
    if (!HINT_IS_VALID(pctx->hint)) {
        ret = alloc_(pctx);
        FTL_RET_CHECK_AND_GOTO(ret, done, "hint alloc r:%d", ret);
    }

    // Create a new context for this entry from parent context
    hctx = ctxnew_(pctx);
    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    ret = buckets_[hctx->table_index].insert_(hctx);
    if (unlikely(ret == SDK_RET_COLLISION)) {
        // Recursively call the insert_ with hint table context
        ret = insert_(hctx);
    }

    // Write to hardware
    if (ret == SDK_RET_OK) {
        ret = buckets_[hctx->table_index].write_(hctx);
    }

done:
    return ret;
}

//---------------------------------------------------------------------------
// ftl_hint_table tail_: Finds the tail node of a hint chain
//                            and return the context
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::tail_(ftl_apictx *ctx,
                           ftl_apictx **retctx) {
    if (!HINT_IS_VALID(ctx->hint)) {
        FTL_TRACE_VERBOSE("No hints, setting TAIL = EXACT.");
        *retctx = ctx;
        return SDK_RET_OK;
    }

    auto tctx = ctxnew_(ctx);
    if (tctx == NULL) {
        FTL_TRACE_ERR("failed to create api context");
        return SDK_RET_OOM;
    }

    if (tctx->is_max_recircs()) {
        FTL_TRACE_ERR("Max Recirc levels reached.");
        // We should have caught this in insert.
        SDK_ASSERT(0);
        return SDK_RET_MAX_RECIRC_EXCEED;
    }

    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(tctx) == SDK_RET_OK, SDK_RET_ERR);

    auto ret = buckets_[tctx->table_index].find_last_hint_(tctx);
    if (ret == SDK_RET_OK) {
        FTL_TRACE_VERBOSE("%s: find_last_hint_ Ctx: [%s]", tctx->idstr(),
                              tctx->metastr(), ret);
        // We have found a valid hint, recursively find the tail of this node
        ret = tail_(tctx, retctx);;
        // NOTE: We are not freeing any context right now, as it gets
        // very complicated especially for smaller chains.
        // Instead it can be freed cleanly by traversing in reverse from the
        // tail context after the processing is done.
    } else if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        // No valid hint in this bucket, this itself is the tail
        *retctx = tctx;
        ret = SDK_RET_OK;
    } else {
        *retctx = NULL;
        FTL_TRACE_ERR("%s: find_last_hint_ failed ret:%d", tctx->idstr(), ret);
        // failure case: destroy the context
    }

    return ret;
}

//---------------------------------------------------------------------------
// ftl_hint_table defragment_: Defragment the hint chain
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::defragment_(ftl_apictx *ectx) {
    sdk_ret_t ret = SDK_RET_OK;
    ftl_apictx *tctx = NULL;
    ftl_apictx *tempctx = NULL;

    // Defragmentation Sequence
    // vars used in this function:
    //  - ectx: exact match context
    //  - tctx: tail node context
    //  - pctx: parent node context (parent of tail node)
    //
    // Special cases:
    //  - If the chain is only 1 level, then ectx == pctx
    //  - If we are deleting tail, then ectx == tctx
    //
    // Steps:
    // 1) Find 'tctx' and it will give 'pctx'
    // 2) Copy the tail node key and data to 'ectx'
    // 3) Write 'ectx' (make before break)
    // 4) Delete link from 'pctx' to 'tctx'
    // 5) Write 'pctx' (make before break)
    // 6) Delete 'tctx'

    FTL_TRACE_VERBOSE("%s: Starting defragmentation, Ctx: [%s]",
                          ectx->idstr(), ectx->metastr());
    // Get tail node context
    ret = tail_(ectx, &tctx);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("defragment_ failed ret:%d", ret);
        return ret;
    }

    ret = buckets_[ectx->table_index].defragment_(ectx, tctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    SDK_ASSERT(tctx);

    // Destroy the all the api contexts of this chain
    if (tctx != ectx) {
        // We have to destroy only the chain built by the tail traversal
        // when etcx == tctx, all the parent contexts will be freed when
        // the recursive stack unwinds.
        tempctx = tctx->pctx;
        while (tctx && tctx != ectx) {
            tctx = tempctx;
            tempctx = tctx->pctx;
        }
    }

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl_hint_table remove_: Remove entry from hint table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::remove_(ftl_apictx *ctx) {
__label__ done;
    // Initialize the context
    auto hctx = ctxnew_(ctx);
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);
    // Remove entry from the bucket
    auto ret = buckets_[hctx->table_index].remove_(hctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket remove r:%d", ret);

    if (hctx->exmatch) {
        // This means there was an exact match in the hint table and
        // it was removed. Check and defragment the hints if required.
        ret = defragment_(hctx);
    } else {
        // We only found a matching hint, so remove the entry recursively
        ret = remove_(hctx);
    }

done:
    return ret;
}

//---------------------------------------------------------------------------
// ftl_hint_table remove_: Remove entry from hint table
//---------------------------------------------------------------------------
sdk_ret_t
ftl_hint_table::find_(ftl_apictx *ctx,
                      ftl_apictx **match_ctx) {
__label__ done;
    // Initialize the context
    auto hctx = ctxnew_(ctx);
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    // Remove entry from the bucket
    auto ret = buckets_[hctx->table_index].find_(hctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket find r:%d", ret);

    if (hctx->exmatch) {
        // This means there was an exact match in the hint table
        *match_ctx = hctx;
        return SDK_RET_OK;
    } else {
        // We only found a matching hint, so find the entry recursively
        ret = find_(hctx, match_ctx);
    }

    SDK_ASSERT(*match_ctx != hctx);
done:
    return ret;
}
