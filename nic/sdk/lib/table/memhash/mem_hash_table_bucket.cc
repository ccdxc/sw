//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "include/sdk/base.hpp"
#include "lib/p4/p4_api.hpp"

#include "mem_hash.hpp"
#include "mem_hash_types.hpp"
#include "mem_hash_table_bucket.hpp"

using sdk::table::memhash::mem_hash_table_bucket;
using sdk::table::memhash::mem_hash_api_context;

#define CTX_IS_SLOT_MATCH(_ctx) \
        ((_ctx)->hint_slot != HINT_SLOT_FREE && (_ctx)->hint_slot != HINT_SLOT_MORE)

#define FOREACH_HINT(__tid) \
        for (uint32_t i = 1; i <= p4pd_mem_hash_entry_get_num_hints((__tid)); i++)
#define FOREACH_HINT_REVERSE(__tid) \
        for (i = p4pd_mem_hash_entry_get_num_hints(__tid); i > 0; i--)

//---------------------------------------------------------------------------
// mem_hash_table_bucket read_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::read_(mem_hash_api_context *ctx) {   
    p4pd_error_t    pdret = 0;

    assert(ctx->table_id);
    assert(ctx->table_index);

    pdret = p4pd_entry_read(ctx->table_id, ctx->table_index,
                            ctx->swkey, NULL, ctx->swdata);
    SDK_TRACE_DEBUG("%s: HW Read: TableID:%d TableIndex:%d", ctx->idstr(),
                    ctx->table_id, ctx->table_index);
    SDK_TRACE_DEBUG("- Key:[%s]", ctx->swkey_str());
    SDK_TRACE_DEBUG("- Data:[%s]", ctx->swdata_str());

    return (pdret != P4PD_SUCCESS) ? SDK_RET_HW_READ_ERR : SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket write_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::write_(mem_hash_api_context *ctx) {   
    p4pd_error_t    pdret = 0;

    if (ctx->write_pending == false) {
        return SDK_RET_OK;
    }

    assert(ctx->table_id);
    assert(ctx->table_index);
    
    p4pd_mem_hash_entry_set_entry_valid(ctx->table_id, ctx->swdata, valid_);

    if (ctx->hint != HINT_INDEX_INVALID) {
        if (ctx->hint_slot == HINT_SLOT_MORE) {
            p4pd_mem_hash_entry_set_more_hashs(ctx->table_id, ctx->swdata, 1);
            p4pd_mem_hash_entry_set_more_hints(ctx->table_id, ctx->swdata,
                                               ctx->hint);
        } else {
            p4pd_mem_hash_entry_set_hint(ctx->table_id, ctx->swdata,
                                         ctx->hint, ctx->hint_slot);
            p4pd_mem_hash_entry_set_hash(ctx->table_id, ctx->swdata,
                                         ctx->hash, ctx->hint_slot);
        }
    }

    SDK_TRACE_DEBUG("%s: HW Write: TableID:%d TableIndex:%d", ctx->idstr(),
                    ctx->table_id, ctx->table_index);
    SDK_TRACE_DEBUG("- Key:[%s]", ctx->swkey_str());
    SDK_TRACE_DEBUG("- Data:[%s]", ctx->swdata_str());

    pdret = p4pd_entry_write(ctx->table_id, ctx->table_index,
                             ctx->swkey, NULL, ctx->swdata);
    if (pdret != P4PD_SUCCESS) {
        SDK_TRACE_ERR("HW write: ret:%d", pdret);
        // Write failure is fatal
        assert(0);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    ctx->write_pending = false;

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_key_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_key_(mem_hash_api_context *ctx) {
    p4pd_mem_hash_entry_set_key(ctx->table_id, ctx->swkey, (uint8_t*)ctx->key);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket set_data_
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::set_data_(mem_hash_api_context *ctx) {
    p4pd_mem_hash_entry_set_action_id(ctx->table_id, ctx->swdata, 0);
    p4pd_mem_hash_entry_set_entry_valid(ctx->table_id, ctx->swdata, 1);
    p4pd_mem_hash_entry_set_data(ctx->table_id, ctx->swdata,
                                 (uint8_t *)ctx->data);
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket create_ : Create a new bucket and add an entry
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::create_(mem_hash_api_context *ctx) {
    //uint32_t    i = 0;
    sdk_ret_t   ret = SDK_RET_OK;
    
    SDK_TRACE_DEBUG("%s: Creating new bucket.", ctx->idstr());
    SDK_TRACE_DEBUG("- Meta: [%s]", ctx->metastr());
    
    // This is a new entry, key is present with the entry.
    ret = set_key_(ctx);
    assert(ret == SDK_RET_OK);
    
    // Fill common data
    ret = set_data_(ctx);
    assert(ret == SDK_RET_OK);

    // Update the bucket meta data
    valid_ = true;
    if (ctx->ismain()) {
        SDK_TRACE_DEBUG("- Type = SPINE");
        type_ = bucket_type::BUCKET_TYPE_SPINE;
    } else {
        if (CTX_IS_SLOT_MATCH(ctx)) {
            SDK_TRACE_DEBUG("- Type = SPINE");
            type_ = bucket_type::BUCKET_TYPE_SPINE;
        } else {
            SDK_TRACE_DEBUG("- Type = CHAIN");
            type_ = bucket_type::BUCKET_TYPE_CHAIN;
        }
    }

    // New entry, write required.
    ctx->write_pending = true;

    // Write to hardware
    return ret;
}

sdk_ret_t
mem_hash_table_bucket::compare_(mem_hash_api_context *ctx) {
    uint32_t    hashX = 0;
    uint32_t    hintX = 0;
    
    // There are 3 possible cases from here
    // 1) There is a matching 'hash' with a valid 'hint' (non-zero hint)
    // 2) There is free 'hash' slot, 'hint' needs to be allocated.
    // 3) All 'hash' slots are full, we have to continue to 'more_hints'

    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->table_id) {
        hashX = p4pd_mem_hash_entry_get_hash(ctx->table_id, ctx->swdata, i);
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->swdata, i);
        if (hashX == ctx->hash && hintX != HINT_INDEX_INVALID) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            SDK_TRACE_DEBUG("%s: Match: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            break;
        } else if (hintX == HINT_INDEX_INVALID &&
                   ctx->hint_slot == HINT_SLOT_FREE) {
            // CASE 2: Save the firstfree slots
            SDK_TRACE_DEBUG("%s: FreeSlot: Hash:%x Slot:%d Hint:%d",
                            ctx->idstr(), hashX, i, hintX);
            ctx->hint_slot = i;
            ctx->hint = HINT_INDEX_INVALID;
            // DO NOT BREAK HERE:
            // We have to match all the slots to see if any hash matches, 
            // if not, then use the empty slot to create a new hint chain.
            // This approach allows us to have holes in the slots. REVISIT TODO
        }
    }

    if (ctx->hint_slot != HINT_SLOT_FREE) {
        // We have hit Case1 or Case2.
        return SDK_RET_COLLISION;
    }

    ctx->more_hashs = p4pd_mem_hash_entry_get_more_hashs(ctx->table_id, ctx->swdata);
    ctx->hint = p4pd_mem_hash_entry_get_more_hints(ctx->table_id, ctx->swdata);
       
    return SDK_RET_COLLISION;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket append_: append an entry to the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::append_(mem_hash_api_context *ctx) {
    sdk_ret_t   ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("%s: Appending to bucket.", ctx->idstr());
    SDK_TRACE_DEBUG("- PreMeta : [%s]", ctx->metastr());

    ret = find_(ctx);
    if (ret == SDK_RET_OK) {
        SDK_ASSERT(ctx->match_type);
        SDK_TRACE_DEBUG("- PostMeta(find_): [%s]", ctx->metastr());
        // CASE: Either a exact match (EXM) or a hint matched
        if (ctx->match_type == mem_hash_api_context::match_type::MATCH_TYPE_EXM) {
            // CASE: if exact match, then its a duplicate insert
            SDK_TRACE_DEBUG("%s: Entry already exists.", ctx->idstr());
            return SDK_RET_ENTRY_EXISTS;
        } else if (ctx->match_type == mem_hash_api_context::match_type::MATCH_TYPE_HINT) {
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
// mem_hash_table_bucket find_first_free_hint_: Finds first free HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_first_free_hint_(mem_hash_api_context *ctx) {
    sdk_ret_t   ret = SDK_RET_OK;
    uint32_t    hintX = 0;

    FOREACH_HINT(ctx->table_id) {
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->swdata, i);
        if (hintX == HINT_INDEX_INVALID) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            break;
        }
    }

    if (ctx->hint_slot != HINT_SLOT_FREE) {
        // We have found a valid hint slot.
        SDK_TRACE_ERR("free hint slot found");
    } else {
        ctx->more_hashs = p4pd_mem_hash_entry_get_more_hashs(ctx->table_id, ctx->swdata);
        if (ctx->more_hashs == 0) {
            ctx->hint = p4pd_mem_hash_entry_get_more_hints(ctx->table_id, ctx->swdata);
            ctx->hint_slot = HINT_SLOT_MORE;
        } else {
            SDK_TRACE_ERR("all hint slots are full");
            ret = SDK_RET_NO_RESOURCE;
        }
    }

    SDK_TRACE_DEBUG("%s: FirstFreeHint: Slot:%d Hint:%d More:%d",
                    ctx->idstr(), ctx->hint_slot, ctx->hint, ctx->more_hashs);
    return ret;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_last_hint_: Finds last valid HINT slot.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_last_hint_(mem_hash_api_context *ctx) {
    uint32_t    i = 0;
    uint32_t    hintX = 0;
    FOREACH_HINT_REVERSE(ctx->table_id) {
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->swdata, i);
        if (hintX != HINT_INDEX_INVALID) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->match_type = mem_hash_api_context::match_type::MATCH_TYPE_HINT;
            SDK_TRACE_DEBUG("LastValidHint: Slot:%d Hint:%d",
                            ctx->hint_slot, ctx->hint);
            break;
        }
    }

    if (ctx->hint_slot == HINT_SLOT_FREE) {
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
    uint32_t    hashX = 0;
    uint32_t    hintX = 0;
    
    // Find a free hint slot in the bucket entry
    FOREACH_HINT(ctx->table_id) {
        hashX = p4pd_mem_hash_entry_get_hash(ctx->table_id, ctx->swdata, i);
        hintX = p4pd_mem_hash_entry_get_hint(ctx->table_id, ctx->swdata, i);
        if (hashX == ctx->hash && hintX != HINT_INDEX_INVALID) {
            ctx->hint_slot = i;
            ctx->hint = hintX;
            ctx->match_type = mem_hash_api_context::match_type::MATCH_TYPE_HINT;
            SDK_TRACE_DEBUG("HintMatch: Hash:%x Slot:%d Hint:%d",
                            hashX, i, hintX);
            return SDK_RET_OK;
        }
    }

    ctx->more_hashs = p4pd_mem_hash_entry_get_more_hashs(ctx->table_id, ctx->swdata);
    if (ctx->more_hashs) {
        // If more_hashs is set, then it is still a match at this level, if we
        // dont treat this as a match, then it will try to allocate a hint at 
        // this level, which is not correct.
        ctx->hint = p4pd_mem_hash_entry_get_more_hints(ctx->table_id, ctx->swdata);
        ctx->hint_slot = HINT_SLOT_MORE;
        ctx->match_type = mem_hash_api_context::match_type::MATCH_TYPE_HINT;
        return SDK_RET_OK;
    }
        
    SDK_TRACE_DEBUG("- No matching hint found.");
    return SDK_RET_ENTRY_NOT_FOUND;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_key_: Clears 'swkey' in the context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_key_(mem_hash_api_context *ctx) {
    p4pd_error_t    p4pdret = P4PD_SUCCESS;

    ctx->write_pending = true;

    p4pdret = p4pd_mem_hash_entry_clear_key(ctx->table_id, ctx->swkey);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket clear_key_: Clears 'swdata' in the context
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::clear_data_(mem_hash_api_context *ctx) {
    p4pd_error_t    p4pdret = P4PD_SUCCESS;

    ctx->write_pending = true;

    p4pdret = p4pd_mem_hash_entry_clear_data(ctx->table_id, ctx->swdata);
    SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);

    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket find_: Find key match or hint slot in the bucket..
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::find_(mem_hash_api_context *ctx) {
    sdk_ret_t   ret = SDK_RET_OK;
    bool        match = false;

    // First read from hardware
    ret = read_(ctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("%s: HW read entry. Ctx:[%s]",
                      ctx->idstr(), ctx->metastr());
        return ret;
    }

    // Compare the Key portion, if it matches, then we have to re-align
    // the entries.
    match = p4pd_mem_hash_entry_compare_key(ctx->table_id, ctx->swkey, 
                                            (uint8_t*)ctx->key);
    if (match) {
        ctx->match_type = mem_hash_api_context::match_type::MATCH_TYPE_EXM;
        return SDK_RET_OK;
    }

    return find_hint_(ctx);
}

//---------------------------------------------------------------------------
// mem_hash_table_bucket remove_: Remove an entry from the bucket.
//---------------------------------------------------------------------------
sdk_ret_t
mem_hash_table_bucket::remove_(mem_hash_api_context *ctx) {
    sdk_ret_t   ret = SDK_RET_OK;

    if (!valid_) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = find_(ctx);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to find match. ret:%d", ret);
        return ret;
    }

    if (ctx->match_type == mem_hash_api_context::match_type::MATCH_TYPE_EXM) {
        clear_key_(ctx);
        clear_data_(ctx);
        // find the last valid hint to move it to key slot
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
    }

    return ret;
}
