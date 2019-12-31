//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash.hpp"
#include "mem_hash_utils.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_api_context.hpp"
#include "mem_hash_p4pd.hpp"

using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_api_context;

#define FOREACH_HINT(_n) for (uint32_t i = 1; i <= (_n); i++)
#define FOREACH_HINT_REVERSE(_n) for (uint32_t i = (_n); i > 0; i--)

//---------------------------------------------------------------------------
// mem_hash_table_bucket read_ : Read entry from the hardware
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::read_(mem_hash_api_context *ctx) {
    p4pd_error_t p4pdret = 0;

    if (ctx->sw_valid) {
        return SDK_RET_OK;
    }

    SDK_ASSERT(ctx->table_id);
    if (!MEMHASH_API_CONTEXT_IS_MAIN(ctx)) {
        SDK_ASSERT(ctx->table_index);
    }

    p4pdret = mem_hash_p4pd_entry_read(ctx->table_id, ctx->table_index,
                                       ctx->sw_key, NULL, ctx->sw_data);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_HW_READ_ERR);

    // Decode the appdata from sw_data
    get_sw_data_appdata_(ctx);
    auto v = mem_hash_p4pd_get_entry_valid(ctx);
    MEMHASH_TRACE_VERBOSE("%s: TID:%d I:%d V:%d",
                          ctx->idstr(), ctx->table_id, ctx->table_index, v);
    ctx->sw_valid = true;

    if (v) {
        MEMHASH_API_CONTEXT_PRINT_SW_FIELDS(ctx);
        if (valid_ != v && !ctx->params->force_hwread) {
            MEMHASH_TRACE_ERR("SW and HW data are out of sync !!");
            SDK_ASSERT_RETURN(0, SDK_RET_HW_READ_ERR);
        }
    }

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket write_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::write_(mem_hash_api_context *ctx) {
    p4pd_error_t p4pdret = 0;

    if (ctx->write_pending == false) {
        return SDK_RET_OK;
    }

    SDK_ASSERT(ctx->sw_valid && ctx->table_id);
    if (!MEMHASH_API_CONTEXT_IS_MAIN(ctx)) {
        SDK_ASSERT(ctx->table_index);
    }

    mem_hash_p4pd_set_entry_valid(ctx, valid_);

    if (ctx->is_hint_valid()) {
        if (HINT_SLOT_IS_MORE(ctx->hint_slot)) {
            mem_hash_p4pd_set_more_hashs(ctx, 1);
            mem_hash_p4pd_set_more_hints(ctx, ctx->hint);
        } else {
            mem_hash_p4pd_set_hint(ctx, ctx->hint, ctx->hint_slot);
            mem_hash_p4pd_set_hash(ctx, ctx->hash_msbits, ctx->hint_slot);
        }
    }

    if (ctx->props->entry_trace_en) {
        MEMHASH_API_CONTEXT_PRINT_SW_FIELDS(ctx);
    } 
    
    p4pdret = mem_hash_p4pd_entry_install(ctx->table_id, ctx->table_index,
                                          ctx->sw_key, NULL, ctx->sw_data);
    if (p4pdret != P4PD_SUCCESS) {
        MEMHASH_TRACE_ERR("failed: r:%d", p4pdret);
        // Write failure is fatal
        SDK_ASSERT(0);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    ctx->write_pending = false;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_sw_key_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_sw_key_(mem_hash_api_context *ctx, void *key) {
    ctx->write_pending = true;
    memcpy(ctx->sw_key, key ? key : ctx->params->key, ctx->props->swkey_len);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_sw_key_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_sw_key_(mem_hash_api_context *ctx) {
    ctx->write_pending = true;
    memset(ctx->sw_key, 0, ctx->props->swkey_len);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket get_sw_data_appdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::get_sw_data_appdata_(mem_hash_api_context *ctx) {
    mem_hash_p4pd_appdata_get(ctx, ctx->sw_appdata);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_sw_data_appdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_sw_data_appdata_(mem_hash_api_context *ctx, void *appdata) {
    ctx->write_pending = true;
    memcpy(ctx->sw_appdata, appdata, ctx->props->swappdata_len);
    mem_hash_p4pd_appdata_set(ctx, appdata);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_sw_data_appdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_sw_data_appdata_(mem_hash_api_context *ctx) {
    uint32_t zero_appdata[ctx->props->swappdata_len] = { 0 };
    ctx->write_pending = true;
    mem_hash_p4pd_appdata_set(ctx, zero_appdata);
    return SDK_RET_OK;
}

#if 0
//---------------------------------------------------------------------------
// mem_hash_table_bucket set_sw_data_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_sw_data_(mem_hash_api_context *ctx, void *data) {
    SDK_ASSERT(ctx->sw_data_len);
    ctx->write_pending = true;
    memcpy(ctx->sw_data, data, ctx->sw_data_len);
    mem_hash_p4pd_set_entry_valid(ctx, 1);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_sw_data_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_sw_data_(mem_hash_api_context *ctx) {
    assert(0);
    ctx->write_pending = true;
    memset(ctx->sw_data, 0, ctx->sw_data_len);
    return SDK_RET_OK;
}
#endif
//---------------------------------------------------------------------------
// mem_hash_table_bucket create_ : Create a new bucket and add an entry
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::create_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    MEMHASH_TRACE_VERBOSE("%s: Meta: [%s]", ctx->idstr(), ctx->metastr());

    if (ctx->op != SDK_TABLE_API_RESERVE) {
        // This is a new entry, key is present with the entry.
        ret = set_sw_key_(ctx, ctx->params->key);
        SDK_ASSERT(ret == SDK_RET_OK);

        // Fill common data
        ret = set_sw_data_appdata_(ctx, ctx->params->appdata);
        SDK_ASSERT(ret == SDK_RET_OK);
        
        if (!reserved_) {
            // update stats only If we are inserting a new entry 
            // reserved entries are counted already during 
            // reserve api
            ctx->table_stats->insert(!MEMHASH_API_CONTEXT_IS_MAIN(ctx));
        } else {
            reserved_ = false;
            ctx->txn->release();
        }
    } else {
        ctx->table_stats->insert(!MEMHASH_API_CONTEXT_IS_MAIN(ctx));
        reserved_ = true;
    }
    
    // Set the Handle
    if (MEMHASH_API_CONTEXT_IS_MAIN(ctx)) {
        ctx->handle->pindex(ctx->table_index);
    } else {
        ctx->handle->sindex(ctx->table_index);
    }

    // Update the bucket meta data
    valid_ = true;

    // New entry, write required.
    ctx->write_pending = true;
    ctx->sw_valid = true;

    return ret;
}

sdk_ret_t
mem_hash_table_bucket::compare_(mem_hash_api_context *ctx) {
    uint32_t hashX = 0;
    uint32_t hintX = 0;

    // There are 3 possible cases from here
    // 1) There is a matching 'hash' with a valid 'hint' (non-zero hint)
    // 2) There is free 'hash' slot, 'hint' needs to be allocated.
    // 3) All 'hash' slots are full, we have to continue to 'more_hints'

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->props->num_hints) {
        hashX = mem_hash_p4pd_get_hash(ctx, i);
        hintX = mem_hash_p4pd_get_hint(ctx, i);
        if (hashX == ctx->hash_msbits && HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            MEMHASH_TRACE_VERBOSE("%s: Match: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            break;
        } else if (!HINT_IS_VALID(hintX) && HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
            // CASE 2: Save the firstfree slots
            MEMHASH_TRACE_VERBOSE("%s: FreeSlot: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            ctx->hint_slot = i;
            HINT_SET_INVALID(ctx->hint);
            // DO NOT BREAK HERE:
            // We have to match all the slots to see if any hash matches,
            // if not, then use the empty slot to create a new hint chain.
            // This approach allows us to have holes in the slots. REVISIT TODO
        }
    }

    if (!HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
        // We have hit Case1 or Case2.
        return SDK_RET_COLLISION;
    }

    ctx->more_hashs = mem_hash_p4pd_get_more_hashs(ctx);
    ctx->hint = mem_hash_p4pd_get_more_hints(ctx);

    return SDK_RET_COLLISION;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket append_: append an entry to the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::append_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    MEMHASH_TRACE_VERBOSE("%s: Appending to bucket.", ctx->idstr());
    MEMHASH_TRACE_VERBOSE("- PreMeta : [%s]", ctx->metastr());

    ret = find_(ctx);
    if (ret == SDK_RET_OK) {
        SDK_ASSERT(ctx->match_type);
        MEMHASH_TRACE_VERBOSE("- PostMeta(find_): [%s]", ctx->metastr());
        // CASE: Either a exact match (EXM) or a hint matched
        if (ctx->is_exact_match()) {
            // CASE: if exact match, then its a duplicate insert
            MEMHASH_TRACE_VERBOSE("%s: Entry already exists.", ctx->idstr());
            return SDK_RET_ENTRY_EXISTS;
        } else if (ctx->is_hint_match()) {
            // CASE: if hint match, then its a collision, new entry should be
            // appended to the hint chain
            return SDK_RET_COLLISION;
        }
    } else if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        ret = find_first_free_hint_(ctx);
        MEMHASH_TRACE_VERBOSE("- PostMeta(find_first_free_hint_): [%s]", ctx->metastr());
        if (ret != SDK_RET_OK) {
            MEMHASH_TRACE_ERR("failed to find_first_free_hint_ ret:%d", ret);
            return ret;
        }
        // We continue with the collision path handling
        ret = SDK_RET_COLLISION;
    }

    // NOTE: Append case, write downstream bucket(s) first
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket insert_: Insert an entry into the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::insert_(mem_hash_api_context *ctx) {
    return valid_ ? append_(ctx) : create_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket insert_with_handle_: Insert an entry into the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::insert_with_handle_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    ret = create_(ctx);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to create entry, ret:%d", ret);
        return ret;
    }

    return write_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket update_ : Update a bucket entry
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::update_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    MEMHASH_TRACE_VERBOSE("%s: Updating bucket.", ctx->idstr());
    MEMHASH_TRACE_VERBOSE("- Meta: [%s]", ctx->metastr());

    // Bucket must be valid
    SDK_ASSERT(valid_);

    // Update app data
    ret = set_sw_data_appdata_(ctx, ctx->params->appdata);
    SDK_ASSERT(ret == SDK_RET_OK);

    // New entry, write required.
    ctx->write_pending = true;
    ctx->sw_valid = true;

    return write_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_first_free_hint_: Finds first free HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_first_free_hint_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t hintX = 0;

    FOREACH_HINT(ctx->props->num_hints) {
        hintX = mem_hash_p4pd_get_hint(ctx, i);
        if (!HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            break;
        }
    }

    if (!HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
        // We have found a valid hint slot.
        MEMHASH_TRACE_VERBOSE("hint slot %d is free", ctx->hint_slot);
    } else {
        ctx->more_hashs = mem_hash_p4pd_get_more_hashs(ctx);
        if (ctx->more_hashs == 0) {
            MEMHASH_TRACE_VERBOSE("more_hashs slot is free");
            ctx->hint = mem_hash_p4pd_get_more_hints(ctx);
            HINT_SLOT_SET_MORE(ctx->hint_slot);
        } else {
            MEMHASH_TRACE_VERBOSE("all hint slots are full");
            ret = SDK_RET_NO_RESOURCE;
        }
    }

    MEMHASH_TRACE_VERBOSE("Result = [ %s: FirstFreeHint: Slot:%d Hint:%d More:%d ]",
                    ctx->idstr(), ctx->hint_slot, ctx->hint, ctx->more_hashs);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_last_hint_: Finds last valid HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_last_hint_(mem_hash_api_context *ctx) {
    uint32_t hintX = 0;
    uint32_t hashX = 0;

    ctx->more_hashs = mem_hash_p4pd_get_more_hashs(ctx);
    if (ctx->more_hashs) {
        // If the more bit is set, traverse that chain
        ctx->hint = mem_hash_p4pd_get_more_hints(ctx);
        HINT_SLOT_SET_MORE(ctx->hint_slot);
    } else {
        FOREACH_HINT_REVERSE(ctx->props->num_hints) {
            hintX = mem_hash_p4pd_get_hint(ctx, i);
            hashX = mem_hash_p4pd_get_hash(ctx, i);
            if (HINT_IS_VALID(hintX)) {
                ctx->hint_slot = i;
                ctx->hint = hintX;
                ctx->hash_msbits = hashX;
                break;
            }
        }
    }

    if (HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
        MEMHASH_TRACE_VERBOSE("- No Valid Hint Found");
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    
    MEMHASH_TRACE_VERBOSE("Result = [ LastHint: Slot:%d Hint:%d ]", ctx->hint_slot, ctx->hint);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_hint_: Finds a matching HINT slot.
//  - Returns SDK_RET_OK and hint_slot = 1 to N, if individual hints match.
//  - Returns SDK_RET_OK and hint_slot = HINT_SLOT_MORE, if more_hints == 1
//  - Returns SDK_RET_ENTRY_NOT_FOUND for all other cases.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_hint_(mem_hash_api_context *ctx) {
    uint32_t hashX = 0;
    uint32_t hintX = 0;

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->props->num_hints) {
        hashX = mem_hash_p4pd_get_hash(ctx, i);
        hintX = mem_hash_p4pd_get_hint(ctx, i);
        if (hashX == ctx->hash_msbits && HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->set_hint_match();
            MEMHASH_TRACE_VERBOSE("HintMatch: Hash:%x Slot:%d Hint:%d",
                                  hashX, i, hintX);
            return SDK_RET_OK;
        }
    }

    ctx->more_hashs = mem_hash_p4pd_get_more_hashs(ctx);
    if (ctx->more_hashs) {
        // If more_hashs is set, then it is still a match at this level, if we
        // dont treat this as a match, then it will try to allocate a hint at
        // this level, which is not correct.
        ctx->hint = mem_hash_p4pd_get_more_hints(ctx);
        HINT_SLOT_SET_MORE(ctx->hint_slot);
        ctx->set_hint_match();
        return SDK_RET_OK;
    }

    MEMHASH_TRACE_VERBOSE("- No matching hint found.");
    return SDK_RET_ENTRY_NOT_FOUND;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_: Find key match or hint slot in the bucket..
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_(mem_hash_api_context *ctx) {
    bool match = false;

    // Compare the Key portion, if it matches, then we have to re-align
    // the entries.
    match = !memcmp(ctx->sw_key, ctx->params->key, ctx->props->swkey_len);
    if (match) {
        ctx->set_exact_match();
        return SDK_RET_OK;
    }
    MEMHASH_TRACE_VERBOSE("Not an exact match, searching hints.");
    return find_hint_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_hint_: Clear hint and hash
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_hint_(mem_hash_api_context *ctx) {
    p4pd_error_t    p4pdret = P4PD_SUCCESS;

    if (HINT_SLOT_IS_MORE(ctx->hint_slot)) {
        p4pdret = mem_hash_p4pd_set_more_hints(ctx, 0);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
        p4pdret = mem_hash_p4pd_set_more_hashs(ctx, 0);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    } else {
        p4pdret = mem_hash_p4pd_set_hint(ctx, 0, ctx->hint_slot);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
        p4pdret = mem_hash_p4pd_set_hash(ctx, 0, ctx->hint_slot);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    }

    HINT_SET_INVALID(ctx->hint);
    ctx->write_pending = true;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket remove_with_handle_: Remove an entry from the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::remove_with_handle_(mem_hash_api_context *ctx) {
    // This is an exact match entry, clear the key and data fields.
    MEMHASH_TRACE_VERBOSE("%s: Removing entry by handle, Ctx: [%s]",
                          ctx->idstr(), ctx->metastr());
    clear_sw_key_(ctx);
    clear_sw_data_appdata_(ctx);
    valid_ = false;
    ctx->table_stats->remove(ctx->handle->svalid());
    return write_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket remove_: Remove an entry from the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::remove_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (!valid_) {
        MEMHASH_TRACE_ERR("invalid bucket");
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = find_(ctx);
    if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("failed to find match. ret:%d", ret);
        return ret;
    }
    MEMHASH_TRACE_VERBOSE("%s: find_ result ret:%d Ctx: [%s]", ctx->idstr(), ret,
                          ctx->metastr());

    // If it is not an exact match, then no further processing is required
    // at this stage.
    if (!ctx->is_exact_match()) {
        return ret;
    }

    // This is an exact match entry, clear the key and data fields.
    clear_sw_key_(ctx);
    clear_sw_data_appdata_(ctx);

    // find the last valid hint for defragmentation
    ret = find_last_hint_(ctx);
    if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        // This bucket has no hints, we can now write to HW and
        // finish the remove processing.
        valid_ = false;
        ret = write_(ctx);
        if (ret != SDK_RET_OK) {
            MEMHASH_TRACE_ERR("HW write failed. ret:%d", ret);
            return ret;
        }
        ret = SDK_RET_OK;
        // Since this bucket has no hints, we can update stats here.
        // If it had hints, then it would be update during defragmentation
        MEMHASH_TRACE_VERBOSE("decrementing table_stats for %s", ctx->idstr());
        ctx->table_stats->remove(ctx->level);
    } else if (ret != SDK_RET_OK) {
        MEMHASH_TRACE_ERR("find_last_hint_ failed. ret:%d", ret);
    }

    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket mvkey_: Move key+data from src bucket to dst bucket
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::move_(mem_hash_api_context *dst,
                             mem_hash_api_context *src) {
    p4pd_error_t p4pdret = P4PD_SUCCESS;
    mem_hash_table_bucket *sbkt = NULL;

    // NOTE NOTE NOTE:
    // This function will be called for 'dst' bucket context.
    SDK_ASSERT(this == dst->bucket);

    sbkt = static_cast<mem_hash_table_bucket *>(src->bucket);

    // copy the key
    p4pdret = set_sw_key_(dst, src->sw_key);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    // Zero out src key
    p4pdret = clear_sw_key_(src);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    // copy the data
    p4pdret = set_sw_data_appdata_(dst, src->sw_appdata);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    // Zero out src data
    p4pdret = clear_sw_data_appdata_(src);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    MEMHASH_TRACE_VERBOSE("- moved key and data");
    // dst node is now dirty, set write pending
    dst->write_pending = true;
    PRINT_API_CTX("MOVE-DST", dst);

    // Source bucket is now ready to be deleted
    MEMHASH_TRACE_VERBOSE("- invalidate tail node");
    sbkt->valid_ = false;
    src->write_pending = true;
    PRINT_API_CTX("MOVE-SRC", src);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket mvkey_: Move key from src bucket to dst bucket
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::delink_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    // Clear the hint linkage from the parent context
    SDK_ASSERT(ctx);
    ret = clear_hint_(ctx);
    SDK_ASSERT(ret == SDK_RET_OK);
    MEMHASH_TRACE_VERBOSE("- cleared tail node hint link from parent node");
    PRINT_API_CTX("DELINK", ctx);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket defragment_: Defragment the bucket
// Special cases:
//  - If the chain is only 1 level, then ectx == pctx
//  - If we are deleting tail, then ectx == tctx
//
// Steps:
// 1) Find 'tctx' and it will give 'pctx'
// 2) Move tail node key and data to 'ectx'
// 3) Write 'ectx' (make before break)
// 4) Delete link from 'pctx' to 'tctx'
// 5) Write 'pctx' (make before break)
// 6) Write all zeros to 'tctx' (delete)
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::defragment_(mem_hash_api_context *ectx,
                                   mem_hash_api_context *tctx) {
    sdk_ret_t ret = SDK_RET_OK;
    mem_hash_api_context *pctx; // Parent context
    
    // NOTE NOTE NOTE:
    // This function will be called for 'ectx' bucket.
    SDK_ASSERT(this == ectx->bucket);

    // Get parent context from the tail node context
    pctx = tctx->pctx;
    SDK_ASSERT(pctx);

    PRINT_API_CTX("ECTX", ectx);
    PRINT_API_CTX("PCTX", pctx);
    PRINT_API_CTX("TCTX", tctx);

    // STEP 2: Move tctx key+data to ectx key+data
    if (ectx != tctx) {
        // Need to check because, we can be deleting a tail node itself
        // in that case ectx == tctx, so nothing to move.
        ret = move_(ectx, tctx);
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    // STEP 3: Write ectx to HW
    ret = write_(ectx);
    SDK_ASSERT(ret == SDK_RET_OK);

    // STEP 4: Delink parent node
    ret = static_cast<mem_hash_table_bucket *>(pctx->bucket)->delink_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    // STEP 5: Write pctx to HW
    ret = static_cast<mem_hash_table_bucket *>(pctx->bucket)->write_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    if (ectx != tctx) {
        // STEP 6: Write tctx to HW
        ret = static_cast<mem_hash_table_bucket *>(tctx->bucket)->write_(tctx);
        SDK_ASSERT(ret == SDK_RET_OK);

        // We always update the stats using the level of the tail context, this
        // is to make sure the 'hints' in table stats are accurate.
        // Without this, following will be the problem scenario,
        // when we remove an entry from main table, we decrement the stats using
        // the main table level (0), however after defragmentation, we will move
        // some hint to this entry, but we never account that stats.
        tctx->table_stats->remove(!MEMHASH_API_CONTEXT_IS_MAIN(tctx));
        MEMHASH_TRACE_VERBOSE("decrementing table_stats for %s", tctx->idstr());
    }
    return SDK_RET_OK;
}

sdk_ret_t
mem_hash_table_bucket::iterate_(mem_hash_api_context *ctx) {
    if (valid_) {
        sdk_table_api_params_t params = { 0 };
        ctx->sw_valid = false;
        read_(ctx);
        params.key = ctx->sw_key;
        params.appdata = ctx->sw_appdata;
        params.cbdata = ctx->params->cbdata;
        // Set the Handle
        if (MEMHASH_API_CONTEXT_IS_MAIN(ctx)) {
            params.handle.pindex(ctx->table_index);
        } else {
            params.handle.sindex(ctx->table_index);
        } 
        ctx->params->itercb(&params);
    }
    return SDK_RET_OK;
}
