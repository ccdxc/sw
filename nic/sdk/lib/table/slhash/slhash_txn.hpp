//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SLHASH_TXN_HPP__
#define __SLHASH_TXN_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/indexer/indexer.hpp"
#include <string>

#include "slhash_api_context.hpp"
#include "slhash_utils.hpp"

using namespace std;

namespace sdk {
namespace table {
namespace slhash_internal {

class txn {
private:
    bool valid_;
    uint32_t reserved_count_;
    uint32_t table_size_ = 0;
    sdk::lib::indexer *indexer_;

private:
    sdk_ret_t alloc_(slhctx &ctx) {
        SDK_ASSERT(ctx.index_valid);
        indexer::status irs = indexer_->alloc_withid(ctx.index);
        if (irs != indexer::SUCCESS) {
            return sdk::SDK_RET_ENTRY_EXISTS;
        }
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t dealloc_(slhctx &ctx) {
        SDK_ASSERT(ctx.index_valid);
        indexer::status irs = indexer_->free(ctx.index);
        if (irs != indexer::SUCCESS) {
            return sdk::SDK_RET_ENTRY_NOT_FOUND;
        }
        return sdk::SDK_RET_OK;
    }

public:
    txn() {
        valid_ = false;
        reserved_count_ = 0;
        table_size_ = 0;
        indexer_ = NULL;
    }

    ~txn() {
        indexer::destroy(indexer_);
    }

    sdk_ret_t init(uint32_t table_size) {
        SDK_ASSERT(table_size);
        indexer_ = indexer::factory(table_size, false, false);
        if (indexer_ == NULL) {
            SLHASH_TRACE_ERR("Failed to create indexer_");
            return sdk::SDK_RET_OOM;
        }
        return sdk::SDK_RET_OK;
    }

    bool valid() {
        return valid_;
    }

    sdk_ret_t validate(slhctx &ctx) {
        if (!valid()) {
            return sdk::SDK_RET_OK;
        }
        if (ctx.op == sdk::table::SDK_TABLE_API_INSERT ||
            ctx.op == sdk::table::SDK_TABLE_API_REMOVE ||
            ctx.op == sdk::table::SDK_TABLE_API_RELEASE) {
            if (!ctx.inhandle().valid()) {
                return sdk::SDK_RET_INVALID_ARG;
            }
        }
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t start() {
        if (valid()) {
            SLHASH_TRACE_ERR("transaction already in progress");
            return sdk::SDK_RET_TXN_EXISTS;
        }

        if (reserved_count_ != 0) {
            SLHASH_TRACE_ERR("transaction incomplete");
            return sdk::SDK_RET_TXN_INCOMPLETE;
        }

        valid_ = true;
        reserved_count_ = 0;
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t end() {
        if (!valid()) {
            SLHASH_TRACE_ERR("transaction not started");
            return sdk::SDK_RET_TXN_NOT_FOUND;
        }

        if (reserved_count_ != 0) {
            SLHASH_TRACE_ERR("transaction incomplete, rsvd count = %d",
                             reserved_count_);
            return sdk::SDK_RET_TXN_INCOMPLETE;
        }
        valid_ = false;
        reserved_count_ = 0;
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t reserve(slhctx &ctx) {
        auto ret = alloc_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            return ret;
        }
        reserved_count_++;
        SLHASH_TRACE_DEBUG("txn: reserved count = %d", reserved_count_);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t release(slhctx &ctx) {
        if (!valid()) {
            return sdk::SDK_RET_TXN_NOT_FOUND;
        }

        auto ret = dealloc_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            return ret;
        }

        SDK_ASSERT(reserved_count_);
        reserved_count_--;
        SLHASH_TRACE_DEBUG("txn: reserved count = %d", reserved_count_);
        return sdk::SDK_RET_OK;
    }
};

} // namespace slhash_internal
} // namespace table
} // namespace sdk

#endif // __SLHASH_TXN_HPP__
