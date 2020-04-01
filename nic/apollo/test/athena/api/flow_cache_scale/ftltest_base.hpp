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
#include <atomic>

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"

#include "ftl_p4pd_mock.hpp"
#include "ftltest_utils.hpp"

#define GET_ATOMIC(v) (v.load(std::memory_order_relaxed))

using namespace std;

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

extern "C" {
// Function prototypes
sdk_ret_t pds_flow_cache_create(void);
void pds_flow_cache_delete(void);
void pds_flow_cache_set_core_id(uint32_t core_id);
}

class flow_cache_base: public ::testing::Test {
protected:
    atomic_uint num_insert;
    atomic_uint num_remove;
    atomic_uint num_update;
    atomic_uint num_reserve;
    atomic_uint num_release;
    atomic_uint num_get;
    
    atomic_uint table_count;

protected:
    flow_cache_base() {}
    virtual ~flow_cache_base() {}

    virtual void SetUp() {
        SDK_TRACE_INFO("============== SETUP : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
        
        ftl_mock_init();
        pds_flow_cache_create();
        pds_flow_cache_set_core_id(2);

        num_insert = 0;
        num_remove = 0;
        num_update = 0;
        num_reserve = 0;
        num_release = 0;
        num_get = 0;

        table_count = 0;
    }

    virtual void TearDown() {
        validate_stats();
        pds_flow_cache_delete();
        ftl_mock_cleanup();
        SDK_TRACE_INFO("============== TEARDOWN : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

private:
    void create_() {
        num_insert++;
    }

    void delete_() {
        num_remove++;
    }

    void update_() {
        num_update++;
    }

    void read_() {
        num_get++;
    }

public:

    sdk_ret_t create_helper(uint32_t index, sdk_ret_t expret) {
        pds_flow_spec_t spec = { 0 };

        fill_key(index, &spec.key);
        fill_data(index, PDS_FLOW_SPEC_INDEX_SESSION, &spec.data);
        auto rs = pds_flow_cache_entry_create(&spec);
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        if (rs == SDK_RET_OK) {
            create_();
            table_count++;
        }
        return rs;
    }

    sdk_ret_t create_entries(uint32_t count, sdk_ret_t expret) {
        for (auto i = 0; i < count; i++) {
            auto rs = create_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t delete_helper(uint32_t index, sdk_ret_t expret) {
        pds_flow_key_t key = { 0 };

        fill_key(index, &key);
        auto rs = pds_flow_cache_entry_delete(&key);
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        if (rs == SDK_RET_OK) {
            delete_();
            table_count--;
        }
        return rs;
    }

    sdk_ret_t delete_entries(uint32_t count, sdk_ret_t expret) {
        for (auto i = 0; i < count; i++) {
            auto rs = delete_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update_helper(uint32_t index, sdk_ret_t expret) {
        pds_flow_spec_t spec = { 0 };

        fill_key(index, &spec.key);
        fill_data(index + 10, PDS_FLOW_SPEC_INDEX_CONNTRACK, &spec.data);
        auto rs = pds_flow_cache_entry_update(&spec);
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        if (rs == SDK_RET_OK) {
            update_();
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update_entries(uint32_t count, sdk_ret_t expret) {
        for (auto i = 0; i < count; i++) {
            auto rs = update_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    sdk_ret_t read_helper(uint32_t index, sdk_ret_t expret) {
        pds_flow_key_t key = { 0 };
        pds_flow_info_t info = { 0 };

        fill_key(index, &key);
        auto rs = pds_flow_cache_entry_read(&key, &info);;
        MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        if (rs == SDK_RET_OK) {
            read_();
        }
        if ((info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_CONNTRACK) &&
            (info.spec.data.index == index + 10))
            return sdk::SDK_RET_OK;
        else if ((info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_SESSION) &&
                 (info.spec.data.index == index))
            return sdk::SDK_RET_OK;
        else
            return sdk::SDK_RET_ERR;
    }

    sdk_ret_t read_entries(uint32_t count, sdk_ret_t expret) {
        for (auto i = 0; i < count; i++) {
            auto rs = read_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, sdk::SDK_RET_MAX);
        }
        return SDK_RET_OK;
    }

    void display_gtest_stats() {
        SDK_TRACE_INFO("GTest Table Stats: Entries:%d", GET_ATOMIC(table_count));
        SDK_TRACE_INFO("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                       GET_ATOMIC(num_insert), GET_ATOMIC(num_update), GET_ATOMIC(num_get),
                       GET_ATOMIC(num_remove), GET_ATOMIC(num_reserve), GET_ATOMIC(num_release));
        return;
    }

    sdk_ret_t validate_stats() {
        pds_flow_stats_t stats = { 0 };

        SDK_ASSERT(pds_flow_cache_stats_get(2, &stats) == SDK_RET_OK);
        display_gtest_stats();
        dump_stats(&stats);

        EXPECT_TRUE(stats.api_insert >= stats.api_remove);
        EXPECT_EQ(table_count, stats.table_entries);
        EXPECT_EQ(num_insert, stats.api_insert + stats.api_insert_duplicate + stats.api_insert_fail);
        EXPECT_EQ(num_remove, stats.api_remove + stats.api_remove_not_found + stats.api_remove_fail);
        EXPECT_EQ(num_update, stats.api_update + stats.api_update_fail);
        // FIXME: For read we use iterate in ftl layer, so this wont work for now
        //EXPECT_EQ(num_get, (stats.api_get + stats.api_get_fail));
        EXPECT_EQ(num_reserve, stats.api_reserve + stats.api_reserve_fail);
        EXPECT_EQ(num_release, stats.api_release + stats.api_release_fail);
        return sdk::SDK_RET_OK;
    }
};
#endif
