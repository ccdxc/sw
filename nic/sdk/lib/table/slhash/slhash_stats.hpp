//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SLHASH_STATS_HPP__
#define __SLHASH_STATS_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "string.h"
#include <string>

namespace sdk {
namespace table {
namespace slhash_internal {

class stats {
private:
    sdk::table::sdk_table_api_stats_t apistats_;
    sdk::table::sdk_table_stats_t tabstats_;

public:
    stats() {
        memset(&apistats_, 0, sizeof(apistats_));
        memset(&tabstats_, 0, sizeof(tabstats_));
    }

    ~stats() {
    }

    sdk_ret_t insert(sdk_ret_t status, sdk::table::handle_t hdl) {
        if (status == SDK_RET_OK) {
            apistats_.insert++;
            SDK_ASSERT(hdl.valid());
            if (hdl.svalid()) {
                tabstats_.collisions++;
            } else {
                tabstats_.entries++;
            }
        } else if (status == SDK_RET_ENTRY_EXISTS) {
            apistats_.insert_duplicate++;
        } else {
            apistats_.insert_fail++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            apistats_.update++;
        } else {
            apistats_.update_fail++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove(sdk_ret_t status, sdk::table::handle_t hdl) {
        if (status == SDK_RET_OK) {
            apistats_.remove++;
            SDK_ASSERT(hdl.valid());
            if (hdl.svalid()) {
                SDK_ASSERT(tabstats_.collisions);
                tabstats_.collisions--;
            } else {
                SDK_ASSERT(tabstats_.entries);
                tabstats_.entries--;
            }
        } else if (status == SDK_RET_ENTRY_NOT_FOUND) {
            apistats_.remove_not_found++;
        } else {
            apistats_.remove_fail++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t reserve(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            apistats_.reserve++;
        } else {
            apistats_.reserve_fail++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t release(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            apistats_.release++;
        } else {
            apistats_.release_fail++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk_ret_t status) {
        if (status == SDK_RET_OK) {
            apistats_.get++;
        } else {
            apistats_.get_fail++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk::table::sdk_table_api_stats_t *apistats,
                  sdk::table::sdk_table_stats_t *tabstats) {
        *apistats = apistats_;
        *tabstats = tabstats_;
        return SDK_RET_OK;
    }
};

} // namespace slhash_internal
} // namespace table
} // namespace sdk

#endif // __SLHASH_HPP__
