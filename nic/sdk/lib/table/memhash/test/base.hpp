//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __MEM_HASH_GTEST_BASE_HPP__
#define __MEM_HASH_GTEST_BASE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "lib/table/memhash/mem_hash.hpp"
#include "lib/table/memhash/test/p4pd_mock/mem_hash_p4pd_mock.hpp"

#include "common.hpp"

using sdk::table::mem_hash;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

class MemHashGtestBase: public ::testing::Test {
protected:
    mem_hash *table;
    uint32_t num_insert;
    uint32_t num_remove;
    uint32_t num_update;
    uint32_t num_reserve;
    uint32_t num_release;
    uint32_t num_get;
    
    uint32_t mtable_count;
    uint32_t htable_count;
    sdk_table_api_stats_t api_stats;
    sdk_table_stats_t table_stats;

protected:
    MemHashGtestBase() {}
    virtual ~MemHashGtestBase() {}
    
    virtual void SetUp() {
        SDK_TRACE_VERBOSE("============== SETUP : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
        sdk_table_factory_params_t params = { 0 };
        
        params.table_id = MEM_HASH_P4TBL_ID_H5;
        params.num_hints = 5;
        params.max_recircs = MAX_RECIRCS;
        params.key2str = h5_key2str;
        params.appdata2str = h5_appdata2str;
#ifdef PERF_DBG
        params.entry_trace_en = false;
#else
        params.entry_trace_en = true;
#endif

        table = mem_hash::factory(&params);
        assert(table);
        mem_hash_mock_init();

        num_insert = 0;
        num_remove = 0;
        num_update = 0;
        num_reserve = 0;
        num_release = 0;
        num_get = 0;

        mtable_count = 0;
        htable_count = 0;
        memset(&api_stats, 0, sizeof(api_stats));
        memset(&table_stats, 0, sizeof(table_stats));
        PrintStats();
    }
    virtual void TearDown() {
        ValidateStats();
        mem_hash::destroy(table);
        h5_reset_cache();
        mem_hash_mock_cleanup();
        SDK_TRACE_VERBOSE("============== TEARDOWN : %s.%s ===============",
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

    sdk_ret_t InsertOrReserve_(bool insert,
                               uint32_t count,
                               sdk_ret_t expret) {
        sdk_table_api_params_t params = { 0 };
        uint32_t i = 0;
        sdk_ret_t rs;
        h5_entry_t *entry = NULL;

        for (i = 0; i < count; i++) {
            entry = h5_gen_cache_entry(NULL, &params, false);
#ifdef PERF_DBG
            params.hash_32b = count % (1024*256);
            params.hash_valid = true;
#endif
            rs = insert ? insert_(&params) : reserve_(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
            
            assert(params.handle.valid());
            entry->handle = params.handle;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t InsertOrReserveWithHash_(bool insert,
                                       uint32_t more_levels,
                                       uint32_t max_hints,
                                       uint32_t max_recircs) {
        sdk_ret_t rs;
        h5_crc32_t crc32;
        uint32_t i = 0;
        uint32_t h = 0;
        uint32_t ml = 0;
        sdk_table_api_params_t params = { 0 };
        h5_entry_t *entry = NULL;
        
        crc32.val = h5_gencrc32();
        entry = h5_gen_cache_entry(&crc32, &params);
        
        rs = insert ? insert_(&params) : reserve_(&params);
        MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);

        assert(params.handle.valid());
        entry->handle = params.handle;

        for (ml = 1; ml < more_levels+1; ml++) {
            for (h = 0; h < max_hints; h++) {
                crc32.hint += 1;
                for (i = 0; i < max_recircs - ml; i++) {
                    entry = h5_gen_cache_entry(&crc32, &params);
                    rs = insert ? insert_(&params) : reserve_(&params);
                    MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);
                    assert(params.handle.valid());
                    entry->handle = params.handle;
                }
            }
        }
        return SDK_RET_OK;
    }

protected:
    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret) {
        return InsertOrReserve_(true, count, expret);
    }

    sdk_ret_t Reserve(uint32_t count, sdk_ret_t expret) {
        return InsertOrReserve_(false, count, expret);
    }

    sdk_ret_t Remove(uint32_t count, sdk_ret_t expret) {
        sdk_table_api_params_t params = { 0 };
        uint32_t i = 0;
        sdk_ret_t rs;

        for (i = 0; i < count; i++) {
            h5_gen_cache_entry(NULL, &params, false);
            rs = remove_(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t InsertWithHash(uint32_t more_levels,
                             uint32_t max_hints = 5,
                             uint32_t max_recircs = 8) {
        return InsertOrReserveWithHash_(true, more_levels,
                                        max_hints, max_recircs);
    }

    sdk_ret_t ReserveWithHash(uint32_t more_levels,
                              uint32_t max_hints = 5,
                              uint32_t max_recircs = 8) {
        return InsertOrReserveWithHash_(false, more_levels,
                                        max_hints, max_recircs);
    }

    sdk_ret_t InsertAllCached(sdk_ret_t expret, bool with_handle) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_cache_entry(i, &params, with_handle);
            rs = insert_(&params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t UpdateAllCached(sdk_ret_t expret, bool with_handle) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_updated_cache_entry(i, &params, with_handle);
            rs = update_(&params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t RemoveAllCached(sdk_ret_t expret, bool with_handle, bool validate_data = false) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_cache_entry(i, &params, with_handle);
            
            rs = remove_(&params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
            if (validate_data) {
                for (auto e = i+1; e < h5_get_cache_count(); e++) {
                    __get_one(e, SDK_RET_OK, false);
                }
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t ReleaseAllCached(sdk_ret_t expret, bool with_handle) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_cache_entry(i, &params, with_handle);
            rs = release_(&params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    void __get_one(uint32_t entry_index, sdk_ret_t expret, bool with_handle) {
        sdk_ret_t rs;
        sdk_table_api_params_t params = { 0 };
        h5_entry_t *cache_entry;
        h5_entry_t entry;
        
        cache_entry = h5_get_cache_entry(entry_index, NULL, with_handle);
        bzero(&entry, sizeof(h5_entry_t));
        memcpy(&entry.key, &cache_entry->key, sizeof(entry.key));
        params.key = &entry.key;
        params.appdata = &entry.appdata;
        params.hash_32b = cache_entry->crc32.val;
        params.hash_valid = cache_entry->crc32.val != 0;
        
        rs = get_(&params);
        if (rs != expret) {
            SDK_TRACE_ERR("Retcode:%d Expected:%d", rs, expret);
            assert(0);
        }

        if (memcmp(&entry.appdata, &cache_entry->appdata, 
                   sizeof(entry.appdata))) {
            SDK_TRACE_VERBOSE("ERROR: Entry:%d Appdata mismatch.", entry_index);
            SDK_TRACE_VERBOSE("Expected Appdata: %s", h5_appdata2str(&cache_entry->appdata));
            SDK_TRACE_VERBOSE("  Actual Appdata: %s", h5_appdata2str(&entry.appdata));
            assert(0);
            return;
        }
    }

    sdk_ret_t GetAllCached(sdk_ret_t expret, bool with_handle) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };
        h5_entry_t *cache_entry;
        h5_entry_t entry;
        
        for (i = 0; i < h5_get_cache_count(); i++) {
            cache_entry = h5_get_cache_entry(i, NULL, with_handle);
            bzero(&entry, sizeof(h5_entry_t));
            memcpy(&entry.key, &cache_entry->key, sizeof(entry.key));
            params.key = &entry.key;
            params.appdata = &entry.appdata;
            
            rs = get_(&params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);

            if (memcmp(&entry.appdata, &cache_entry->appdata, 
                       sizeof(entry.appdata))) {
                SDK_TRACE_VERBOSE("ERROR: Entry:%d Appdata mismatch.", i);
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }

            SDK_ASSERT(params.handle == cache_entry->handle);
        }

        return SDK_RET_OK;
    }

    void PrintStats() {
#ifndef PERF_DBG
        mtable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5);
        htable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5_OHASH);

        table->stats_get(&api_stats, &table_stats);
        SDK_TRACE_VERBOSE("HW Table Stats: Entries:%d Collisions:%d",
               (mtable_count + htable_count), htable_count);
        SDK_TRACE_VERBOSE("SW Table Stats: Entries=%lu Collisions:%lu",
                table_stats.entries, table_stats.collisions);

        SDK_TRACE_VERBOSE("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                          num_insert, num_update, num_get, num_remove, num_reserve, num_release);
        SDK_TRACE_VERBOSE("Table API Stats: Insert=%lu Update=%lu Get=%lu Remove:%lu Reserve:%lu Release:%lu",
                          api_stats.insert, api_stats.update, api_stats.get,
                          api_stats.remove, api_stats.reserve, api_stats.release);
#endif
        return;
    }

    sdk_ret_t ValidateStats() {
#ifndef PERF_DBG
        EXPECT_TRUE(table->state_validate() == sdk::SDK_RET_OK);
        PrintStats();
        EXPECT_TRUE(api_stats.insert >= api_stats.remove);
        EXPECT_EQ(mtable_count + htable_count, table_stats.entries);
        EXPECT_EQ(htable_count, table_stats.collisions);
        EXPECT_EQ(num_insert, api_stats.insert + api_stats.insert_duplicate + api_stats.insert_fail);
        EXPECT_EQ(num_remove, api_stats.remove + api_stats.remove_not_found + api_stats.remove_fail);
        EXPECT_EQ(num_update, api_stats.update + api_stats.update_fail);
        EXPECT_EQ(num_get, api_stats.get + api_stats.get_fail);
        EXPECT_EQ(num_reserve, api_stats.reserve + api_stats.reserve_fail);
        EXPECT_EQ(num_release, api_stats.release + api_stats.release_fail);
#endif
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t TxnStart(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        rs = table->txn_start();
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        return sdk::SDK_RET_OK;
    }

    sdk_ret_t TxnEnd(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        rs = table->txn_end();
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        return sdk::SDK_RET_OK;
    }

    static void
    IterateCallback(sdk_table_api_params_t *params) {
        SDK_TRACE_VERBOSE("Entry[%p] Key=[%s] Data=[%s]", &params->handle,
               h5_key2str(params->key), h5_appdata2str(params->appdata));
        return;
    }

    sdk_ret_t Iterate() {
        sdk_table_api_params_t params = { 0 };
        params.itercb = IterateCallback;
        return table->iterate(&params);
    }
};
#endif
