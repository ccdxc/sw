//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "ftl_includes.hpp"

#define FOREACH_HINT(_n) for (uint32_t i = 1; i <= (_n); i++)
#define FOREACH_HINT_REVERSE(_n) for (uint32_t i = (_n); i > 0; i--)

sdk_ret_t
Bucket::read_(Apictx *ctx, bool force_hwread) {
    SDK_ASSERT(ctx->table_id);
    if (!ctx->is_main()) {
        SDK_ASSERT(ctx->table_index);
    }

    if (valid_ || force_hwread) {
        auto p4pdret = memrd(ctx);
        SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_HW_READ_ERR);
    }
    if (ctx->entry->get_entry_valid()) {
        FTL_TRACE_VERBOSE("%s: TID:%d I:%d V:%d",
                          ctx->idstr(), ctx->table_id, ctx->table_index,
                          ctx->entry->get_entry_valid());
        ctx->trace();
        if (valid_ != ctx->entry->get_entry_valid()) {
            FTL_TRACE_ERR("SW and HW data are out of sync !!");
            SDK_ASSERT_RETURN(0, SDK_RET_HW_READ_ERR);
        }
    }
    ctx->tstats->read(ctx->level);
    return SDK_RET_OK;
}

sdk_ret_t
Bucket::write_(Apictx *ctx) {
    p4pd_error_t p4pdret = 0;

    if (ctx->write_pending == false) {
        return SDK_RET_OK;
    }

    SDK_ASSERT(ctx->table_id);
    if (!ctx->is_main()) {
        SDK_ASSERT(ctx->table_index);
    }

    ctx->entry->set_entry_valid(valid_);
    if (ctx->hint) {
        if (ctx->hint_slot == ctx->entry->get_more_hint_slot()) {
            ctx->entry->set_hint_hash(ctx->hint_slot, ctx->hint, 1);
        } else {
            ctx->entry->set_hint_hash(ctx->hint_slot, ctx->hint, ctx->hash_msbits);
        }
    }

    ctx->trace();

    auto ret = memwr(ctx);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("failed: r:%d", p4pdret);
        // Write failure is fatal
        SDK_ASSERT(0);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    ctx->write_pending = false;

    ctx->tstats->write(ctx->level);
    return SDK_RET_OK;
}

sdk_ret_t
Bucket::create_(Apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_TRACE_VERBOSE("%s: Meta: [%s]", ctx->idstr(), ctx->metastr());
    ctx->entry->copy_key_data(ctx->params->entry);
    ctx->tstats->insert(!ctx->is_main());

    // Set the Handle
    if (ctx->is_main()) {
        ctx->params->handle.pindex(ctx->table_index);
        FTL_TRACE_VERBOSE("pindex: %d", ctx->params->handle.pindex());
    } else {
        ctx->params->handle.sindex(ctx->table_index);
        FTL_TRACE_VERBOSE("sindex: %d", ctx->params->handle.sindex());
    }

    // Update the bucket meta data
    valid_ = true;
    // New entry, write required.
    ctx->write_pending = true;

    return ret;
}

sdk_ret_t
Bucket::compare_(Apictx *ctx) {
    uint16_t hashX = 0;
    uint32_t hintX = 0;

    // There are 3 possible cases from here
    // 1) There is a matching 'hash' with a valid 'hint' (non-zero hint)
    // 2) There is free 'hash' slot, 'hint' needs to be allocated.
    // 3) All 'hash' slots are full, we have to continue to 'more_hints'

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->props->num_hints) {
        ctx->entry->get_hint_hash(i, hintX, hashX);
        if (hashX == ctx->hash_msbits && HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            FTL_TRACE_VERBOSE("%s: Match: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            break;
        } else if (!HINT_IS_VALID(hintX) && HINT_SLOT_IS_INVALID(ctx->hint_slot)) {
            // CASE 2: Save the firstfree slots
            FTL_TRACE_VERBOSE("%s: FreeSlot: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            ctx->hint_slot = i;
            HINT_SET_INVALID(ctx->hint);
            // DO NOT BREAK HERE:
            // We have to match all the slots to see if any hash matches,
            // if not, then use the empty slot to create a new hint chain.
            // This approach allows us to have holes in the slots. REVISIT TODO
        }
    }

    if (HINT_SLOT_IS_VALID(ctx->hint_slot)) {
        // We have hit Case1 or Case2.
        return SDK_RET_COLLISION;
    }

    ctx->entry->get_hint_hash(Apictx::hint_slot::HINT_SLOT_MORE,
                             ctx->hint, ctx->more_hashs);

    return SDK_RET_COLLISION;
}

sdk_ret_t
Bucket::append_(Apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    FTL_TRACE_VERBOSE("%s: Appending to bucket. - PreMeta: [%s].", ctx->idstr(), ctx->metastr());

    ret = find_(ctx);
    if (ret == SDK_RET_OK) {
        SDK_ASSERT(ctx->match);
        FTL_TRACE_VERBOSE("- PostMeta(find_): [%s]", ctx->metastr());
        // CASE: Either a exact match (EXM) or a hint matched
        if (ctx->exmatch) {
            // CASE: if exact match, then its a duplicate insert
            FTL_TRACE_VERBOSE("%s: Entry already exists.", ctx->idstr());
            return SDK_RET_ENTRY_EXISTS;
        } else if (ctx->match) {
            // CASE: if hint match, then its a collision, new entry should be
            // appended to the hint chain
            return SDK_RET_COLLISION;
        }
    } else if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        ret = find_first_free_hint_(ctx);
        FTL_TRACE_VERBOSE("- PostMeta(find_first_free_hint_): [%s]", ctx->metastr());
        if (ret != SDK_RET_OK) {
            FTL_TRACE_ERR("failed to find_first_free_hint_ ret:%d", ret);
            return ret;
        }
        // We continue with the collision path handling
        ret = SDK_RET_COLLISION;
    }

    // NOTE: Append case, write downstream bucket(s) first
    return ret;
}

