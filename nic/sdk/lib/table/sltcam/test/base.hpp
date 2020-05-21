//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __MEM_HASH_GTEST_BASE_HPP__
#define __MEM_HASH_GTEST_BASE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/sltcam/sltcam.hpp"
#include "lib/table/sltcam/test/p4pd_mock/sltcam_p4pd_mock.hpp"

using sdk::table::sltcam;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;
using sdk::table::handle_t;
using sdk::table::sdk_table_api_op_t;

#define SLTCAM_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

typedef struct SltcamCacheEntry_ {
    sltcam_mock_table_entry_t table_entry;
    handle_t handle;
} SltcamCacheEntry;

static inline char*
sltcam_key2str(void *key)
{
    static char str[512];
    sltcam_table_key_t *swkey = (sltcam_table_key_t*)key;
    sprintf(str, "V:%#x,K:%#x", swkey->entry_inactive, swkey->k);
    return str;
}

static inline char*
sltcam_data2str(void *data)
{
    static char str[512];
    sltcam_table_actiondata_t *acdata = (sltcam_table_actiondata_t*)data;
    sprintf(str, "D:%#x", acdata->action_u.info.d);
    return str;
}


class SltcamCache {
private:
    SltcamCacheEntry entries[SLTCAM_MOCK_TABLE_SIZE];
    uint32_t count_;

public:
    SltcamCache() { reset(); }
    ~SltcamCache() {}
    void reset() { memset(entries, 0, sizeof(entries)); }
    uint32_t count() { return count_; }

    sltcam_mock_table_entry_t* get(uint32_t index, 
                                   sdk_table_api_params_t *params,
                                   bool with_handle) {
        entries[index].table_entry.key.entry_inactive = 0;
        entries[index].table_entry.key.k = index+1;
        entries[index].table_entry.mask.entry_inactive_mask = 0xFF;
        entries[index].table_entry.mask.m = 0xFFFFFFFF;
        entries[index].table_entry.data.action_u.info.d = index+1;
        count_ = index+1;

        if (params) {
            params->key = &entries[index].table_entry.key;
            params->mask = &entries[index].table_entry.mask;
            params->appdata = &entries[index].table_entry.data;
            params->handle = with_handle ? entries[index].handle : SDK_TABLE_HANDLE_INVALID;
        }
        return &entries[index].table_entry;
    }
    handle_t gethandle(uint32_t index) { return entries[index].handle; } 
    void sethandle(uint32_t index, handle_t handle) { entries[index].handle = handle; } 
};

class SltcamGtestBase: public ::testing::Test {
protected:
    SltcamCache cache;
    sltcam *table;
    uint32_t num_insert;
    uint32_t num_remove;
    uint32_t num_update;
    uint32_t num_reserve;
    uint32_t num_release;
    uint32_t num_get;
    
    uint32_t table_count;
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

protected:
    SltcamGtestBase() {}
    virtual ~SltcamGtestBase() {}
    
    virtual void SetUp() {
        SDK_TRACE_DEBUG("============== SETUP : %s.%s ===============",
                        ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                        ::testing::UnitTest::GetInstance()->current_test_info()->name());

        sdk_table_factory_params_t params = { 0 };
        params.table_id = SLTCAM_TABLE_ID_MOCK;
        params.key2str = sltcam_key2str;
        params.mask2str = sltcam_key2str;
        params.appdata2str = sltcam_data2str;
        params.entry_trace_en = true;
        table = sltcam::factory(&params);
        assert(table);
        
        num_insert = 0;
        num_remove = 0;
        num_update = 0;
        num_reserve = 0;
        num_release = 0;
        num_get = 0;

        table_count = 0;
        memset(&api_stats, 0, sizeof(api_stats));
        memset(&table_stats, 0, sizeof(table_stats));

        sltcam_mock_init();
        PrintStats();
    }

    virtual void TearDown() {
        cache.reset();
        ValidateStats();
        table->sanitize();
        sltcam::destroy(table);
        SDK_TRACE_DEBUG("============== TEARDOWN : %s.%s ===============",
                        ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                        ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

private:
    sdk_ret_t insert_(sdk_table_api_params_t *params) {
        num_insert++;
        return table->insert(params);
    }

    sdk_ret_t reserve_(sdk_table_api_params_t *params) {
        num_reserve++;
        return table->reserve(params);
    }
    
    sdk_ret_t release_(sdk_table_api_params_t *params) {
        num_release++;
        return table->release(params);
    }

    sdk_ret_t remove_(sdk_table_api_params_t *params) {
        num_remove++;
        return table->remove(params);
    }

    sdk_ret_t update_(sdk_table_api_params_t *params) {
        num_update++;
        return table->update(params);
    }

    sdk_ret_t get_(sdk_table_api_params_t *params) {
        num_get++;
        return table->get(params);
    }

    sdk_ret_t ApiHandler_(sdk_table_api_op_t op, sdk_ret_t expret,
                          uint32_t count, bool with_handle) {
        sdk_ret_t rs = sdk::SDK_RET_OK;
        sdk_table_api_params_t params = { 0 };
        for (auto i = 0; i < count; i++) {
            cache.get(i, &params, with_handle);
            switch (op) {
            case sdk::table::SDK_TABLE_API_INSERT: 
                rs = insert_(&params);
                SLTCAM_CHECK_RETURN(rs == expret, rs);
                if (rs == sdk::SDK_RET_OK) {
                    assert(params.handle.valid());
                    cache.sethandle(i, params.handle);
                }
                break;
            case sdk::table::SDK_TABLE_API_REMOVE:
                rs = remove_(&params);
                SLTCAM_CHECK_RETURN(rs == expret, rs);
                break;
            case sdk::table::SDK_TABLE_API_UPDATE:
                rs = update_(&params);
                SLTCAM_CHECK_RETURN(rs == expret, rs);
                break;
            case sdk::table::SDK_TABLE_API_RESERVE:
                rs = reserve_(&params);
                SLTCAM_CHECK_RETURN(rs == expret, rs);
                if (rs == sdk::SDK_RET_OK) {
                    assert(params.handle.valid());
                    cache.sethandle(i, params.handle);
                }
                break;
            case sdk::table::SDK_TABLE_API_RELEASE:
                rs = release_(&params);
                SLTCAM_CHECK_RETURN(rs == expret, rs);
                break;
            default:
                SDK_ASSERT(0);
                break;
            }
        }
        return SDK_RET_OK;
    }

protected:
    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_INSERT, expret, count, false);
    }

