//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_STATS_HPP__
#define __MEM_HASH_STATS_HPP__

#include "include/sdk/base.hpp"
#include <string>

using namespace std;

namespace sdk {
namespace table {
namespace memhash {

class mem_hash_api_stats {
private:
    uint32_t    insert_;
    uint32_t    dup_insert_;
    uint32_t    insert_fail_;
    uint32_t    remove_;
    uint32_t    dup_remove_;
    uint32_t    remove_fail_;
    uint32_t    update_;
    uint32_t    update_fail_;

public:
    mem_hash_api_stats() {
        insert_ = 0;
        dup_insert_ = 0;
        insert_fail_ = 0;
        remove_ = 0;
        dup_remove_ = 0;
        remove_fail_ = 0;
        update_ = 0;
        update_fail_ = 0;
    }

    ~mem_hash_api_stats() {
    }

    sdk_ret_t insert(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            insert_++;
        } else if (status == SDK_RET_ENTRY_EXISTS) {
            dup_insert_++;
        } else {
            insert_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            update_++;
        } else {
            update_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            remove_++;
        } else if (status == SDK_RET_ENTRY_NOT_FOUND) {
            dup_remove_++;
        } else {
            remove_fail_++;
        }
        return SDK_RET_OK;
    }
};

class mem_hash_table_stats {
private:
    uint32_t    entries_;
    uint32_t    hints_;
    uint32_t    avg_hint_level_;
    uint32_t    max_hint_level_;

public:
    mem_hash_table_stats() {
        entries_ = 0;
        hints_ = 0;
        avg_hint_level_ = 0;
        max_hint_level_ = 0;
    }

    ~mem_hash_table_stats() {
    }

    sdk_ret_t insert(uint32_t level) {
        SDK_ASSERT(entries_ && hints_);
        entries_++;
        if (level) {
            hints_++;
            if (level > max_hint_level_) {
                max_hint_level_ = level;
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove(uint32_t level) {
        SDK_ASSERT(entries_ && hints_);
        entries_--;
        if (level) {
            hints_--;
        }
        return SDK_RET_OK;
    }
};

} // namespace memhash
} // namespace table
} // namespace sdk

#endif // __MEM_HASH_HPP__