sdk_ret_t
Bucket::insert_(Apictx *ctx) {
    return valid_ ? append_(ctx) : create_(ctx);
}

sdk_ret_t
Bucket::update_(Apictx *ctx) {
    FTL_TRACE_VERBOSE("%s: Updating bucket.", ctx->idstr());
    FTL_TRACE_VERBOSE("- Meta: [%s]", ctx->metastr());

    // Bucket must be valid
    SDK_ASSERT(valid_);

    // Update app data
    ctx->entry->copy_data(ctx->params->entry);

    // Set the Handle
    if (ctx->is_main()) {
        ctx->params->handle.pindex(ctx->table_index);
        FTL_TRACE_VERBOSE("pindex: %d", ctx->params->handle.pindex());
    } else {
        ctx->params->handle.sindex(ctx->table_index);
        FTL_TRACE_VERBOSE("sindex: %d", ctx->params->handle.sindex());
    }

    // New entry, write required.
    ctx->write_pending = true;

    return write_(ctx);
}

sdk_ret_t
Bucket::find_first_free_hint_(Apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t hintX = 0;

    FOREACH_HINT(ctx->props->num_hints) {
        ctx->entry->get_hint(i, hintX);
        if (!HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            break;
        }
    }

    if (HINT_SLOT_IS_VALID(ctx->hint_slot)) {
        // We have found a valid hint slot.
        FTL_TRACE_VERBOSE("hint slot %d is free", ctx->hint_slot);
    } else {
        ctx->entry->get_hint_hash(Apictx::hint_slot::HINT_SLOT_MORE,
                                 ctx->hint, ctx->more_hashs);
        if (ctx->more_hashs == 0) {
            FTL_TRACE_VERBOSE("more_hashs slot is free");
            ctx->hint_slot = ctx->entry->get_more_hint_slot();
        } else {
            FTL_TRACE_VERBOSE("all hint slots are full");
            ret = SDK_RET_NO_RESOURCE;
        }
    }

    FTL_TRACE_VERBOSE("Result = [ %s: FirstFreeHint: Slot:%d Hint:%d More:%d ]",
                    ctx->idstr(), ctx->hint_slot, ctx->hint, ctx->more_hashs);
    return ret;
}

