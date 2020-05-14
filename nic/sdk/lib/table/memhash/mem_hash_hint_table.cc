//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"

#include "mem_hash_table.hpp"
#include "mem_hash_api_context.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_utils.hpp"

using sdk::table::memhash::mem_hash_base_table;
using sdk::table::memhash::mem_hash_hint_table;
using sdk::table::memhash::mem_hash_api_context;
using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_table_bucket;

//---------------------------------------------------------------------------
// Factory method to instantiate the mem_hash_main_table class
//---------------------------------------------------------------------------
mem_hash_hint_table *
mem_hash_hint_table::factory(sdk::table::properties_t *props) {
    void *mem = NULL;
    mem_hash_hint_table *table = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (mem_hash_hint_table *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_HINT_TABLE,
                                          sizeof(mem_hash_hint_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) mem_hash_hint_table();

    ret = table->init_(props);
    if (ret != SDK_RET_OK) {
        table->~mem_hash_hint_table();
        SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_HINT_TABLE, mem);
    }

    return table;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table init()
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::init_(sdk::table::properties_t *props) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = mem_hash_base_table::init_(props->stable_id,
                                     props->stable_size);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    indexer_ = indexer::factory(table_size_, true, true);
    if (indexer_ == NULL) {
        return SDK_RET_OOM;
    }

    MEMHASH_TRACE_VERBOSE("Created mem_hash_main_table TableID:%d TableSize:%d "
                          "NumTableIndexBits:%d",
                          table_id_, table_size_, num_table_index_bits_);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_main_table destroy_
//---------------------------------------------------------------------------
void
mem_hash_hint_table::destroy_(mem_hash_hint_table *table)
{
    mem_hash_base_table::destroy_(table);
    indexer::destroy(table->indexer_);
    SDK_FREE(SDK_MEM_ALLOC_MEM_HASH_HINT_TABLE, table);
}

//---------------------------------------------------------------------------
// mem_hash_main_table alloc_: Allocate a hint (bucket)
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::alloc_(mem_hash_api_context *ctx) {
#ifdef PERF_DBG
    static uint32_t hint_index = 1;
    ctx->hint = hint_index++;
    return SDK_RET_OK;
#endif
    indexer::status irs = indexer_->alloc(&(ctx->hint));
    if (irs != indexer::SUCCESS) {
        MEMHASH_TRACE_ERR("HintTable: capacity reached: %d",
                      indexer_->get_size());
        return SDK_RET_NO_RESOURCE;
    }
    MEMHASH_TRACE_VERBOSE("%s: Allocated index:%d Meta:[%s]",
                          ctx->idstr(), ctx->hint, ctx->metastr());

    // Set write pending
    ctx->write_pending = true;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_main_table dealloc_: De-Allocate a hint (bucket)
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::dealloc_(mem_hash_api_context *ctx) {
    if (HINT_IS_VALID(ctx->table_index)) {
        indexer::status irs = indexer_->free(ctx->table_index);
        if (irs == indexer::DUPLICATE_FREE) {
            // TODO: Why do we need to special case duplicate free ?
            SDK_ASSERT(0);
            return SDK_RET_DUPLICATE_FREE;
        }
        if (irs != indexer::SUCCESS) {
            return SDK_RET_ERR;
        }
        MEMHASH_TRACE_VERBOSE("HintTable: Freed index:%d num_indices_allocated:%ld",
                              ctx->table_index, indexer_->num_indices_allocated());
    }

    // Clear the hint and set write pending
    HINT_SET_INVALID(ctx->table_index);
    ctx->write_pending = true;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table initctx_ : Initialize API context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::initctx_(mem_hash_api_context *ctx) {
    // All HintTable api contexts must have parent api context.
    SDK_ASSERT(ctx->pctx);

    // Make the hint from parent context as table_index
    ctx->table_index = ctx->pctx->hint;
    // Save the table_id
    ctx->table_id = table_id_;

    MEMHASH_TRACE_VERBOSE("%s: TID:%d Idx:%d", ctx->idstr(),
                          ctx->table_id, ctx->table_index);

    ctx->bucket = &buckets_[ctx->table_index];
    SDK_ASSERT(ctx->bucket);

    return static_cast<mem_hash_table_bucket*>(ctx->bucket)->read_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_hint_table insert_: Recursive function
//                              level0 context is main table context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::insert_(mem_hash_api_context *pctx) {
    sdk_ret_t ret = SDK_RET_COLLISION;
    mem_hash_api_context *hctx = NULL;

    if ((pctx->level + 1) >= pctx->props->max_recircs) {
        MEMHASH_TRACE_ERR("Max Recirc levels reached.");
        return SDK_RET_MAX_RECIRC_EXCEED;
    }

    // If the hint is invalid, allocate a new hint
    if (!HINT_IS_VALID(pctx->hint)) {
        ret = alloc_(pctx);
        if (ret != SDK_RET_OK) {
            MEMHASH_TRACE_ERR("failed to allocate hint. ret:%d", ret);
            return ret;
        }
    }

    // Create a new context for this entry from parent context
    hctx = ctxnew_(pctx);
    if (hctx == NULL) {
        MEMHASH_TRACE_ERR("failed to create api context");
        return SDK_RET_OOM;
    }

    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    if (hctx->handle->valid()) {
        // If handle is valid, insert directly using the handle.
        ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->insert_with_handle_(hctx);
    } else {
        // Insert entry to the bucket
        ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->insert_(hctx);
        if (ret == SDK_RET_COLLISION) {
            // Recursively call the insert_ with hint table context
            ret = insert_(hctx);
        }

        // Write to hardware
        if (ret == SDK_RET_OK) {
            ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->write_(hctx);
        }
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table tail_: Finds the tail node of a hint chain
//                            and return the context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::tail_(mem_hash_api_context *ctx,
                           mem_hash_api_context **retctx) {
    sdk_ret_t ret = SDK_RET_ENTRY_NOT_FOUND;
    mem_hash_api_context *tctx = NULL;

    if (!HINT_IS_VALID(ctx->hint)) {
        MEMHASH_TRACE_VERBOSE("No hints, setting TAIL = EXACT.");
        *retctx = ctx;
        return SDK_RET_OK;
    }

    tctx = ctxnew_(ctx);
    if (tctx == NULL) {
        MEMHASH_TRACE_ERR("failed to create api context");
        return SDK_RET_OOM;
    }

    if (tctx->is_max_recircs()) {
        MEMHASH_TRACE_ERR("Max Recirc levels reached.");
        // We should have caught this in insert.
        SDK_ASSERT(0);
        return SDK_RET_MAX_RECIRC_EXCEED;
    }

    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(tctx) == SDK_RET_OK, SDK_RET_ERR);

    ret = static_cast<mem_hash_table_bucket*>(tctx->bucket)->find_last_hint_(tctx);
    if (ret == SDK_RET_OK) {
        MEMHASH_TRACE_VERBOSE("%s: find_last_hint_ Ctx: [%s], ret:%d", tctx->idstr(),
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
        MEMHASH_TRACE_ERR("%s: find_last_hint_ failed ret:%d", tctx->idstr(), ret);
        // failure case: destroy the context
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table defragment_: Defragment the hint chain
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::defragment_(mem_hash_api_context *ectx) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *tctx = NULL;
    mem_hash_api_context *tempctx = NULL;

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

    MEMHASH_TRACE_VERBOSE("%s: Starting defragmentation, Ctx: [%s]",
                          ectx->idstr(), ectx->metastr());
    // Get tail node context
    ret = tail_(ectx, &tctx);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("defragment_ failed ret:%d", ret);
        return ret;
    }

    ret = static_cast<mem_hash_table_bucket*>(ectx->bucket)->defragment_(ectx, tctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    SDK_ASSERT(tctx);
    dealloc_(tctx);

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
// mem_hash_hint_table remove_: Remove entry from hint table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::remove_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *hctx = NULL;

    hctx = ctxnew_(ctx);
    if (hctx == NULL) {
        MEMHASH_TRACE_ERR("failed to create api context");
        return SDK_RET_OOM;
    }

    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);
    if (hctx->handle->valid()) {
        // If handle is valid, insert directly using the handle.
        ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->remove_with_handle_(hctx);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        dealloc_(hctx);
    } else {
        // Remove entry from the bucket
        ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->remove_(hctx);
        if (ret != SDK_RET_OK) {
            MEMHASH_TRACE_ERR("remove_ failed. ret:%d", ret);
            return ret;
        }

        if (hctx->is_exact_match()) {
            // This means there was an exact match in the hint table and
            // it was removed. Check and defragment the hints if required.
            ret = defragment_(hctx);
        } else {
            // We only found a matching hint, so remove the entry recursively
            ret = remove_(hctx);
        }
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table remove_: Remove entry from hint table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::find_(mem_hash_api_context *ctx,
                           mem_hash_api_context **match_ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *hctx = NULL;

    hctx = ctxnew_(ctx);
    if (hctx == NULL) {
        MEMHASH_TRACE_ERR("failed to create api context");
        return SDK_RET_OOM;
    }

    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    // Remove entry from the bucket
    ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->find_(hctx);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("find_ failed. ret:%d", ret);
        return ret;
    }

    if (hctx->is_exact_match()) {
        // This means there was an exact match in the hint table
        *match_ctx = hctx;
        return SDK_RET_OK;
    } else {
        // We only found a matching hint, so find the entry recursively
        ret = find_(hctx, match_ctx);
    }

    SDK_ASSERT(*match_ctx != hctx);
    return ret;
}

sdk_ret_t
mem_hash_hint_table::validate_(mem_hash_api_context *ctx) {
    sdk_table_stats_t ts;
    ctx->table_stats->get(&ts);
    if (ts.collisions != indexer_->num_indices_allocated()) {
        MEMHASH_TRACE_ERR("Hint count mismatch table_stats:%lu indexer_stats:%lu",
                          ts.collisions, indexer_->num_indices_allocated());
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}
