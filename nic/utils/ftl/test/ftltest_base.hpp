//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __FTL_GTEST_BASE_HPP__
#define __FTL_GTEST_BASE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <cinttypes>

#include "include/sdk/base.hpp"
#include "include/sdk/table.hpp"
#include "p4pd_mock/ftl_p4pd_mock.hpp"
#include "gen/p4gen/p4/include/ftl.h"
#include "lib/table/ftl/ftl_base.hpp"
#include "gen/p4gen/p4/include/ftl_table.hpp"

#include "ftltest_common.hpp"

#define WITH_HASH true
#define WITHOUT_HASH false

using sdk::table::ftl_base;
using sdk::table::sdk_table_api_params_t;
using sdk::table::sdk_table_api_stats_t;
using sdk::table::sdk_table_stats_t;
using sdk::table::sdk_table_factory_params_t;

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

class ftl_test_base: public ::testing::Test {
protected:
    ftl_base *table;
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
    ftl_test_base() {}
    virtual ~ftl_test_base() {}
    
    virtual void SetUp() {
        SDK_TRACE_INFO("============== SETUP : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
        
        ftl_mock_init();
        
        sdk_table_factory_params_t params = { 0 };
        params.entry_trace_en = true;

#ifdef IRIS
        table = flow_hash_info::factory(&params);
#else
        table = flow_hash::factory(&params);
#endif
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
        print_stats();
    }
    virtual void TearDown() {
        validate_stats();
#ifdef IRIS
        flow_hash_info::destroy(table);
#else
        flow_hash::destroy(table);
#endif
        ftl_mock_cleanup();
        SDK_TRACE_INFO("============== TEARDOWN : %s.%s ===============",
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

public:
    sdk_ret_t insert(uint32_t count, sdk_ret_t expret,
                     bool with_hash = false, uint32_t hash_32b = 0) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash, hash_32b);
            auto rs = insert_(params);
            MHTEST_CHECK_RETURN(rs == expret, rs);
            if (rs == SDK_RET_OK) {
                assert(params->handle.valid());
                table_count++;
            }
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove_helper(uint32_t index, sdk_ret_t expret,
                           bool with_hash, uint32_t hash_32b) {
        auto params = gen_entry(index, with_hash, hash_32b);
        auto rs = remove_(params);
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        if (rs == SDK_RET_OK) {
            table_count--;
        }
        return rs;
    }

    sdk_ret_t remove(uint32_t count, sdk_ret_t expret,
                     bool with_hash = false, uint32_t hash_32b = 0) {
        for (auto i = 0; i < count; i++) {
            auto rs = remove_helper(i, expret, with_hash, hash_32b);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update(uint32_t count, sdk_ret_t expret,
                     bool with_hash = false, uint32_t hash_32b = 0) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash, hash_32b);
            auto rs = update_(params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(uint32_t count, sdk_ret_t expret,
                  bool with_hash = false) {
        for (auto i = 0; i < count; i++) {
            auto params = gen_entry(i, with_hash);
            auto params2 = gen_entry(i, with_hash);
            auto rs = get_(params);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
            assert(params->handle.valid());
            if (!params->entry->compare_key(params2->entry)) {
                return sdk::SDK_RET_ENTRY_NOT_FOUND;
            }
        }
        return SDK_RET_OK;
    }

    void print_stats() {
        //auto hw_count = ftl_mock_get_valid_count(FTL_TBLID_IPV6);
        table->stats_get(&api_stats, &table_stats);
        SDK_TRACE_INFO("GTest Table Stats: Entries:%d", table_count);
        //SDK_TRACE_INFO("HW Table Stats: Entries=%d", hw_count);
        SDK_TRACE_INFO("SW Table Stats: Entries=%d Collisions:%d",
                          table_stats.entries, table_stats.collisions);
        SDK_TRACE_INFO("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                          num_insert, num_update, num_get, num_remove, num_reserve, num_release);
        SDK_TRACE_INFO("Table API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                          api_stats.insert, api_stats.update, api_stats.get,
                          api_stats.remove, api_stats.reserve, api_stats.release);
        return;    
    }

    sdk_ret_t validate_stats() {
        print_stats();
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

    static void
    iterate_callback(sdk_table_api_params_t *params) {
        static char buff[512];
        (params->entry)->tostr(buff, 512);
        SDK_TRACE_INFO("Handle[%s] Entry[%s]",
                          params->handle.tostr(), buff);
        return;
    }

    sdk_ret_t iterate() {
        sdk_table_api_params_t params = { 0 };
        params.itercb = iterate_callback;
        return table->iterate(&params);
    }
};
#endif