sdk_ret_t
Bucket::find_last_hint_(Apictx *ctx) {
    ctx->hint_slot = ctx->entry->find_last_hint();

    if (ctx->entry->is_hint_slot_valid(ctx->hint_slot) == false) {
        FTL_TRACE_VERBOSE("- No Valid Hint Found, hint_slot: %d, hint: %d", ctx->hint_slot, ctx->hint);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (ctx->hint_slot == ctx->entry->get_more_hint_slot()) {
        ctx->entry->get_hint_hash(ctx->hint_slot, ctx->hint, ctx->more_hashs);
    } else {
        ctx->entry->get_hint_hash(ctx->hint_slot, ctx->hint, ctx->hash_msbits);
    }

    FTL_TRACE_VERBOSE("Result = [ LastHint: Slot:%d Hint:%d ]",
                      ctx->hint_slot, ctx->hint);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// Bucket find_hint_: Finds a matching HINT slot.
//  - Returns SDK_RET_OK and hint_slot = 1 to N, if individual hints match.
//  - Returns SDK_RET_OK and hint_slot = HINT_SLOT_MORE, if more_hints == 1
//  - Returns SDK_RET_ENTRY_NOT_FOUND for all other cases.
//---------------------------------------------------------------------------
sdk_ret_t
Bucket::find_hint_(Apictx *ctx) {
    uint16_t hashX = 0;
    uint32_t hintX = 0;

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->props->num_hints) {
        ctx->entry->get_hint_hash(i, hintX, hashX);
        if (hashX == ctx->hash_msbits && HINT_IS_VALID(hintX)) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->match = 1;
            FTL_TRACE_VERBOSE("HintMatch: Hash:%x Slot:%d Hint:%d",
                                  hashX, i, hintX);
            return SDK_RET_OK;
        }
    }

    ctx->entry->get_hint_hash(Apictx::hint_slot::HINT_SLOT_MORE,
                             ctx->hint, ctx->more_hashs);
    if (ctx->more_hashs) {
        // If more_hashs is set, then it is still a match at this level, if we
        // dont treat this as a match, then it will try to allocate a hint at
        // this level, which is not correct.
        ctx->hint_slot = ctx->entry->get_more_hint_slot();
        ctx->match = 1;
        return SDK_RET_OK;
    }

    FTL_TRACE_VERBOSE("- No matching hint found.");
    return SDK_RET_ENTRY_NOT_FOUND;
}

//---------------------------------------------------------------------------
// Bucket find_: Find key match or hint slot in the bucket..
//---------------------------------------------------------------------------
sdk_ret_t
Bucket::find_(Apictx *ctx) {
    bool match = false;

    // Compare the Key portion, if it matches, then we have to re-align
    // the entries.
    // TODO

    //char buff[500];
    //(ctx->params->entry)->tostr(buff, 500);
    //FTL_TRACE_VERBOSE("Key :%s", buff);
    match = ctx->entry->compare_key(ctx->params->entry);
    if (match) {
        ctx->match = 1; ctx->exmatch = 1;
        return SDK_RET_OK;
    }
    FTL_TRACE_VERBOSE("Not an exact match, searching hints.");
    return find_hint_(ctx);
}

