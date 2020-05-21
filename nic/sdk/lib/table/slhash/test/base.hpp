//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __MEM_HASH_GTEST_BASE_HPP__
#define __MEM_HASH_GTEST_BASE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/slhash/slhash.hpp"
#include "lib/table/slhash/test/p4pd_mock/slhash_p4pd_mock.hpp"

#define USE_HANDLE true
#define USE_KEY false

#define SET_HASH true
#define CLR_HASH false

using sdk::table::slhash;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;
using sdk::table::handle_t;
using sdk::table::sdk_table_api_op_t;

#define SLHASH_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

typedef struct SlhashCacheEntry_ {
    slhash_mock_table_entry_t table_entry;
    handle_t handle;
} SlhashCacheEntry;

static inline char*
slhash_key2str(void *key)
{
    static char str[512];
    slhash_table_key_t *swkey = (slhash_table_key_t*)key;
    sprintf(str, "k1:%#x,k2:%#x", swkey->k1, swkey->k2);
    return str;
}

static inline char*
slhash_data2str(void *data)
{
    static char str[512];
    slhash_table_actiondata_t *acdata = (slhash_table_actiondata_t*)data;
    slhash_table_info_t *info = (slhash_table_info_t*)(&acdata->action_u.info);
    snprintf(str, 512, "d1:%#x,d2:%#x,epoch1:%d,epoch2:%d,v1:%d,v2:%d",
             info->d1, info->d2, info->epoch1, info->epoch2,
             info->valid1, info->valid2);
    return str;
}


class SlhashCache {
private:
    SlhashCacheEntry entries[2*TABLE_SIZE]; // Hash + TCAM entries
    uint32_t count_;

public:
    SlhashCache() { reset(); }
    ~SlhashCache() {}
    void reset() { memset(entries, 0, sizeof(entries)); }
    uint32_t count() { return count_; }

    slhash_mock_table_entry_t* get(uint32_t index, 
                                   sdk_table_api_params_t *params,
                                   bool use_handle_or_key, bool set_or_clr_hash) {
        entries[index].table_entry.key.k1 = index+1;
        entries[index].table_entry.key.k2 = index+1;
        entries[index].table_entry.mask.k1 = 0xFFFFFFFF;
        entries[index].table_entry.mask.k2 = 0xFFFFFFFF;
        entries[index].table_entry.data.action_u.info.epoch1 = index+1;
        entries[index].table_entry.data.action_u.info.valid1 = 1;
        count_ = index+1;

        if (params) {
            memset(params, 0, sizeof(sdk_table_api_params_t));
            params->key = &entries[index].table_entry.key;
            params->mask = &entries[index].table_entry.mask;
            params->appdata = &entries[index].table_entry.data;
            params->handle = (use_handle_or_key == USE_HANDLE) ? entries[index].handle
                                                               : SDK_TABLE_HANDLE_INVALID;
            if (set_or_clr_hash == SET_HASH) {
                params->hash_32b = index + 1;
                params->hash_valid = true;
            }
        }
        return &entries[index].table_entry;
    }
    handle_t gethandle(uint32_t index) { return entries[index].handle; } 
    void sethandle(uint32_t index, handle_t handle) { entries[index].handle = handle; } 
};

class SlhashGtestBase: public ::testing::Test {
protected:
    SlhashCache cache;
    slhash *table;
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
    SlhashGtestBase() {}
    virtual ~SlhashGtestBase() {}
    
    virtual void SetUp() {
        SDK_TRACE_DEBUG("============== SETUP : %s.%s ===============",
                        ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                        ::testing::UnitTest::GetInstance()->current_test_info()->name());

        sdk_table_factory_params_t params = { 0 };
        params.table_id = SLHASH_TABLE_ID_MOCK;
        params.key2str = slhash_key2str;
        params.mask2str = slhash_key2str;
        params.appdata2str = slhash_data2str;
        params.entry_trace_en = true;
        table = slhash::factory(&params);
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

        slhash_mock_init();
        PrintStats();
    }

