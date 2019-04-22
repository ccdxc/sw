//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __FTL_GTEST_BASE_HPP__
#define __FTL_GTEST_BASE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "nic/utils/ftl/ftl.hpp"
#include "nic/utils/ftl/test/p4pd_mock/ftl_p4pd_mock.hpp"

#include "common.hpp"

#define WITH_HASH true
#define WITHOUT_HASH false

using sdk::table::ftl;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

class FtlGtestBase: public ::testing::Test {
protected:
    ftl *table;
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
    FtlGtestBase() {}
    virtual ~FtlGtestBase() {}
    
    virtual void SetUp() {
        SDK_TRACE_VERBOSE("============== SETUP : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
        sdk_table_factory_params_t params = { 0 };
        
        params.table_id = FTL_TBLID_H5;
        params.num_hints = 5;
        params.max_recircs = MAX_RECIRCS;
        params.entry_trace_en = false;

        ftl_mock_init();
        table = ftl::factory(&params);
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
        PrintStats();
    }
    virtual void TearDown() {
        ftl::destroy(table);
        reset_cache();
        ValidateStats();
        ftl_mock_cleanup();
        SDK_TRACE_VERBOSE("============== TEARDOWN : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

private:
    sdk_ret_t insert_(sdk_table_api_params_t *params) {
        num_insert++;
        return table->insert(params);
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

protected:
    sdk_ret_t Insert(uint32_t count, sdk_ret_t expret, 
                     bool with_hash = false) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash);
            auto rs = insert_(params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
            if (rs == SDK_RET_OK) {
                assert(params->handle.valid());
                table_count++;
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t Remove(uint32_t count, sdk_ret_t expret,
                     bool with_hash = false) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash);
            auto rs = remove_(params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
            if (rs == SDK_RET_OK) {
                table_count--;
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t Update(uint32_t count, sdk_ret_t expret,
                     bool with_hash = false) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash);
            auto rs = update_(params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t Get(uint32_t count, sdk_ret_t expret,
                  bool with_hash = false) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash);
            auto params2 = gen_entry(i, with_hash);
            auto rs = get_(params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
            assert(params->handle.valid());
            if (memcmp(params->entry, params2->entry, sizeof(ftl_entry_t))) {
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }
        }
        return SDK_RET_OK;
    }

    void PrintStats() {
        table->stats_get(&api_stats, &table_stats);
        SDK_TRACE_VERBOSE("GTest Table Stats: Entries:%d", table_count);
        SDK_TRACE_VERBOSE("SW Table Stats: Entries=%d Collisions:%d",
                          table_stats.entries, table_stats.collisions);
        
        SDK_TRACE_VERBOSE("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                          num_insert, num_update, num_get, num_remove, num_reserve, num_release);
        SDK_TRACE_VERBOSE("Table API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                          api_stats.insert, api_stats.update, api_stats.get,
                          api_stats.remove, api_stats.reserve, api_stats.release);
        return;    
    }

    sdk_ret_t ValidateStats() {
        PrintStats();
        EXPECT_TRUE(api_stats.insert >= api_stats.remove);
        EXPECT_EQ(table_count, table_stats.entries + table_stats.collisions);
        EXPECT_EQ(num_insert, api_stats.insert + api_stats.insert_duplicate + api_stats.insert_fail);
        EXPECT_EQ(num_remove, api_stats.remove + api_stats.remove_not_found + api_stats.remove_fail);
        EXPECT_EQ(num_update, api_stats.update + api_stats.update_fail);
        EXPECT_EQ(num_get, api_stats.get + api_stats.get_fail);
        EXPECT_EQ(num_reserve, api_stats.reserve + api_stats.reserve_fail);
        EXPECT_EQ(num_release, api_stats.release + api_stats.release_fail);
        return sdk::SDK_RET_OK;
    }

    static void
    IterateCallback(sdk_table_api_params_t *params) {
        static char buff[512];
        FTLENTRY_STR(((ftl_entry_t *)params->entry), buff, 512);
        SDK_TRACE_VERBOSE("Handle[%s] Entry[%s]",
                          params->handle.tostr(), buff);
        return;
    }

    sdk_ret_t Iterate() {
        sdk_table_api_params_t params = { 0 };
        params.itercb = IterateCallback;
        return table->iterate(&params);
    }
};
#endif
