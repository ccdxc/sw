//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "ftl_base.hpp"
#include "ftl_includes.hpp"

thread_local uint8_t hint_table::nctx_ = 0;

hint_table *
hint_table::factory(sdk::table::properties_t *props) {
    void *mem = NULL;
    hint_table *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (hint_table *) SDK_CALLOC(SDK_MEM_ALLOC_FTL_HINT_TABLE,
                                          sizeof(hint_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) hint_table();

    ret = table->init_(props);
    if (ret != SDK_RET_OK) {
        table->~hint_table();
        SDK_FREE(SDK_MEM_ALLOC_FTL_HINT_TABLE, mem);
    }

    return table;
}

sdk_ret_t
hint_table::init_(sdk::table::properties_t *props) {
    auto ret = base_table::init_(props->stable_id,
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
    FTL_TRACE_VERBOSE("Created hint_table TableID:%d TableSize:%d "
                      "NumTableIndexBits:%d",
                      table_id_, table_size_, num_table_index_bits_);

    for (auto i = 0; i < PDS_FLOW_HINT_POOLS_MAX; i++) {
        thr_local_pools_[i].pool_count = -1;
    }
    return ret;
}

void
hint_table::destroy_(hint_table *table)
{
    base_table::destroy_(table);
    table->indexer_.deinit();
    SDK_FREE(SDK_MEM_ALLOC_FTL_HINT_TABLE, table);
}

inline Apictx *
hint_table::ctxnew_(Apictx *src) {
    if (src->is_main()) {
        nctx_ = 0;
    } else {
        SDK_ASSERT(nctx_ < FTL_MAX_API_CONTEXTS);
    }
    auto c = src + 1;
    sdk::table::ftl_base *ftlbase = (sdk::table::ftl_base *)src->ftlbase();
    nctx_++;
    base_table_entry_t *entry = ftlbase->get_entry(nctx_);
    c->init(src, entry);
    return c;
}

sdk_ret_t
hint_table::alloc_(Apictx *ctx) {

    sdk_ret_t ret = SDK_RET_OK;
    uint16_t refill_count;
    uint32_t hint = 0;
    ftl_flow_hint_id_thr_local_pool_t *thr_local_pool =
                                &thr_local_pools_[ctx->thread_id];

    if (thr_local_pool->pool_count >= 0) {
        hint = thr_local_pool->hint_ids[thr_local_pool->pool_count];
        thr_local_pool->pool_count--;
    }
    else {
        /* refill pool */
        refill_count = PDS_FLOW_HINT_POOL_COUNT_MAX;
        spin_lock_();
        while (refill_count) {
            thr_local_pool->pool_count++;
            ret = indexer_.alloc(hint);
            if (ret != SDK_RET_OK) {
                thr_local_pool->pool_count--;
                break;
            }
            thr_local_pool->hint_ids[thr_local_pool->pool_count] = hint;
            refill_count--;
        }
        spin_unlock_();
        if (thr_local_pool->pool_count >= 0) {
            hint = thr_local_pool->hint_ids[thr_local_pool->pool_count];
            thr_local_pool->pool_count--;
            ret = SDK_RET_OK;
        }
        else {
            ret = SDK_RET_NO_RESOURCE;
        }
    }

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

sdk_ret_t
hint_table::dealloc_(Apictx *ctx) {
    spin_lock_();
    indexer_.free(ctx->table_index);
    spin_unlock_();
    FTL_TRACE_VERBOSE("hint_table: Freed index:%d", ctx->table_index);
    return SDK_RET_OK;
}

inline void
hint_table::lock_(Apictx *ctx) {
    buckets_[ctx->table_index].lock_();
}

inline void
hint_table::unlock_(Apictx *ctx) {
    buckets_[ctx->table_index].unlock_();
}

sdk_ret_t
hint_table::initctx_(Apictx *ctx) {
    // All hint_table api contexts must have parent api context.
    SDK_ASSERT(ctx->pctx);

    // Make the hint from parent context as table_index
    ctx->table_index = ctx->pctx->hint;
    // Save the table_id
    ctx->table_id = table_id_;
    // Save the bucket for this context
    ctx->bucket = &buckets_[ctx->table_index];

    FTL_TRACE_VERBOSE("%s: TID:%d Idx:%d", ctx->idstr(),
                      ctx->table_id, ctx->table_index);
    return SDK_RET_OK;
}

sdk_ret_t
hint_table::initctx_with_handle_(Apictx *ctx) {
    ctx->table_index = ctx->params->handle.sindex();
    // Save the table_id
    ctx->table_id = table_id_;
    // Save the bucket for this context
    ctx->bucket = &buckets_[ctx->table_index];
    ctx->level++;

    FTL_TRACE_VERBOSE("%s: TID:%d Idx:%d", ctx->idstr(),
                      ctx->table_id, ctx->table_index);
    return SDK_RET_OK;
}
    
sdk_ret_t
hint_table::insert_(Apictx *pctx) {
__label__ done;
    sdk_ret_t ret = SDK_RET_OK;
    Apictx *hctx = NULL;

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

    lock_(hctx);
    hctx->bucket->read_(hctx);
    ret = hctx->bucket->insert_(hctx);
    if (unlikely(ret == SDK_RET_COLLISION)) {
        // Recursively call the insert_ with hint table context
        ret = insert_(hctx);
    }

    // Write to hardware
    if (ret == SDK_RET_OK) {
        ret = hctx->bucket->write_(hctx);
    }

done:
    unlock_(hctx);
    return ret;
}

//---------------------------------------------------------------------------
// hint_table tail_: Finds the tail node of a hint chain
//                            and return the context
//---------------------------------------------------------------------------
sdk_ret_t
hint_table::tail_(Apictx *ctx,
                           Apictx **retctx) {
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

    lock_(tctx);
    tctx->bucket->read_(tctx);

    auto ret = tctx->bucket->find_last_hint_(tctx);
    if (ret == SDK_RET_OK) {
        FTL_TRACE_VERBOSE("%s: find_last_hint_ Ctx: [%s], ret:%d", tctx->idstr(),
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
        FTL_TRACE_VERBOSE("%s: SDK_RET_ENTRY_NOT_FOUND: returning end tail, "
                          "metastr: [%s], ret:%d",
                          tctx->idstr(), tctx->metastr(), ret);
    } else {
        *retctx = NULL;
        FTL_TRACE_ERR("%s: find_last_hint_ failed ret:%d", tctx->idstr(), ret);
        // failure case: destroy the context
    }

    unlock_(tctx);
    return ret;
}

//---------------------------------------------------------------------------
// hint_table defragment_: Defragment the hint chain
//---------------------------------------------------------------------------
sdk_ret_t
hint_table::defragment_(Apictx *ectx) {
    sdk_ret_t ret = SDK_RET_OK;
    Apictx *tctx = NULL;

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

    ret = ectx->bucket->defragment_(ectx, tctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    dealloc_(tctx);

    SDK_ASSERT(tctx);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// hint_table remove_: Remove entry from hint table
//---------------------------------------------------------------------------
sdk_ret_t
hint_table::remove_(Apictx *ctx) {
__label__ done;
    // Initialize the context
    auto hctx = ctxnew_(ctx);
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    lock_(hctx);
    hctx->bucket->read_(hctx);

    // Remove entry from the bucket
    auto ret = hctx->bucket->remove_(hctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket remove r:%d", ret);

    if (hctx->exmatch) {
        // This means there was an exact match in the hint table and
        // it was removed. Check and defragment the hints if required.
        FTL_TRACE_VERBOSE("exact match in hint table. defragment.");
        ret = defragment_(hctx);
    } else {
        // We only found a matching hint, so remove the entry recursively
        FTL_TRACE_VERBOSE("not an exact match in hint table. remove recursively.");
        ret = remove_(hctx);
    }

done:
    unlock_(hctx);
    return ret;
}

//---------------------------------------------------------------------------
// hint_table remove_: Remove entry from hint table
//---------------------------------------------------------------------------
sdk_ret_t
hint_table::find_(Apictx *ctx,
                  Apictx **match_ctx) {
__label__ done;
    // Initialize the context
    auto hctx = ctxnew_(ctx);
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    lock_(hctx);
    hctx->bucket->read_(hctx);

    // Remove entry from the bucket
    auto ret = hctx->bucket->find_(hctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "bucket find r:%d", ret);

    if (hctx->exmatch) {
        // This means there was an exact match in the hint table
        *match_ctx = hctx;
        ret = SDK_RET_OK;
        goto done;
    } else {
        // We only found a matching hint, so find the entry recursively
        ret = find_(hctx, match_ctx);
    }

    SDK_ASSERT(*match_ctx != hctx);
done:
    unlock_(hctx);
    return ret;
}

sdk_ret_t
hint_table::get_with_handle_(Apictx *ctx) {
    SDK_ASSERT(initctx_with_handle_(ctx) == SDK_RET_OK);

    lock_(ctx);
    ctx->bucket->read_(ctx);
    ctx->params->entry->copy_key_data(ctx->entry);
    unlock_(ctx);
    
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftl_hint_table iterate_: Iterate entries from hint table
//---------------------------------------------------------------------------
sdk_ret_t
hint_table::iterate_(Apictx *ctx) {
    // Initialize the context
    auto hctx = ctxnew_(ctx);
    auto ret = base_table::iterate_(hctx);
    FTL_RET_CHECK_AND_GOTO(ret, done, "hint table iterate r:%d", ret);
done:
    return SDK_RET_OK;
}

sdk_ret_t
hint_table::clear_(Apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (ctx->clear_global_state) {
        ret = base_table::clear_(ctx);
        FTL_RET_CHECK_AND_GOTO(ret, done, "hint table clear r:%d", ret);
        memclr(ctx->props->stable_base_mem_va,
                           ctx->props->stable_base_mem_pa,
                           ctx->props->stable_size,
                           ctx->entry->entry_size());
        indexer_.clear(); 

        for (auto i = 0; i < PDS_FLOW_HINT_POOLS_MAX; i++) {
            thr_local_pools_[i].pool_count = -1;
            memset(thr_local_pools_[i].hint_ids, 0, sizeof(thr_local_pools_[i].hint_ids));
        }
    }
done:
    return SDK_RET_OK;
}