    sdk_ret_t Reserve(uint32_t count, sdk_ret_t expret) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_RESERVE, expret, count, false);
    }

    sdk_ret_t Release(uint32_t count, sdk_ret_t expret) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_RELEASE, expret, count, true);
    }

    sdk_ret_t Remove(uint32_t count, sdk_ret_t expret) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_REMOVE, expret, count, false);
    }

    sdk_ret_t Update(uint32_t count, sdk_ret_t expret) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_UPDATE, expret, count, false);
    }

    sdk_ret_t InsertAllCached(sdk_ret_t expret, bool with_handle) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_INSERT, expret, cache.count(), with_handle);
    }

    sdk_ret_t UpdateAllCached(sdk_ret_t expret, bool with_handle) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_UPDATE, expret, cache.count(), with_handle);
    }

    sdk_ret_t RemoveAllCached(sdk_ret_t expret, bool with_handle) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_REMOVE, expret, cache.count(), with_handle);
    }

    sdk_ret_t ReleaseAllCached(sdk_ret_t expret, bool with_handle) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_RELEASE, expret, cache.count(), with_handle);
    }

    sdk_ret_t GetAllCached(sdk_ret_t expret, bool with_handle) {
        sdk_table_api_params_t params = { 0 };
        sltcam_mock_table_entry_t *cache_entry;
        sltcam_mock_table_entry_t entry;
        
        for (auto i = 0; i < cache.count(); i++) {
            cache_entry = cache.get(i, NULL, with_handle);
            bzero(&entry, sizeof(entry));
            if (with_handle) {
                params.handle = cache.gethandle(i);
            } else {
                // Copy Key and Mask only for get by key
                memcpy(&entry.key, &cache_entry->key, sizeof(entry.key));
                memcpy(&entry.mask, &cache_entry->mask, sizeof(entry.mask));
            }
            // Set the Key, Data and Mask pointers
            params.appdata = &entry.data;
            params.key = &entry.key;
            params.mask = &entry.mask;
            
            auto rs = get_(&params);
            SLTCAM_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);

            if (memcmp(&entry.data, &cache_entry->data, 
                       sizeof(entry.data))) {
                SDK_TRACE_DEBUG("ERROR: Entry:%d Appdata mismatch.", i);
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }
        }

        return SDK_RET_OK;
    }


    void PrintStats() {
        table_count = sltcam_mock_get_valid_count(SLTCAM_TABLE_ID_MOCK);

        table->stats_get(&api_stats, &table_stats);
        SDK_TRACE_DEBUG("HW Table Stats: Entries:%d", table_count);
        SDK_TRACE_DEBUG("SW Table Stats: Entries=%lu", table_stats.entries);

        SDK_TRACE_DEBUG("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                        num_insert, num_update, num_get, num_remove, num_reserve, num_release);
        SDK_TRACE_DEBUG("Table API Stats: Insert=%lu Update=%lu Get=%lu Remove:%lu Reserve:%lu Release:%lu",
                        api_stats.insert + api_stats.insert_duplicate + api_stats.insert_fail,
                        api_stats.update + api_stats.update_fail,
                        api_stats.get + api_stats.get_fail,
                        api_stats.remove + api_stats.remove_not_found + api_stats.remove_fail,
                        api_stats.reserve + api_stats.reserve_fail,
                        api_stats.release + api_stats.release_fail);
        return;
    }

    sdk_ret_t ValidateStats() {
        PrintStats();
        EXPECT_TRUE(api_stats.insert >= api_stats.remove);
        EXPECT_EQ(table_count, table_stats.entries);
        EXPECT_EQ(num_insert, api_stats.insert + api_stats.insert_duplicate + api_stats.insert_fail);
        EXPECT_EQ(num_remove, api_stats.remove + api_stats.remove_not_found + api_stats.remove_fail);
        EXPECT_EQ(num_update, api_stats.update + api_stats.update_fail);
        EXPECT_EQ(num_get, api_stats.get + api_stats.get_fail);
        EXPECT_EQ(num_reserve, api_stats.reserve + api_stats.reserve_fail);
        EXPECT_EQ(num_release, api_stats.release + api_stats.release_fail);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t TxnStart(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        rs = table->txn_start();
        SLTCAM_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t TxnEnd(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        rs = table->txn_end();
        SLTCAM_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        return sdk::SDK_RET_OK;
    }

    static void
    IterateCallback(sdk_table_api_params_t *params) {
        SDK_TRACE_DEBUG("Entry[%p] Key=[%s] Mask=[%s] Data=[%s]", &params->handle,
                        sltcam_key2str(params->key), sltcam_key2str(params->mask),
                        sltcam_data2str(params->appdata));
        return;
    }

    sdk_ret_t Iterate() {
        sdk_table_api_params_t params = { 0 };
        params.itercb = IterateCallback;
        return table->iterate(&params);
    }
};
#endif
