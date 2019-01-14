//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash.hpp"
#include "mem_hash_utils.hpp"
#include "mem_hash_table_bucket.hpp"
#include "mem_hash_api_context.hpp"

using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_api_context;

#define FOREACH_HINT(_n) for (uint32_t i = 1; i <= (_n); i++)
#define FOREACH_HINT_REVERSE(_n) for (uint32_t i = (_n); i > 0; i--)

//---------------------------------------------------------------------------
// mem_hash_table_bucket read_ : Read entry from the hardware
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::read_(mem_hash_api_context *ctx) {
    p4pd_error_t pdret = 0;

    if (ctx->hw_valid) {
        return SDK_RET_OK;
    }

    SDK_ASSERT(ctx->table_id && ctx->table_index);

    pdret = p4pd_entry_read(ctx->table_id, ctx->table_index,
                            ctx->hwkey, NULL, ctx->hwdata);
    SDK_ASSERT_RETURN(pdret == P4PD_SUCCESS, SDK_RET_HW_READ_ERR);

    SDK_TRACE_DEBUG("%s: HW Read: TableID:%d TableIndex:%d", ctx->idstr(),
                    ctx->table_id, ctx->table_index);
    PRINT_API_CTX_HW_DATA(ctx);

    ctx->hw_valid = true;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket write_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::write_(mem_hash_api_context *ctx) {
    p4pd_error_t pdret = 0;

    if (ctx->write_pending == false) {
        return SDK_RET_OK;
    }

    SDK_ASSERT(ctx->hw_valid && ctx->table_id && ctx->table_index);

    p4pd_mem_hash_entry_set_entry_valid(ctx->table_id, ctx->hwdata, valid_);

    if (ctx->is_hint_valid()) {
        if (HINT_SLOT_IS_MORE(ctx->hint_slot)) {
            p4pd_mem_hash_entry_set_more_hashes(ctx->table_id, ctx->hwdata, 1);
            p4pd_mem_hash_entry_set_more_hints(ctx->table_id, ctx->hwdata,
                                               ctx->hint);
        } else {
            p4pd_mem_hash_entry_set_hint(ctx->table_id, ctx->hwdata,
                                         ctx->hint, ctx->hint_slot);
            p4pd_mem_hash_entry_set_hash(ctx->table_id, ctx->hwdata,
                                         ctx->hash_msbits, ctx->hint_slot);
        }
    }

    SDK_TRACE_DEBUG("%s: HW Write: TableID:%d TableIndex:%d", ctx->idstr(),
                    ctx->table_id, ctx->table_index);
    PRINT_API_CTX_HW_DATA(ctx);

    pdret = p4pd_entry_write(ctx->table_id, ctx->table_index,
                             ctx->hwkey, NULL, ctx->hwdata);
    if (pdret != P4PD_SUCCESS) {
        SDK_TRACE_ERR("HW write: ret:%d", pdret);
        // Write failure is fatal
        SDK_ASSERT(0);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    ctx->write_pending = false;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_hwkey_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_hwkey_(mem_hash_api_context *ctx, void *key) {
    ctx->write_pending = true;
    memcpy(ctx->hwkey, key ? key : ctx->key, ctx->hwkey_len);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_hwkey_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_hwkey_(mem_hash_api_context *ctx) {
    ctx->write_pending = true;
    memset(ctx->hwkey, 0, ctx->hwkey_len);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_hwdata_appdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_hwdata_appdata_(mem_hash_api_context *ctx, void *appdata) {
    SDK_ASSERT(ctx->appdata_len);
    ctx->write_pending = true;
    memcpy(ctx->hwdata, appdata, ctx->appdata_len);
    PRINT_API_CTX_HW_DATA(ctx);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_hwdata_appdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_hwdata_appdata_(mem_hash_api_context *ctx) {
    SDK_ASSERT(ctx->appdata_len);
    ctx->write_pending = true;
    memset(ctx->hwdata, 0, ctx->appdata_len);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_hwdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_hwdata_(mem_hash_api_context *ctx, void *data) {
    SDK_ASSERT(ctx->hwdata_len);
    ctx->write_pending = true;
    memcpy(ctx->hwdata, data, ctx->hwdata_len);
    p4pd_mem_hash_entry_set_entry_valid(ctx->table_id, ctx->hwdata, 1);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_hwdata_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_hwdata_(mem_hash_api_context *ctx) {
    assert(0);
    ctx->write_pending = true;
    memset(ctx->hwdata, 0, ctx->hwdata_len);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket create_ : Create a new bucket and add an entry
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::create_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("%s: Creating new bucket.", ctx->idstr());
    SDK_TRACE_DEBUG("- Meta: [%s]", ctx->metastr());

    // This is a new entry, key is present with the entry.
    ret = set_hwkey_(ctx, ctx->key);
    SDK_ASSERT(ret == SDK_RET_OK);

    // Fill common data
    ret = set_hwdata_appdata_(ctx, ctx->appdata);
    SDK_ASSERT(ret == SDK_RET_OK);

    // Update the bucket meta data
    valid_ = true;

    // New entry, write required.
    ctx->write_pending = true;
    ctx->hw_valid = true;

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
    FOREACH_HINT(ctx->num_hints) {
        hashX = p4pd_mem_hash_entry_get_hash(ctx->table_id, ctx->hwdata, i);
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->hwdata, i);
        if (hashX == ctx->hash_msbits && HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            SDK_TRACE_DEBUG("%s: Match: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            break;
        } else if (!HINT_IS_VALID(hintX) && HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
            // CASE 2: Save the firstfree slots
            SDK_TRACE_DEBUG("%s: FreeSlot: Hash:%x Slot:%d Hint:%d",
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

    ctx->more_hashes = p4pd_mem_hash_entry_get_more_hashes(ctx->table_id, ctx->hwdata);
    ctx->hint = p4pd_mem_hash_entry_get_more_hints(ctx->table_id, ctx->hwdata);

    return SDK_RET_COLLISION;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket append_: append an entry to the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::append_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("%s: Appending to bucket.", ctx->idstr());
    SDK_TRACE_DEBUG("- PreMeta : [%s]", ctx->metastr());

    ret = find_(ctx);
    if (ret == SDK_RET_OK) {
        SDK_ASSERT(ctx->match_type);
        SDK_TRACE_DEBUG("- PostMeta(find_): [%s]", ctx->metastr());
        // CASE: Either a exact match (EXM) or a hint matched
        if (ctx->is_exact_match()) {
            // CASE: if exact match, then its a duplicate insert
            SDK_TRACE_DEBUG("%s: Entry already exists.", ctx->idstr());
            return SDK_RET_ENTRY_EXISTS;
        } else if (ctx->is_hint_match()) {
            // CASE: if hint match, then its a collision, new entry should be
            // appended to the hint chain
            return SDK_RET_COLLISION;
        }
    } else if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        ret = find_first_free_hint_(ctx);
        SDK_TRACE_DEBUG("- PostMeta(find_first_free_hint_): [%s]", ctx->metastr());
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Failed to find_first_free_hint_ ret:%d", ret);
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
// mem_hash_table_bucket update_ : Update a bucket entry
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::update_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("%s: Updating bucket.", ctx->idstr());
    SDK_TRACE_DEBUG("- Meta: [%s]", ctx->metastr());

    // Bucket must be valid
    SDK_ASSERT(valid_);

    // Update app data
    ret = set_hwdata_appdata_(ctx, ctx->appdata);
    SDK_ASSERT(ret == SDK_RET_OK);

    // New entry, write required.
    ctx->write_pending = true;
    ctx->hw_valid = true;

    return write_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_first_free_hint_: Finds first free HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_first_free_hint_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t hintX = 0;

    FOREACH_HINT(ctx->num_hints) {
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->hwdata, i);
        if (!HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            break;
        }
    }

    if (!HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
        // We have found a valid hint slot.
        SDK_TRACE_ERR("hint slot %d is free", ctx->hint_slot);
    } else {
        ctx->more_hashes = p4pd_mem_hash_entry_get_more_hashes(ctx->table_id, ctx->hwdata);
        if (ctx->more_hashes == 0) {
            SDK_TRACE_ERR("more_hashes slot is free");
            ctx->hint = p4pd_mem_hash_entry_get_more_hints(ctx->table_id, ctx->hwdata);
            HINT_SLOT_SET_MORE(ctx->hint_slot);
        } else {
            SDK_TRACE_ERR("all hint slots are full");
            ret = SDK_RET_NO_RESOURCE;
        }
    }

    SDK_TRACE_DEBUG("%s: FirstFreeHint: Slot:%d Hint:%d More:%d",
                    ctx->idstr(), ctx->hint_slot, ctx->hint, ctx->more_hashes);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_last_hint_: Finds last valid HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_last_hint_(mem_hash_api_context *ctx) {
    uint32_t hintX = 0;
    uint32_t hashX = 0;

    FOREACH_HINT_REVERSE(ctx->num_hints) {
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->hwdata, i);
        hashX = p4pd_mem_hash_entry_get_hash(ctx->table_id, ctx->hwdata, i);
        if (HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->hash_msbits = hashX;
            SDK_TRACE_DEBUG("LastValidHint: Slot:%d Hint:%d",
                            ctx->hint_slot, ctx->hint);
            break;
        }
    }

    if (HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
        SDK_TRACE_DEBUG("- No Valid Hint Found");
        return SDK_RET_ENTRY_NOT_FOUND;
    }

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
    FOREACH_HINT(ctx->num_hints) {
        hashX = p4pd_mem_hash_entry_get_hash(ctx->table_id, ctx->hwdata, i);
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->hwdata, i);
        if (hashX == ctx->hash_msbits && HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->set_hint_match();
            SDK_TRACE_DEBUG("HintMatch: Hash:%x Slot:%d Hint:%d",
                            hashX, i, hintX);
            return SDK_RET_OK;
        }
    }

    ctx->more_hashes = p4pd_mem_hash_entry_get_more_hashes(ctx->table_id, ctx->hwdata);
    if (ctx->more_hashes) {
        // If more_hashes is set, then it is still a match at this level, if we
        // dont treat this as a match, then it will try to allocate a hint at
        // this level, which is not correct.
        ctx->hint = p4pd_mem_hash_entry_get_more_hints(ctx->table_id, ctx->hwdata);
        HINT_SLOT_SET_MORE(ctx->hint_slot);
        ctx->set_hint_match();
        return SDK_RET_OK;
    }

    SDK_TRACE_DEBUG("- No matching hint found.");
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
    match = !memcmp(ctx->hwkey, ctx->key, ctx->hwkey_len);
    if (match) {
        ctx->set_exact_match();
        return SDK_RET_OK;
    }
    SDK_TRACE_DEBUG("Not an exact match, searching hints.");
    return find_hint_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_hint_: Clear hint and hash
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_hint_(mem_hash_api_context *ctx) {
    p4pd_error_t    p4pdret = P4PD_SUCCESS;

    if (HINT_SLOT_IS_MORE(ctx->hint_slot)) {
        p4pdret = p4pd_mem_hash_entry_set_more_hints(ctx->table_id,
                                                     ctx->hwdata, 0);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);

        p4pdret = p4pd_mem_hash_entry_set_more_hashes(ctx->table_id,
                                                     ctx->hwdata, 0);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    } else {
        p4pdret = p4pd_mem_hash_entry_set_hint(ctx->table_id, ctx->hwdata,
                                               0, ctx->hint_slot);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);

        p4pdret = p4pd_mem_hash_entry_set_hash(ctx->table_id, ctx->hwdata,
                                               0, ctx->hint_slot);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
    }

    HINT_SET_INVALID(ctx->hint);
    ctx->write_pending = true;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket remove_: Remove an entry from the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::remove_(mem_hash_api_context *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (!valid_) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = find_(ctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to find match. ret:%d", ret);
        return ret;
    }
    SDK_TRACE_DEBUG("%s: find_ result ret:%d Ctx: [%s]", ctx->idstr(), ret,
                    ctx->metastr());

    // If it is not an exact match, then no further processing is required
    // at this stage.
    if (! ctx->is_exact_match()) {
        return ret;
    }

    // This is an exact match entry, clear the key and data fields.
    clear_hwkey_(ctx);
    clear_hwdata_appdata_(ctx);

    // find the last valid hint for defragmentation
    ret = find_last_hint_(ctx);
    if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        // This bucket has no hints, we can now write to HW and
        // finish the remove processing.
        valid_ = false;
        ret = write_(ctx);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("HW write failed. ret:%d", ret);
            return ret;
        }
        ret = SDK_RET_OK;
    } else if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("find_last_hint_ failed. ret:%d", ret);
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

    sbkt = static_cast<mem_hash_table_bucket *>(src->bucket);

    // copy the key
    p4pdret = set_hwkey_(dst, src->hwkey);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    // Zero out src key
    p4pdret = clear_hwkey_(src);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    // copy the data
    p4pdret = set_hwdata_appdata_(dst, src->hwdata);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    // Zero out src data
    p4pdret = clear_hwdata_appdata_(src);
    SDK_ASSERT(p4pdret == P4PD_SUCCESS);

    SDK_TRACE_DEBUG("- moved key and data");
    // dst node is now dirty, set write pending
    dst->write_pending = true;
    PRINT_API_CTX("MOVE-DST", dst);

    // Source bucket is now ready to be deleted
    SDK_TRACE_DEBUG("- invalidate tail node");
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
    SDK_TRACE_DEBUG("- cleared tail node hint link from parent node");
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

    // Get parent context from the tail node context
    pctx = tctx->pctx;
    SDK_ASSERT(pctx);

    PRINT_API_CTX("ECTX", ectx);
    PRINT_API_CTX("TCTX", tctx);
    PRINT_API_CTX("PCTX", pctx);

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
    ret = delink_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    // STEP 5: Write pctx to HW
    ret = write_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    if (ectx != tctx) {
        // STEP 6: Write tctx to HW
        ret = write_(tctx);
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    return SDK_RET_OK;
}
