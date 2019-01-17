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
using sdk::table::mem_hash_factory_params_t;

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

class MemHashGtestBase: public ::testing::Test {
protected:
    mem_hash *table;
    uint32_t num_insert;
    uint32_t num_remove;
    uint32_t num_update;

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

        PrintStats();
    }
    virtual void TearDown() {
        mem_hash::destroy(table);
        h5_reset_cache();
        PrintStats();
        ValidateStats();
        mem_hash_mock_cleanup();
    }

    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret = SDK_RET_OK) {
        sdk_table_api_params_t params = { 0 };
        uint32_t i = 0;
        sdk_ret_t rs;

        for (i = 0; i < count; i++) {
            h5_gen_cache_entry(NULL, &params);
            rs = table->insert(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
            
            assert(params.handle != 0);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t Remove(uint32_t count, sdk_ret_t expret = SDK_RET_OK) {
        sdk_table_api_params_t params = { 0 };
        uint32_t i = 0;
        sdk_ret_t rs;

        for (i = 0; i < count; i++) {
            h5_gen_cache_entry(NULL, &params);
            rs = table->remove(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t InsertWithHash(uint32_t more_levels, uint32_t max_hints,
                             uint32_t max_recircs) {
        sdk_ret_t rs;
        h5_crc32_t crc32;
        uint32_t i = 0;
        uint32_t h = 0;
        uint32_t ml = 0;
        sdk_table_api_params_t params = { 0 };
        
        crc32.val = h5_gencrc32();
        h5_gen_cache_entry(&crc32, &params);

        rs = table->insert(&params);
        MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);

        for (ml = 1; ml < more_levels+1; ml++) {
            for (h = 0; h < max_hints; h++) {
                crc32.hint += 1;
                for (i = 0; i < max_recircs - ml; i++) {
                    h5_gen_cache_entry(&crc32, &params);

                    rs = table->insert(&params);
                    MHTEST_CHECK_RETURN(rs == sdk::SDK_RET_OK, rs);;
                }
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t InsertAllCached(sdk_ret_t expret = SDK_RET_OK) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_updated_cache_entry(i, &params);
            rs = table->insert(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t UpdateAllCached(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_updated_cache_entry(i, &params);
            rs = table->update(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t RemoveAllCached(sdk_ret_t expret = sdk::SDK_RET_OK) {
        sdk_ret_t rs;
        uint32_t i = 0;
        sdk_table_api_params_t params = { 0 };

        for (i = 0; i < h5_get_cache_count(); i++) {
            h5_get_cache_entry(i, &params);
            
            rs = table->remove(&params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
        }
        return SDK_RET_OK;
    }

    void PrintStats() {
        uint32_t mtable_count;
        uint32_t htable_count;
        sdk_table_api_stats_t stats;

        mtable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5);
        htable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5_OHASH);
        printf("Table Count, Main:%d Hint:%d\n", mtable_count, htable_count);

        table->getstats(&stats);
        printf("Stats: Insert=%d Remove:%d\n", stats.insert, stats.remove);
        return;    
    }

    sdk_ret_t ValidateStats() {
        uint32_t mtable_count;
        uint32_t htable_count;
        sdk_table_api_stats_t stats;

        mtable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5);
        htable_count = mem_hash_mock_get_valid_count(MEM_HASH_P4TBL_ID_H5_OHASH);

        printf("Table Count Main:%d Hint:%d\n", mtable_count, htable_count);
        
        table->getstats(&stats);
        printf("Stats: Insert=%d Remove:%d\n", stats.insert, stats.remove);

        SDK_ASSERT(stats.insert >= stats.remove);
        SDK_ASSERT((mtable_count + htable_count) == (stats.insert - stats.remove));
        return SDK_RET_OK;
    }

};

#endif
