//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <string.h>

#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "ftlv4.hpp"
#include "ftlv4_utils.hpp"
#include "ftlv4_bucket.hpp"
#include "ftlv4_apictx.hpp"
#include "ftlv4_platform.hpp"
#include "ftlv4_structs.hpp"

using sdk::table::ftlint_ipv4::ftlv4_bucket;
using sdk::table::ftlint_ipv4::ftlv4_apictx;

#define FOREACH_HINT(_n) for (uint32_t i = 1; i <= (_n); i++)
#define FOREACH_HINT_REVERSE(_n) for (uint32_t i = (_n); i > 0; i--)

//---------------------------------------------------------------------------
// ftlv4_bucket read_ : Read entry from the hardware
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::read_(ftlv4_apictx *ctx) {
    SDK_ASSERT(ctx->table_id);
    if (!FTLV4_API_CONTEXT_IS_MAIN(ctx)) {
        SDK_ASSERT(ctx->table_index);
    }

    auto p4pdret = ftlv4_platform_read(ctx);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_HW_READ_ERR);

    FTLV4_TRACE_VERBOSE("%s: TID:%d I:%d V:%d",
                      ctx->idstr(), ctx->table_id, ctx->table_index,
                      ctx->entry.entry_valid);
    if (ctx->entry.entry_valid) {
        FTLV4_API_CONTEXT_PRINT_SW_FIELDS(ctx);
        if (valid_ != ctx->entry.entry_valid) {
            FTLV4_TRACE_ERR("SW and HW data are out of sync !!");
            SDK_ASSERT_RETURN(0, SDK_RET_HW_READ_ERR);
        }
    }

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftlv4_bucket write_
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::write_(ftlv4_apictx *ctx) {
    p4pd_error_t p4pdret = 0;

    if (ctx->write_pending == false) {
        return SDK_RET_OK;
    }

    SDK_ASSERT(ctx->table_id);
    if (!FTLV4_API_CONTEXT_IS_MAIN(ctx)) {
        SDK_ASSERT(ctx->table_index);
    }

    ctx->entry.entry_valid = valid_;
    if (ctx->hint) {
        FTLV4_ENTRY_SET_HINT_HASH(ctx->hint_slot, &ctx->entry,
                                ctx->hint, ctx->hash_msbits);
    }
    
    if (ctx->props->entry_trace_en) {
        FTLV4_API_CONTEXT_PRINT_SW_FIELDS(ctx);
    } 
    
    auto ret = ftlv4_platform_write(ctx);
    if (ret != SDK_RET_OK) {
        FTLV4_TRACE_ERR("failed: r:%d", p4pdret);
        // Write failure is fatal
        SDK_ASSERT(0);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    ctx->write_pending = false;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftlv4_bucket create_ : Create a new bucket and add an entry
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::create_(ftlv4_apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    FTLV4_TRACE_VERBOSE("%s: Meta: [%s]", ctx->idstr(), ctx->metastr());
    FTLV4_ENTRY_COPY_KEY_DATA(&ctx->entry, (ftlv4_entry_t*)ctx->params->entry);
    ctx->table_stats->insert(!FTLV4_API_CONTEXT_IS_MAIN(ctx));
    
    // Set the Handle
    if (FTLV4_API_CONTEXT_IS_MAIN(ctx)) {
        ctx->params->handle.pindex(ctx->table_index);
    } else {
        ctx->params->handle.sindex(ctx->table_index);
    }

    // Update the bucket meta data
    valid_ = true;
    // New entry, write required.
    ctx->write_pending = true;

    return ret;
}

sdk_ret_t
ftlv4_bucket::compare_(ftlv4_apictx *ctx) {
    uint32_t hashX = 0;
    uint32_t hintX = 0;

    // There are 3 possible cases from here
    // 1) There is a matching 'hash' with a valid 'hint' (non-zero hint)
    // 2) There is free 'hash' slot, 'hint' needs to be allocated.
    // 3) All 'hash' slots are full, we have to continue to 'more_hints'

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->props->num_hints) {
        FTLV4_ENTRY_GET_HINT_HASH(i, &ctx->entry, hintX, hashX);
        if (hashX == ctx->hash_msbits && FTLV4_HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            FTLV4_TRACE_VERBOSE("%s: Match: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            break;
        } else if (!FTLV4_HINT_IS_VALID(hintX) && FTLV4_HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
            // CASE 2: Save the firstfree slots
            FTLV4_TRACE_VERBOSE("%s: FreeSlot: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            ctx->hint_slot = i;
            FTLV4_HINT_SET_INVALID(ctx->hint);
            // DO NOT BREAK HERE:
            // We have to match all the slots to see if any hash matches,
            // if not, then use the empty slot to create a new hint chain.
            // This approach allows us to have holes in the slots. REVISIT TODO
        }
    }

    if (FTLV4_HINT_SLOT_IS_VALID(ctx->hint_slot)) {
        // We have hit Case1 or Case2.
        return SDK_RET_COLLISION;
    }

    FTLV4_ENTRY_GET_HINT_HASH(ftlv4_apictx::hint_slot::HINT_SLOT_MORE, 
                          &ctx->entry, ctx->hint, ctx->more_hashs);

    return SDK_RET_COLLISION;
}

//---------------------------------------------------------------------------
// ftlv4_bucket append_: append an entry to the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::append_(ftlv4_apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    FTLV4_TRACE_VERBOSE("%s: Appending to bucket.", ctx->idstr());
    FTLV4_TRACE_VERBOSE("- PreMeta : [%s]", ctx->metastr());

    ret = find_(ctx);
    if (ret == SDK_RET_OK) {
        SDK_ASSERT(ctx->match);
        FTLV4_TRACE_VERBOSE("- PostMeta(find_): [%s]", ctx->metastr());
        // CASE: Either a exact match (EXM) or a hint matched
        if (ctx->exmatch) {
            // CASE: if exact match, then its a duplicate insert
            FTLV4_TRACE_VERBOSE("%s: Entry already exists.", ctx->idstr());
            return SDK_RET_ENTRY_EXISTS;
        } else if (ctx->match) {
            // CASE: if hint match, then its a collision, new entry should be
            // appended to the hint chain
            return SDK_RET_COLLISION;
        }
    } else if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        ret = find_first_free_hint_(ctx);
        FTLV4_TRACE_VERBOSE("- PostMeta(find_first_free_hint_): [%s]", ctx->metastr());
        if (ret != SDK_RET_OK) {
            FTLV4_TRACE_ERR("failed to find_first_free_hint_ ret:%d", ret);
            return ret;
        }
        // We continue with the collision path handling
        ret = SDK_RET_COLLISION;
    }

    // NOTE: Append case, write downstream bucket(s) first
    return ret;
}

//---------------------------------------------------------------------------
// ftlv4_bucket insert_: Insert an entry into the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::insert_(ftlv4_apictx *ctx) {
    return valid_ ? append_(ctx) : create_(ctx);
}

