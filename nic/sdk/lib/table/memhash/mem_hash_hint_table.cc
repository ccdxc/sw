//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"

#include "mem_hash_table.hpp"
#include "mem_hash_api_context.hpp"
#include "mem_hash_table_bucket.hpp"

using sdk::table::memhash::mem_hash_base_table;
using sdk::table::memhash::mem_hash_hint_table;
using sdk::table::memhash::mem_hash_api_context;
using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_table_bucket;

//---------------------------------------------------------------------------
// Factory method to instantiate the mem_hash_main_table class
//---------------------------------------------------------------------------
mem_hash_hint_table *
mem_hash_hint_table::factory(uint32_t id, uint32_t size) {
    void                *mem = NULL;
    mem_hash_hint_table    *table = NULL;
    sdk_ret_t           ret = SDK_RET_OK;

    mem = (mem_hash_hint_table *) SDK_CALLOC(SDK_MEM_ALLOC_MEM_HASH_HINT_TABLE,
                                          sizeof(mem_hash_hint_table));
    if (!mem) {
        return NULL;
    }

    table = new (mem) mem_hash_hint_table();

    ret = table->init_(id, size);
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
mem_hash_hint_table::init_(uint32_t id, uint32_t size) {
    sdk_ret_t   ret = SDK_RET_OK;

    ret = mem_hash_base_table::init_(id, size);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    indexer_ = indexer::factory(table_size_, true, true);
    if (indexer_ == NULL) {
        return SDK_RET_OOM;
    }
 
    SDK_TRACE_DEBUG("Created mem_hash_main_table TableID:%d TableSize:%d "
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
}

//---------------------------------------------------------------------------
// mem_hash_main_table alloc_: Allocate a free table index (bucket)
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::alloc_(mem_hash_api_context *ctx) {
    indexer::status irs = indexer_->alloc(&(ctx->table_index));
    if (irs != indexer::SUCCESS) {
        SDK_TRACE_DEBUG("HintTable: capacity reached: %d",
                        indexer_->get_size());
        return SDK_RET_NO_RESOURCE;
    }
    SDK_TRACE_DEBUG("HintTable: Allocated index:%d", ctx->table_index);
    return SDK_RET_OK;
}    

//---------------------------------------------------------------------------
// mem_hash_main_table free_: Free a table index (bucket)
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::free_(mem_hash_api_context *ctx) {
    indexer::status irs = indexer_->free(ctx->table_index);
    if (irs == indexer::DUPLICATE_FREE) {
        // TODO: Why do we need to special case duplicate free ?
        assert(0);
        return SDK_RET_DUPLICATE_FREE;
    }
    if (irs != indexer::SUCCESS) {
        return SDK_RET_ERR;
    }
    SDK_TRACE_DEBUG("HintTable: Freed index:%d", ctx->table_index);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table initctx_ : Initialize API context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::initctx_(mem_hash_api_context *ctx) {
    sdk_ret_t   ret = SDK_RET_OK;

    // All HintTable api contexts must have parent api context.
    assert(ctx->pctx);

    // 2 cases here:
    // Case1: Hash Match in main table
    // Case2: Hit a free slot in main table
    if (ctx->pctx->hint == mem_hash_table_bucket::hint_index::HINT_INDEX_INVALID) {
        // Case 2:
        // Allocate a new hint.
        ret = alloc_(ctx);
        if (ret != SDK_RET_OK) {
            return ret;
        }

        // Save the hint in parent api context
        // REVISIT TODO:
        ctx->pctx->hint = ctx->table_index;
        ctx->pctx->write_pending = true;
    } else {
        // Derive the table_index from parent api context
        ctx->table_index = ctx->pctx->hint;
    }
    // Save the table_id
    ctx->table_id = table_id_;
    
    SDK_TRACE_DEBUG("HintTable: TableID:%d Index:%d", ctx->table_id, ctx->table_index);

    ctx->bucket = &buckets_[ctx->table_index];
    assert(ctx->bucket);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table insert_: Recursive function
//                              level0 context is main table context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::insert_(mem_hash_api_context *pctx) {
    sdk_ret_t           ret = SDK_RET_COLLISION;
    mem_hash_api_context   *hctx = NULL;

    if ((pctx->level + 1) >= pctx->max_recircs) {
        SDK_TRACE_ERR("Max Recirc levels reached.");
        return SDK_RET_MAX_RECIRC_EXCEED;
    }

    hctx = mem_hash_api_context::factory(pctx);
    if (hctx == NULL) {
        SDK_TRACE_ERR("Failed to create api context");
        return SDK_RET_OOM;
    }
    
    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);
    
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
    
    mem_hash_api_context::destroy(hctx);

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table find_: Find the entry in hint table and return ctx
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::find_(mem_hash_api_context *ctx,
                           mem_hash_api_context **retctx) {
    sdk_ret_t           ret = SDK_RET_ENTRY_NOT_FOUND;
    mem_hash_api_context   *hctx = NULL;

    hctx = mem_hash_api_context::factory(ctx);
    if (hctx == NULL) {
        SDK_TRACE_ERR("Failed to create api context");
        return SDK_RET_OOM;
    }

    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);

    ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->find_(hctx);
    if (ret == SDK_RET_OK) {
        // We have found a match, return this hint bucket ctx
        *retctx = hctx;
        return ret;
    } else if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        if (hctx->hint_slot == mem_hash_table_bucket::hint_slot::HINT_SLOT_FREE) {
            mem_hash_api_context::destroy(hctx);
            return SDK_RET_ENTRY_NOT_FOUND;
        }
        // Recursively find the entry.
        ret = find_(hctx, retctx);
    }
    
    mem_hash_api_context::destroy(hctx);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table defragment_: Defragment the bucket
// - Find the last matching entry in the hint chain
// - Move the last entry to 'ctx' bucket's key and data slots
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::defragment_(mem_hash_api_context *ctx) {
    sdk_ret_t   ret = SDK_RET_OK;

    //TODO: Find the last entry in the chain.
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_hint_table remove_: Remove entry from main table
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_hint_table::remove_(mem_hash_api_context *ctx) {
    sdk_ret_t           ret = SDK_RET_OK;
    mem_hash_api_context   *hctx = NULL;

    hctx = mem_hash_api_context::factory(ctx);
    if (hctx == NULL) {
        SDK_TRACE_ERR("Failed to create api context");
        return SDK_RET_OOM;
    }
    
    // Initialize the context
    SDK_ASSERT_RETURN(initctx_(hctx) == SDK_RET_OK, SDK_RET_ERR);
    
    // Remove entry from the bucket
    ret = static_cast<mem_hash_table_bucket*>(hctx->bucket)->remove_(hctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("remove_ failed. ret:%d", ret);
        mem_hash_api_context::destroy(hctx);
        return ret;
    }

    SDK_ASSERT(hctx->match_type);
    if (hctx->match_type == mem_hash_api_context::match_type::MATCH_TYPE_EXM) {
        // This means there was an exact match in the main table and 
        // it was removed. Check and defragment the hints if required.
        ret = defragment_(hctx);
    } else {
        // We only found a matching hint, so remove the entry recursively
        ret = remove_(hctx);
    }
    
    mem_hash_api_context::destroy(ctx);
    return ret;
}
