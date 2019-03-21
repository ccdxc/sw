//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "include/sdk/mem.hpp"
#include "sltcam_db.hpp"
#include "sltcam_utils.hpp"
#include "sltcam_api_context.hpp"

namespace sdk {
namespace table {
namespace sltcam_internal {

sdk_ret_t
db::init(uint32_t max_elems) {
    max_elems_ = max_elems;
    elems_ = (elem *)SDK_CALLOC(SDK_MEM_ALLOC_SLTCAM_TREE_NODES, 
                                max_elems * sizeof(elem));
    if (elems_ == NULL) {
        return sdk::SDK_RET_OOM;
    }
    SLTCAM_TRACE_DEBUG("db max_elems:%d", max_elems);
    return sdk::SDK_RET_OK;
}

#define MID(_lo, _hi) ((_hi)>=(_lo)) ? ((_hi)+(_lo))/2 : (_lo)
sdk_ret_t
db::findslot_(sltctx *ctx) {
__label__ done, notfound;
    int cmpresult = 0;
    uint32_t lo = 0;
    uint32_t hi = count_ ? count_-1 : 0;
    sdk::sdk_ret_t ret = sdk::SDK_RET_ERR;
    
    ctx->dbslot_valid = true;
    ctx->dbslot = MID(lo,hi); // Start from middle
    while(lo <= hi && elem_get_(ctx->dbslot)->valid()) {
        //SLTCAM_TRACE_VERBOSE("lo:%d hi:%d slot:%d", lo, hi, slot);
        cmpresult = ctx->swcompare(elem_get_(ctx->dbslot)->data());
        if (cmpresult == 0) {
            // Match
            ctx->tcam_index = elem_get_(ctx->dbslot)->data();
            ctx->tcam_index_valid = true;
            ret = sdk::SDK_RET_OK;
            goto done;
        } else if (cmpresult < 0) {
            // Lower bound
            if (ctx->dbslot == 0) {
                goto notfound;
            }
            hi = ctx->dbslot-1;
        } else {
            lo = ctx->dbslot+1;
        }
        ctx->dbslot = MID(lo,hi);
    }

notfound:
    //SLTCAM_TRACE_VERBOSE("not found");
    ret = sdk::SDK_RET_ENTRY_NOT_FOUND;

done:
    SLTCAM_TRACE_DEBUG("slot:%d, ret:%d", ctx->dbslot, ret);
    return ret;
}

void
db::sanitize(sltctx *ctx) {
    if (isempty()) {
        return;
    }
    for (uint32_t i = 0; i < count_-1; i++) {
        auto cmpresult = ctx->swcompare(elem_get_(i)->data(),
                                        elem_get_(i+1)->data());
        if (cmpresult >= 0) {
            SLTCAM_TRACE_DEBUG("ERROR: Sanity Check Failed: %d<-->%d = %d",
                            i, i+1, cmpresult);
            SDK_ASSERT(0);
        }
    }
    return;
}

sdk_ret_t
db::find(sltctx *ctx) {
    return findslot_(ctx);
}

sdk_ret_t
db::grow_(sltctx *ctx) {
    SDK_ASSERT(ctx->dbslot_valid);
    if (elem_get_(ctx->dbslot)->valid()) {
        memmove(elems_+ctx->dbslot+1, elems_+ctx->dbslot,
                (count_-ctx->dbslot)*sizeof(elem));
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
db::shrink_(sltctx *ctx) {
    if (isempty()) {
        return sdk::SDK_RET_OK;
    }
    SDK_ASSERT(ctx->dbslot_valid);
    SLTCAM_TRACE_VERBOSE("dbslot:%d count_:%d", ctx->dbslot, count_);
    memmove(elems_+ctx->dbslot, elems_+ctx->dbslot+1,
            (count_-ctx->dbslot-1)*sizeof(elem));
    return sdk::SDK_RET_OK;
}

sdk_ret_t
db::insert(sltctx *ctx) {
    // dbslot must be valid by now
    SDK_ASSERT(ctx->dbslot_valid);
    // Grow the DB for insertion
    SDK_ASSERT(grow_(ctx) == sdk::SDK_RET_OK);
    // Set the Data
    elem_get_(ctx->dbslot)->set(ctx->tcam_index);
    // Increment the count
    count_++;
    return sdk::SDK_RET_OK;
}

sdk_ret_t
db::remove(sltctx *ctx) {
    // dbslot must be valid by now
    SDK_ASSERT(ctx->dbslot_valid);
    // Delete the data
    elem_get_(ctx->dbslot)->clear();
    // Shrink the DB
    SDK_ASSERT(shrink_(ctx) == sdk::SDK_RET_OK);
    // Decrement the count
    SDK_ASSERT(count_);
    count_--;
    return sdk::SDK_RET_OK;
}


} // namespace sltcam_internal
} // namespace table
} // namespace sdk