//---------------------------------------------------------------------------
// ftlv4_bucket update_ : Update a bucket entry
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::update_(ftlv4_apictx *ctx) {
    FTLV4_TRACE_VERBOSE("%s: Updating bucket.", ctx->idstr());
    FTLV4_TRACE_VERBOSE("- Meta: [%s]", ctx->metastr());

    // Bucket must be valid
    SDK_ASSERT(valid_);

    // Update app data
    FTLV4_ENTRY_COPY_DATA(&ctx->entry, (ftlv4_entry_t *)ctx->params->entry);

    // New entry, write required.
    ctx->write_pending = true;

    return write_(ctx);
}

//---------------------------------------------------------------------------
// ftlv4_bucket find_first_free_hint_: Finds first free HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::find_first_free_hint_(ftlv4_apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t hintX = 0;
    uint32_t hashX = 0;

    FOREACH_HINT(ctx->props->num_hints) {
        FTLV4_ENTRY_GET_HINT_HASH(i, &ctx->entry, hintX, hashX);
        if (!FTLV4_HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            break;
        }
    }

    if (FTLV4_HINT_SLOT_IS_VALID(ctx->hint_slot)) {
        // We have found a valid hint slot.
        FTLV4_TRACE_VERBOSE("hint slot %d is free", ctx->hint_slot);
    } else {
        FTLV4_ENTRY_GET_HINT_HASH(ftlv4_apictx::hint_slot::HINT_SLOT_MORE,
                              &ctx->entry, ctx->hint, ctx->more_hashs);
        if (ctx->more_hashs == 0) {
            FTLV4_TRACE_VERBOSE("more_hashs slot is free");
            FTLV4_HINT_SLOT_SET_MORE(ctx->hint_slot);
        } else {
            FTLV4_TRACE_VERBOSE("all hint slots are full");
            ret = SDK_RET_NO_RESOURCE;
        }
    }

    FTLV4_TRACE_VERBOSE("Result = [ %s: FirstFreeHint: Slot:%d Hint:%d More:%d ]",
                    ctx->idstr(), ctx->hint_slot, ctx->hint, ctx->more_hashs);
    return ret;
}

