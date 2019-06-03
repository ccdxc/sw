//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "include/sdk/mem.hpp"
#include "slhash_table.hpp"
#include "slhash_utils.hpp"
#include "slhash_api_context.hpp"

namespace sdk {
namespace table {
namespace slhash_internal {

sdk_ret_t
table::init(uint32_t max) {
    max_ = max;
    buckets_ = (bucket *)SDK_CALLOC(SDK_MEM_TYPE_SLHASH_BUCKETS,
                                max_ * sizeof(bucket));
    if (buckets_ == NULL) {
        return sdk::SDK_RET_OOM;
    }
    SLHASH_TRACE_DEBUG("max buckets:%d", max_);
    return sdk::SDK_RET_OK;
}


sdk_ret_t
table::find(slhctx &ctx) {
__label__ done;
    sdk::sdk_ret_t ret = sdk::SDK_RET_OK;
    if (ctx.inhandle().valid()) {
        if (ctx.inhandle().svalid()) {
            // Secondary index is valid for this handle,
            // then the entry will be present in TCAM
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
        if (!buckets_[ctx.index].isreserved()) {
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
        return SDK_RET_OK;
    } else {
        // Calculate the hash
        ret = ctx.calchash();
        SLHASH_RET_CHECK_AND_GOTO(ret, done, "calchash, r:%d", ret);
    }

    if (!buckets_[ctx.index].isbusy()) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    ret = ctx.read();
    SDK_ASSERT(ret == sdk::SDK_RET_OK);

    if (ctx.keycompare() == 0) {
        return sdk::SDK_RET_OK;
    }

done:
    return sdk::SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
table::insert(slhctx &ctx) {
    // index must be valid by now
    SDK_ASSERT(ctx.index_valid);
    auto &bkt = buckets_[ctx.index];

    // Use the bucket if it is available
    if (bkt.isused()) {
        // Bucket is already used.
        return sdk::SDK_RET_COLLISION;
    }

    // Allocate this bucket
    bkt.alloc();

    // Write the bucket to HW
    ctx.write();

    return sdk::SDK_RET_OK;
}

sdk_ret_t
table::update(slhctx &ctx) {
    // index must be valid by now
    SDK_ASSERT(ctx.index_valid);
    // bucket must be valid for update
    SDK_ASSERT(buckets_[ctx.index].isused());

    // Write the bucket to HW
    ctx.write();

    return sdk::SDK_RET_OK;
}

sdk_ret_t
table::remove(slhctx &ctx) {
    // index must be valid by now
    SDK_ASSERT(ctx.index_valid);
    auto &bkt = buckets_[ctx.index];

    // If this bucket is not valid, then this entry is not present.
    if (!bkt.isused()) {
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    // Make sure the key matches the entry
    SDK_ASSERT(ctx.keycompare() == 0);
   
    // Write the bucket to HW
    ctx.write();

    // Free this bucket
    bkt.free();

    return sdk::SDK_RET_OK;
}

sdk_ret_t
table::reserve(slhctx &ctx) {
    // index must be valid by now
    SDK_ASSERT(ctx.index_valid);
    auto &bkt = buckets_[ctx.index];

    // Use the bucket if it is available
    if (bkt.isbusy()) {
        // Bucket is already used.
        return sdk::SDK_RET_COLLISION;
    }

    // Reserve this bucket
    bkt.reserve();
    return sdk::SDK_RET_OK;
}

sdk_ret_t
table::release(slhctx &ctx) {
    // index must be valid by now
    SDK_ASSERT(ctx.index_valid);
    auto &bkt = buckets_[ctx.index];

    // Use the bucket if it is available
    if (bkt.isreserved() == false) {
        // Bucket is already used.
        return sdk::SDK_RET_ENTRY_NOT_FOUND;
    }

    // Allocate this bucket
    bkt.release();

    return sdk::SDK_RET_OK;
}

} // namespace slhash_internal
} // namespace table
} // namespace sdk
