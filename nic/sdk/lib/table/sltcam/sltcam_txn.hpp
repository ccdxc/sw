//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SLTCAM_TXN_HPP__
#define __SLTCAM_TXN_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/indexer/indexer.hpp"
#include <string>

#include "sltcam_api_context.hpp"
#include "sltcam_utils.hpp"

using namespace std;

namespace sdk {
namespace table {
namespace sltcam_internal {

class txn {
private:
    bool valid_;
    uint32_t reserved_count_;
    uint32_t table_size_ = 0;
    sdk::lib::indexer *indexer_;

private:
    sdk_ret_t alloc_(sltctx *ctx) {
        indexer::status irs;
        SDK_ASSERT(ctx->tcam_index_valid);

        if (ctx->params->num_handles) {
            irs = indexer_->alloc_withid(ctx->tcam_index,
                                         ctx->tcam_index_valid);
        } else {
            irs = indexer_->alloc_withid(ctx->tcam_index);
        }
        if (irs != indexer::SUCCESS) {
            return sdk::SDK_RET_ENTRY_EXISTS;
        }
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t dealloc_(sltctx *ctx) {
        SDK_ASSERT(ctx->tcam_index_valid);
        indexer::status irs;

        if (ctx->params->num_handles) {
            for (uint32_t i = 0; i < ctx->params->num_handles; i++) {
                irs = indexer_->free(ctx->tcam_index + i);
                if (irs != indexer::SUCCESS) {
                    SLTCAM_TRACE_ERR("Failed to free index %u",
                                     ctx->tcam_index + i);
                    // continue to free other indices as much as possible
                }
            }
        } else {
            irs = indexer_->free(ctx->tcam_index);
        }
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
            SLTCAM_TRACE_ERR("Failed to create indexer_");
            return sdk::SDK_RET_OOM;
        }
        return sdk::SDK_RET_OK;
    }

    bool valid() {
        return valid_;
    }

    sdk_ret_t validate(sltctx *ctx) {
        if (!valid()) {
            return sdk::SDK_RET_OK;
        }
        if (ctx->op == sdk::table::SDK_TABLE_API_INSERT ||
            ctx->op == sdk::table::SDK_TABLE_API_REMOVE) {
            if (!ctx->params->handle.valid()) {
                return sdk::SDK_RET_INVALID_ARG;
            }
        }
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t start() {
        if (valid()) {
            SLTCAM_TRACE_ERR("transaction already in progress");
            return sdk::SDK_RET_TXN_EXISTS;
        }

        if (reserved_count_ != 0) {
            SLTCAM_TRACE_ERR("transaction incomplete");
            return sdk::SDK_RET_TXN_INCOMPLETE;
        }

        valid_ = true;
        reserved_count_ = 0;
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t end() {
        if (!valid()) {
            SLTCAM_TRACE_ERR("transaction not started");
            return sdk::SDK_RET_TXN_NOT_FOUND;
        }

        if (reserved_count_ != 0) {
            SLTCAM_TRACE_ERR("transaction incomplete, rsvd count = %d",
                             reserved_count_);
            return sdk::SDK_RET_TXN_INCOMPLETE;
        }
        valid_ = false;
        reserved_count_ = 0;
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t reserve(sltctx *ctx) {
        if (!valid()) {
            return sdk::SDK_RET_TXN_NOT_FOUND;
        }

        auto ret = alloc_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            return ret;
        }
        reserved_count_++;
        SLTCAM_TRACE_DEBUG("txn: reserved count = %d", reserved_count_);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t release(sltctx *ctx) {
        if (!valid()) {
            return sdk::SDK_RET_TXN_NOT_FOUND;
        }

        auto ret = dealloc_(ctx);
        if (ret != sdk::SDK_RET_OK) {
            return ret;
        }

        SDK_ASSERT(reserved_count_);
        reserved_count_--;
        SLTCAM_TRACE_DEBUG("txn: reserved count = %d", reserved_count_);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t validate() {
        if (valid() && reserved_count_ != 0) {
            return SDK_RET_TXN_INCOMPLETE;
        }
        return SDK_RET_OK;
    }
};

} // namespace sltcam_internal
} // namespace table
} // namespace sdk

#endif // __SLTCAM_TXN_HPP__