//---------------------------------------------------------------------------
// ftlv4_bucket find_last_hint_: Finds last valid HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::find_last_hint_(ftlv4_apictx *ctx) {
    uint32_t hintX = 0;
    uint32_t hashX = 0;

    FTLV4_ENTRY_GET_HINT_HASH(ftlv4_apictx::hint_slot::HINT_SLOT_MORE,
                          &ctx->entry, ctx->hint, ctx->more_hashs);
    if (ctx->more_hashs) {
        // If the more bit is set, traverse that chain
        FTLV4_HINT_SLOT_SET_MORE(ctx->hint_slot);
    } else {
        FOREACH_HINT_REVERSE(ctx->props->num_hints) {
            FTLV4_ENTRY_GET_HINT_HASH(i, &ctx->entry, hintX, hashX);
            if (FTLV4_HINT_IS_VALID(hintX)) {
                ctx->hint_slot = i;
                ctx->hint = hintX;
                ctx->hash_msbits = hashX;
                break;
            }
        }
    }

    if (FTLV4_HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
        FTLV4_TRACE_VERBOSE("- No Valid Hint Found");
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    
    FTLV4_TRACE_VERBOSE("Result = [ LastHint: Slot:%d Hint:%d ]", ctx->hint_slot, ctx->hint);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftlv4_bucket find_hint_: Finds a matching HINT slot.
//  - Returns SDK_RET_OK and hint_slot = 1 to N, if individual hints match.
//  - Returns SDK_RET_OK and hint_slot = HINT_SLOT_MORE, if more_hints == 1
//  - Returns SDK_RET_ENTRY_NOT_FOUND for all other cases.
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::find_hint_(ftlv4_apictx *ctx) {
    uint32_t hashX = 0;
    uint32_t hintX = 0;

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->props->num_hints) {
        FTLV4_ENTRY_GET_HINT_HASH(i, &ctx->entry, hintX, hashX);
        if (hashX == ctx->hash_msbits && FTLV4_HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->match = 1;
            FTLV4_TRACE_VERBOSE("HintMatch: Hash:%x Slot:%d Hint:%d",
                                  hashX, i, hintX);
            return SDK_RET_OK;
        }
    }

    FTLV4_ENTRY_GET_HINT_HASH(ftlv4_apictx::hint_slot::HINT_SLOT_MORE,
                          &ctx->entry, ctx->hint, ctx->more_hashs);
    if (ctx->more_hashs) {
        // If more_hashs is set, then it is still a match at this level, if we
        // dont treat this as a match, then it will try to allocate a hint at
        // this level, which is not correct.
        FTLV4_HINT_SLOT_SET_MORE(ctx->hint_slot);
        ctx->match = 1;
        return SDK_RET_OK;
    }

    FTLV4_TRACE_VERBOSE("- No matching hint found.");
    return SDK_RET_ENTRY_NOT_FOUND;
}

//---------------------------------------------------------------------------
// ftlv4_bucket find_: Find key match or hint slot in the bucket..
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::find_(ftlv4_apictx *ctx) {
    bool match = false;

    // Compare the Key portion, if it matches, then we have to re-align
    // the entries.
    // TODO
    match = ftlv4_entry_key_compare(&ctx->entry,
                                  (ftlv4_entry_t *)ctx->params->entry);
    if (match) {
        ctx->match = 1; ctx->exmatch = 1;
        return SDK_RET_OK;
    }
    FTLV4_TRACE_VERBOSE("Not an exact match, searching hints.");
    return find_hint_(ctx);
}

//---------------------------------------------------------------------------
// ftlv4_bucket clear_hint_: Clear hint and hash
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::clear_hint_(ftlv4_apictx *ctx) {
    FTLV4_ENTRY_SET_HINT_HASH(ctx->hint_slot, &ctx->entry, 0, 0);
    FTLV4_HINT_SET_INVALID(ctx->hint);
    ctx->write_pending = true;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftlv4_bucket remove_: Remove an entry from the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::remove_(ftlv4_apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (!valid_) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = find_(ctx);
    if (ret != SDK_RET_OK) {
        FTLV4_TRACE_ERR("failed to find match. ret:%d", ret);
        return ret;
    }
    FTLV4_TRACE_VERBOSE("%s: find_ result ret:%d Ctx: [%s]", ctx->idstr(), ret,
                      ctx->metastr());

    // If it is not an exact match, then no further processing is required
    // at this stage.
    if (!ctx->exmatch) {
        return ret;
    }

    // This is an exact match entry, clear the key and data fields.
    FTLV4_ENTRY_CLEAR_KEY_DATA(&ctx->entry);
    ctx->write_pending = true;

    // find the last valid hint for defragmentation
    ret = find_last_hint_(ctx);
    if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        // This bucket has no hints, we can now write to HW and
        // finish the remove processing.
        valid_ = false;
        ret = write_(ctx);
        if (ret != SDK_RET_OK) {
            FTLV4_TRACE_ERR("HW write failed. ret:%d", ret);
            return ret;
        }
        ret = SDK_RET_OK;
        // Since this bucket has no hints, we can update stats here.
        // If it had hints, then it would be update during defragmentation
        FTLV4_TRACE_VERBOSE("decrementing table_stats for %s", ctx->idstr());
        ctx->table_stats->remove(ctx->level);
    } else if (ret != SDK_RET_OK) {
        FTLV4_TRACE_ERR("find_last_hint_ failed. ret:%d", ret);
    }

    return ret;
}

