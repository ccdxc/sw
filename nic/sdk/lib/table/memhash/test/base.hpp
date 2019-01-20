//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __MEM_HASH_GTEST_BASE_HPP__
#define __MEM_HASH_GTEST_BASE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/table/memhash/test/p4pd_mock/mem_hash_p4pd_mock.hpp"

#include "common.hpp"

using sdk::table::mem_hash;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::mem_hash_factory_params_t;

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
    sdk_table_api_stats_t stats;
    sdk_table_stats_t table_stats;

protected:
    MemHashGtestBase() {}
    virtual ~MemHashGtestBase() {}
    
    virtual void SetUp() {
        mem_hash_factory_params_t params = { 0 };
        
        params.table_id = MEM_HASH_P4TBL_ID_H5;
        params.num_hints = 5;
        params.max_recircs = MAX_RECIRCS;
        params.key2str = h5_key2str;
        params.appdata2str = h5_appdata2str;

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
        memset(&stats, 0, sizeof(stats));
        memset(&table_stats, 0, sizeof(table_stats));

        printf("============== SetUp ===============\n");
        PrintStats();
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
        h5_reset_cache();
        printf("============== TearDown ===============\n");
        ValidateStats();
        mem_hash_mock_cleanup();
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
            rs = insert ? insert_(&params) : reserve_(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
            
            assert(params.handle != 0);
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

        assert(params.handle != 0);
        entry->handle = params.handle;

        for (ml = 1; ml < more_levels+1; ml++) {
            for (h = 0; h < max_hints; h++) {
                crc32.hint += 1;
                for (i = 0; i < max_recircs - ml; i++) {
                    entry = h5_gen_cache_entry(&crc32, &params);
                    rs = insert ? insert_(&params) : reserve_(&params);
                    MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);
                    assert(params.handle != 0);
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

    sdk_ret_t RemoveAllCached(sdk_ret_t expret, bool with_handle) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_cache_entry(i, &params, with_handle);
            
            rs = remove_(&params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
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
                printf("ERROR: Entry:%d Appdata mismatch.\n", i);
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }
        }

        return SDK_RET_OK;
    }


    void PrintStats() {
        mtable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5);
        htable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5_OHASH);

        table->stats_get(&stats, &table_stats);
        printf("HW Table Stats: Entries:%d Collisions:%d\n",
               (mtable_count + htable_count), htable_count);
        printf("SW Table Stats: Entries=%d Collisions:%d\n",
                table_stats.entries, table_stats.collisions);
        
        printf("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d\n",
                num_insert, num_update, num_get, num_remove, num_reserve, num_release);
        printf("Table API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d\n",
                stats.insert, stats.update, stats.get, stats.remove, stats.reserve, stats.release);
        return;    
    }

    sdk_ret_t ValidateStats() {
        PrintStats();
        EXPECT_TRUE(stats.insert >= stats.remove);
        EXPECT_EQ(mtable_count + htable_count, table_stats.entries);
        EXPECT_EQ(htable_count, table_stats.collisions);
        EXPECT_EQ(num_insert, stats.insert + stats.insert_duplicate + stats.insert_fail);
        EXPECT_EQ(num_remove, stats.remove + stats.remove_not_found + stats.remove_fail);
        EXPECT_EQ(num_update, stats.update + stats.update_fail);
        EXPECT_EQ(num_get, stats.get + stats.get_fail);
        EXPECT_EQ(num_reserve, stats.reserve + stats.reserve_fail);
        EXPECT_EQ(num_release, stats.release + stats.release_fail);
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
};
#endif
