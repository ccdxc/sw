//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_TXN_HPP__
#define __MEM_HASH_TXN_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include <string>

using namespace std;

namespace sdk {
namespace table {
namespace memhash {

class mem_hash_txn {
private:
    bool valid_;
    uint32_t reserved_count_;

public:
    mem_hash_txn() {
        valid_ = false;
        reserved_count_ = 0;
    }

    ~mem_hash_txn() {
    }

    bool is_valid() {
        return valid_;
    }

    sdk_ret_t start() {
        if (valid_) {
            SDK_TRACE_ERR("transaction already in progress");
            return SDK_RET_TXN_EXISTS;
        }

        if (reserved_count_ != 0) {
            SDK_TRACE_ERR("previous transaction is incomplete");
            return SDK_RET_TXN_INCOMPLETE;
        }

        valid_ = true;
        reserved_count_ = 0;
        return SDK_RET_OK;
    }

    sdk_ret_t end() {
        if (valid_ == false) {
            SDK_TRACE_ERR("transaction not started");
            return SDK_RET_TXN_NOT_FOUND;
        }

        if (reserved_count_ != 0) {
            SDK_TRACE_ERR("trying to end incomplete transaction");
            return SDK_RET_TXN_INCOMPLETE;
        }
        valid_ = false;
        reserved_count_ = 0;
        return SDK_RET_OK;
    }

    sdk_ret_t reserve() {
        reserved_count_++;
        SDK_TRACE_DEBUG("txn: reserved count = %d", reserved_count_);
        return SDK_RET_OK;
    }

    sdk_ret_t release() {
        SDK_ASSERT(reserved_count_);
        reserved_count_--;
        SDK_TRACE_DEBUG("txn: reserved count = %d", reserved_count_);
        return SDK_RET_OK;
    }

    sdk_ret_t validate() {
        if (valid_ == false && reserved_count_ != 0) {
            return SDK_RET_TXN_INCOMPLETE;
        }
        return SDK_RET_OK;
    }
};

} // namespace memhash
} // namespace table
} // namespace sdk

#endif // __MEM_HASH_TXN_HPP__