//---------------------------------------------------------------------------
// ftlv4_bucket mvkey_: Move key+data from src bucket to dst bucket
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::move_(ftlv4_apictx *dst,
                             ftlv4_apictx *src) {
    // NOTE NOTE NOTE:
    // This function will be called for 'dst' bucket context.
    SDK_ASSERT(this == dst->bucket);

    auto sbkt = static_cast<ftlv4_bucket *>(src->bucket);

    // Copy the data
    FTLV4_ENTRY_COPY_DATA(&dst->entry, &src->entry);

    // Zero out src Key and Data
    FTLV4_ENTRY_CLEAR_KEY_DATA(&src->entry);

    FTLV4_TRACE_VERBOSE("- moved key and data");
    // dst node is now dirty, set write pending
    dst->write_pending = true;
    FTLV4_PRINT_API_CTX("MOVE-DST", dst);

    // Source bucket is now ready to be deleted
    FTLV4_TRACE_VERBOSE("- invalidate tail node");
    sbkt->valid_ = false;
    src->write_pending = true;
    FTLV4_PRINT_API_CTX("MOVE-SRC", src);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// ftlv4_bucket mvkey_: Move key from src bucket to dst bucket
//---------------------------------------------------------------------------
sdk_ret_t
ftlv4_bucket::delink_(ftlv4_apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    // Clear the hint linkage from the parent context
    SDK_ASSERT(ctx);
    ret = clear_hint_(ctx);
    SDK_ASSERT(ret == SDK_RET_OK);
    FTLV4_TRACE_VERBOSE("- cleared tail node hint link from parent node");
    FTLV4_PRINT_API_CTX("DELINK", ctx);
    return ret;
}

//---------------------------------------------------------------------------
// ftlv4_bucket defragment_: Defragment the bucket
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
ftlv4_bucket::defragment_(ftlv4_apictx *ectx, ftlv4_apictx *tctx) {
    sdk_ret_t ret = SDK_RET_OK;

    // Get parent context from the tail node context
    auto pctx = tctx->pctx;
    SDK_ASSERT(pctx);

    FTLV4_PRINT_API_CTX("ECTX", ectx);
    FTLV4_PRINT_API_CTX("PCTX", pctx);
    FTLV4_PRINT_API_CTX("TCTX", tctx);

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
    ret = static_cast<ftlv4_bucket *>(pctx->bucket)->delink_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    // STEP 5: Write pctx to HW
    ret = static_cast<ftlv4_bucket *>(pctx->bucket)->write_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    if (ectx != tctx) {
        // STEP 6: Write tctx to HW
        ret = static_cast<ftlv4_bucket *>(tctx->bucket)->write_(tctx);
        SDK_ASSERT(ret == SDK_RET_OK);

        // We always update the stats using the level of the tail context, this
        // is to make sure the 'hints' in table stats are accurate.
        // Without this, following will be the problem scenario,
        // when we remove an entry from main table, we decrement the stats using
        // the main table level (0), however after defragmentation, we will move
        // some hint to this entry, but we never account that stats.
        tctx->table_stats->remove(!FTLV4_API_CONTEXT_IS_MAIN(tctx));
        FTLV4_TRACE_VERBOSE("decrementing table_stats for %s", tctx->idstr());
    }
    return SDK_RET_OK;
}

sdk_ret_t
ftlv4_bucket::iterate_(ftlv4_apictx *ctx) {
    if (valid_) {
        sdk_table_api_params_t params = { 0 };
        read_(ctx);
        // Set the Handle
        if (FTLV4_API_CONTEXT_IS_MAIN(ctx)) {
            params.handle.pindex(ctx->table_index);
        } else {
            params.handle.sindex(ctx->table_index);
        }
        params.entry = &ctx->entry;
        params.cbdata = ctx->params->cbdata;
        ctx->params->itercb(&params);
    }
    return SDK_RET_OK;
}