//---------------------------------------------------------------------------
// Bucket clear_hint_: Clear hint and hash
//---------------------------------------------------------------------------
sdk_ret_t
Bucket::clear_hint_(Apictx *ctx) {
    ctx->entry->set_hint_hash(ctx->hint_slot, 0, 0);
    HINT_SET_INVALID(ctx->hint);
    ctx->write_pending = true;
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// Bucket remove_: Remove an entry from the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
Bucket::remove_(Apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    if (!valid_) {
        FTL_TRACE_ERR("%s: failed to remove, entry not found Ctx: [%s]",
                      ctx->idstr(), ctx->metastr());
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = find_(ctx);
    if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("%s: failed to find match. Ctx:[%s], ret:%d",
                      ctx->idstr(), ctx->metastr(), ret);
        return ret;
    }

    FTL_TRACE_VERBOSE("%s: find_ result ret:%d exact_match:%d Ctx:[%s]",
                      ctx->idstr(), ret, ctx->exmatch, ctx->metastr());

    // If it is not an exact match, then no further processing is required
    // at this stage.
    if (!ctx->exmatch) {
        return ret;
    }

    // This is an exact match entry, clear the key and data fields.
    ctx->entry->clear_key_data();
    ctx->write_pending = true;

    // find the last valid hint for defragmentation
    ret = find_last_hint_(ctx);
    if (ret == SDK_RET_ENTRY_NOT_FOUND) {
        // This bucket has no hints, we can now write to HW and
        // finish the remove processing.
        valid_ = false;
        ret = write_(ctx);
        if (ret != SDK_RET_OK) {
            FTL_TRACE_ERR("HW write failed. ret:%d", ret);
            return ret;
        }
        ret = SDK_RET_OK;
        // Since this bucket has no hints, we can update stats here.
        // If it had hints, then it would be update during defragmentation
        FTL_TRACE_VERBOSE("decrementing table stats for idstr:%s, metastr:%s",
                          ctx->idstr(), ctx->metastr());

        ctx->tstats->remove(ctx->level);
    } else if (ret != SDK_RET_OK) {
        FTL_TRACE_ERR("find_last_hint_ failed. ret:%d", ret);
    }

    return ret;
}

//---------------------------------------------------------------------------
// Bucket mvkey_: Move key+data from src bucket to dst bucket
//---------------------------------------------------------------------------
sdk_ret_t
Bucket::move_(Apictx *dst, Apictx *src) {
    // NOTE NOTE NOTE:
    // This function will be called for 'dst' bucket context.
    SDK_ASSERT(this == dst->bucket);

    auto sbkt = static_cast<Bucket *>(src->bucket);

    // Copy key and data
    dst->entry->copy_key_data(src->entry);
    // dst node is now dirty, set write pending
    dst->write_pending = true;

    // Zero out src Key and Data
    src->entry->clear_key_data();

    FTL_TRACE_VERBOSE("- moved key and data");
    PRINT_API_CTX("MOVE-DST", dst);

    // Source bucket is now ready to be deleted
    FTL_TRACE_VERBOSE("- invalidate tail node");
    sbkt->valid_ = false;
    src->write_pending = true;
    PRINT_API_CTX("MOVE-SRC", src);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// Bucket mvkey_: Move key from src bucket to dst bucket
//---------------------------------------------------------------------------
sdk_ret_t
Bucket::delink_(Apictx *ctx) {
    sdk_ret_t ret = SDK_RET_OK;

    // Clear the hint linkage from the parent context
    SDK_ASSERT(ctx);
    ret = clear_hint_(ctx);
    SDK_ASSERT(ret == SDK_RET_OK);
    FTL_TRACE_VERBOSE("- cleared tail node hint link from parent node");
    PRINT_API_CTX("DELINK", ctx);
    return ret;
}

//---------------------------------------------------------------------------
// Bucket defragment_: Defragment the bucket
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
Bucket::defragment_(Apictx *ectx, Apictx *tctx) {
    sdk_ret_t ret = SDK_RET_OK;
    handle_t old_handle;
    handle_t new_handle;

    // Get parent context from the tail node context
    auto pctx = tctx->pctx;
    SDK_ASSERT(pctx);

    PRINT_API_CTX("ECTX", ectx);
    PRINT_API_CTX("PCTX", pctx);
    PRINT_API_CTX("TCTX", tctx);

    // notify the application whenever handle changes for an entry.
    // notification is sent twice - once before move and once after move.
    // reason for 2 notifications is application can invoke its locking
    // mechanism before start move and unlock after move.
    if ((ectx != tctx) && ectx->params->movecb) {
        tctx->is_main() ? old_handle.pindex(tctx->table_index) :
                          old_handle.sindex(tctx->table_index);
        ectx->is_main() ? new_handle.pindex(ectx->table_index) :
                          new_handle.sindex(ectx->table_index);
        ectx->params->movecb(tctx->entry, old_handle, new_handle, false);
    }

    // STEP 2: Move tctx key+data to ectx key+data
    // Need to check because, we can be deleting a tail node itself
    // if ectx == tctx, so nothing to move, but using this to 
    // clear the entry.
    ret = move_(ectx, tctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    // STEP 3: Write ectx to HW
    if (ectx != pctx) {
        ret = ectx->bucket->write_(ectx);
        SDK_ASSERT(ret == SDK_RET_OK);
    }

    // STEP 4: Delink parent node
    ret = pctx->bucket->delink_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    // STEP 5: Write pctx to HW
    ret = pctx->bucket->write_(pctx);
    SDK_ASSERT(ret == SDK_RET_OK);

    if (ectx != tctx) {
        // STEP 6: Write tctx to HW
        ret = tctx->bucket->write_(tctx);
        SDK_ASSERT(ret == SDK_RET_OK);

        // We always update the stats using the level of the tail context, this
        // is to make sure the 'hints' in table stats are accurate.
        // Without this, following will be the problem scenario,
        // when we remove an entry from main table, we decrement the stats using
        // the main table level (0), however after defragmentation, we will move
        // some hint to this entry, but we never account that stats.
        tctx->tstats->remove(!tctx->is_main());
        FTL_TRACE_VERBOSE("decrementing table stats for %s", tctx->idstr());

        // second notification - move complete
        if (ectx->params->movecb) {

            // Note: tctx->entry by this time has been zeroed out by move_().
            // However, it has been copied to ectx but, if ectx itself had gone
            // thru write_() above, it would have been byte swizzled. So to provide
            // correct context to the 2nd move step, ectx has to be re-read.
            ret = ectx->bucket->read_(ectx, true);
            SDK_ASSERT(ret == SDK_RET_OK);
            ectx->params->movecb(ectx->entry, old_handle, new_handle, true);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
Bucket::iterate_(Apictx *ctx) {
    bool force_hwread = ctx->params->force_hwread;

    ctx->entry->set_entry_valid(false);
    if (valid_ || force_hwread) {
        sdk_table_api_params_t params = { 0 };
        read_(ctx, force_hwread);
        // Set the Handle
        if (ctx->is_main()) {
            params.handle.pindex(ctx->table_index);
        } else {
            params.handle.sindex(ctx->table_index);
        }
        params.entry = ctx->entry;
        params.cbdata = ctx->params->cbdata;
        ctx->params->itercb(&params);
    }
    return SDK_RET_OK;
}