    virtual void TearDown() {
        cache.reset();
        ValidateStats();
        table->sanitize();
        slhash::destroy(table);
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
                          uint32_t count, bool use_handle_or_key, bool set_or_clr_hash) {
        sdk_ret_t rs = sdk::SDK_RET_OK;
        sdk_table_api_params_t params = { 0 };
        for (auto i = 0; i < count; i++) {
            cache.get(i, &params, use_handle_or_key, set_or_clr_hash);
            switch (op) {
            case sdk::table::SDK_TABLE_API_INSERT: 
                rs = insert_(&params);
                SLHASH_CHECK_RETURN(rs == expret, rs);
                if (rs == sdk::SDK_RET_OK) {
                    assert(params.handle.valid());
                    cache.sethandle(i, params.handle);
                }
                break;
            case sdk::table::SDK_TABLE_API_REMOVE:
                rs = remove_(&params);
                SLHASH_CHECK_RETURN(rs == expret, rs);
                break;
            case sdk::table::SDK_TABLE_API_UPDATE:
                rs = update_(&params);
                SLHASH_CHECK_RETURN(rs == expret, rs);
                break;
            case sdk::table::SDK_TABLE_API_RESERVE:
                rs = reserve_(&params);
                SLHASH_CHECK_RETURN(rs == expret, rs);
                if (rs == sdk::SDK_RET_OK) {
                    assert(params.handle.valid());
                    cache.sethandle(i, params.handle);
                }
                break;
            case sdk::table::SDK_TABLE_API_RELEASE:
                rs = release_(&params);
                SLHASH_CHECK_RETURN(rs == expret, rs);
                break;
            default:
                SDK_ASSERT(0);
                break;
            }
        }
        return SDK_RET_OK;
    }

protected:
    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret, 
                     bool use_handle_or_key, bool set_or_clr_hash) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_INSERT, expret, 
                           count, use_handle_or_key, set_or_clr_hash);
    }

    sdk_ret_t Reserve(uint32_t count, sdk_ret_t expret, 
                      bool use_handle_or_key, bool set_or_clr_hash) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_RESERVE, expret, 
                           count, use_handle_or_key, set_or_clr_hash);
    }

    sdk_ret_t Release(uint32_t count, sdk_ret_t expret, 
                      bool use_handle_or_key, bool set_or_clr_hash) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_RELEASE, expret,
                           count, use_handle_or_key, set_or_clr_hash);
    }

    sdk_ret_t Remove(uint32_t count, sdk_ret_t expret,
                     bool use_handle_or_key, bool set_or_clr_hash) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_REMOVE, expret,
                           count, use_handle_or_key, set_or_clr_hash);
    }

    sdk_ret_t Update(uint32_t count, sdk_ret_t expret,
                     bool use_handle_or_key, bool set_or_clr_hash) {
        return ApiHandler_(sdk::table::SDK_TABLE_API_UPDATE, expret, 
                           count, use_handle_or_key, set_or_clr_hash);
    }

    sdk_ret_t GetHandler_(uint32_t count, sdk_ret_t expret,
                          bool use_handle_or_key, bool set_or_clr_hash) {
        sdk_table_api_params_t params = { 0 };
        slhash_mock_table_entry_t *cache_entry;
        slhash_mock_table_entry_t entry;
        
        for (auto i = 0; i < count; i++) {
            cache_entry = cache.get(i, NULL, use_handle_or_key,
                                    set_or_clr_hash);
            bzero(&entry, sizeof(entry));
            if (use_handle_or_key == USE_HANDLE) {
                params.handle = cache.gethandle(i);
            } else {
                memcpy(&entry.key, &cache_entry->key, sizeof(entry.key));
                params.key = &entry.key;
                memcpy(&entry.mask, &cache_entry->mask, sizeof(entry.mask));
                params.mask = &entry.mask;
                params.appdata = &entry.data;
                if (set_or_clr_hash == SET_HASH) {
                    params.hash_32b = i+1;
                    params.hash_valid = true;
                }
            }
            
            auto rs = get_(&params);
            SLHASH_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
            
            if (rs == sdk::SDK_RET_OK) {
                if (memcmp(&entry.data, &cache_entry->data, 
                           sizeof(entry.data))) {
                    SDK_TRACE_DEBUG("ERROR: Entry:%d Appdata mismatch.", i);
                    return sdk::SDK_RET_ENTRY_NOT_FOUND;
                }
            }
        }

        return SDK_RET_OK;
    }

    sdk_ret_t Get(uint32_t count, sdk_ret_t expret,
                  bool use_handle_or_key, bool set_or_clr_hash) {
        return GetHandler_(count, expret, use_handle_or_key, set_or_clr_hash);
    }

    void PrintStats() {
        table_count = slhash_mock_get_valid_count(SLHASH_TABLE_ID_MOCK);

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
        SLHASH_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t TxnEnd(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        rs = table->txn_end();
        SLHASH_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        return sdk::SDK_RET_OK;
    }

    static void
    IterateCallback(sdk_table_api_params_t *params) {
        SDK_TRACE_DEBUG("Entry[%p] Key=[%s] Mask=[%s] Data=[%s]", &params->handle,
                        slhash_key2str(params->key), slhash_key2str(params->mask),
                        slhash_data2str(params->appdata));
        return;
    }

    sdk_ret_t Iterate() {
        sdk_table_api_params_t params = { 0 };
        params.itercb = IterateCallback;
        return table->iterate(&params);
    }
};
#endif
