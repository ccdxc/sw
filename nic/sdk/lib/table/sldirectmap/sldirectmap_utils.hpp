//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __SDK_LIB_TABLE_SLDIRECTMAP_UTILS_HPP__
#define __SDK_LIB_TABLE_SLDIRECTMAP_UTILS_HPP__

namespace sdk {
namespace table {

//#define DIRECTMAP_TRACES_ENABLE 1
#ifdef DIRECTMAP_TRACES_ENABLE
#define DIRECTMAP_TRACE_VERBOSE(_msg, _args...) SDK_TRACE_VERBOSE(_msg, ##_args)
#define DIRECTMAP_TRACE_DEBUG(_msg, _args...) SDK_TRACE_DEBUG(_msg, ##_args)
#define PRINT_DATA(fn, data, index)  entry_trace_(fn, data, index)
#else
#define DIRECTMAP_TRACE_VERBOSE(_msg, _args...)
#define DIRECTMAP_TRACE_DEBUG(_msg, _args...)
#define PRINT_DATA(fn, data, index)
#endif
#define DIRECTMAP_TRACE_ERR(_msg, _args...) SDK_TRACE_ERR(_msg, ##_args)

class sldirectmap_api_stats {
private:
    uint32_t insert_;
    uint32_t insert_withid_;
    uint32_t insert_atid_;
    uint32_t insert_duplicate_;
    uint32_t insert_fail_;
    uint32_t insert_withid_fail_;
    uint32_t insert_atid_fail_;
    uint32_t remove_;
    uint32_t remove_not_found_;
    uint32_t remove_fail_;
    uint32_t update_;
    uint32_t update_fail_;
    uint32_t get_;
    uint32_t get_fail_;
    uint32_t reserve_;
    uint32_t reserve_fail_;
    uint32_t release_;
    uint32_t release_fail_;
    uint32_t reserve_index_;
    uint32_t reserve_index_fail_;

public:
    sldirectmap_api_stats() {
        insert_ = 0;
        insert_withid_ = 0;
        insert_atid_ = 0;
        insert_duplicate_ = 0;
        insert_fail_ = 0;
        insert_withid_fail_ = 0;
        insert_atid_fail_ = 0;
        remove_ = 0;
        remove_not_found_ = 0;
        remove_fail_ = 0;
        update_ = 0;
        update_fail_ = 0;
        get_ = 0;
        get_fail_ = 0;
        reserve_ = 0;
        reserve_fail_ = 0;
        release_ = 0;
        release_fail_ = 0;
        reserve_index_ = 0;
        reserve_index_fail_ = 0;
    }

    ~sldirectmap_api_stats() {
    }

    sdk_ret_t insert(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating insert stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            insert_++;
        } else if (status == SDK_RET_ENTRY_EXISTS) {
            insert_duplicate_++;
        } else {
            insert_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t insert_withid(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating insert stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            insert_withid_++;
        } else if (status == SDK_RET_ENTRY_EXISTS) {
            insert_duplicate_++;
        } else {
            insert_withid_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t insert_atid(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating insert stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            insert_atid_++;
        } else if (status == SDK_RET_ENTRY_EXISTS) {
            insert_duplicate_++;
        } else {
            insert_atid_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating update stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            update_++;
        } else {
            update_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating remove stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            remove_++;
        } else if (status == SDK_RET_ENTRY_NOT_FOUND) {
            remove_not_found_++;
        } else {
            remove_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t reserve(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating reserve stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            reserve_++;
        } else {
            reserve_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t reserve_index(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating reserve stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            reserve_index_++;
        } else {
            reserve_index_fail_++;
        }
        return SDK_RET_OK;
    }


    sdk_ret_t release(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating release stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            release_++;
        } else {
            release_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk_ret_t status) {
        DIRECTMAP_TRACE_VERBOSE("Updating get stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            get_++;
        } else {
            get_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk_table_api_stats_t *stats) {
        stats->insert = insert_ + insert_withid_ + insert_atid_;
        stats->insert_duplicate = insert_duplicate_;
        stats->insert_fail = insert_fail_ + insert_withid_fail_ + insert_atid_fail_;
        stats->remove = remove_;
        stats->remove_not_found = remove_not_found_;
        stats->remove_fail = remove_fail_;
        stats->update = update_;
        stats->update_fail = update_fail_;
        stats->get = get_;
        stats->get_fail = get_fail_;
        stats->reserve = reserve_ + reserve_index_;
        stats->reserve_fail = reserve_fail_ + reserve_index_fail_;
        stats->release = release_;
        stats->release_fail = release_fail_;
        return SDK_RET_OK;
    }
};

}   // namespace table
}   // namespace sdk

#endif   // __SDK_LIB_TABLE_SLDIRECTMAP_UTILS_HPP__
