//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#ifndef __FTL_SCALE_HPP__
#define __FTL_SCALE_HPP__

#include <gtest/gtest.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <cinttypes>
#include <atomic>
#include "nic/sdk/include/sdk/base.hpp"

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

#define GET_ATOMIC(v) (v.load(std::memory_order_relaxed))

using namespace std;

class ftl_scale_base: public ::testing::Test {
protected:
    atomic_uint num_insert;
    atomic_uint num_remove;
    atomic_uint num_update;
    atomic_uint num_reserve;
    atomic_uint num_release;
    atomic_uint num_get;
    
    atomic_uint table_count;

protected:
    ftl_scale_base() {}
    virtual ~ftl_scale_base() {}

    virtual void SetUp() {
        SDK_TRACE_INFO("============== SETUP : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
        
        num_insert = 0;
        num_remove = 0;
        num_update = 0;
        num_reserve = 0;
        num_release = 0;
        num_get = 0;

        table_count = 0;
    }

    virtual void TearDown() {
        SDK_TRACE_INFO("============== TEARDOWN : %s.%s ===============",
                          ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                          ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }

    virtual pds_ret_t create_helper(uint32_t index, pds_ret_t expret) {
        return PDS_RET_OK;
    }
    virtual pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
        return PDS_RET_OK;
    }
    virtual pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
        return PDS_RET_OK;
    }
    virtual pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
        return PDS_RET_OK;
    }

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
    void display_gtest_stats() {
        SDK_TRACE_INFO("GTest Table Stats: Entries:%d", GET_ATOMIC(table_count));
        SDK_TRACE_INFO("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                       GET_ATOMIC(num_insert), GET_ATOMIC(num_update), GET_ATOMIC(num_get),
                       GET_ATOMIC(num_remove), GET_ATOMIC(num_reserve), GET_ATOMIC(num_release));
        return;
    }

    pds_ret_t create_entries(uint32_t count, pds_ret_t expret) {
        for (auto i = 1; i <= count; i++) {
            auto rs = create_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        }
        return PDS_RET_OK;
    }

    pds_ret_t delete_entries(uint32_t count, pds_ret_t expret) {
        for (auto i = 1; i <= count; i++) {
            auto rs = delete_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        }
        return PDS_RET_OK;
    }

    pds_ret_t update_entries(uint32_t count, pds_ret_t expret) {
        for (auto i = 1; i <= count; i++) {
            auto rs = update_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        } 
        return PDS_RET_OK;
    }

    pds_ret_t read_entries(uint32_t count, pds_ret_t expret) {
        for (auto i = 1; i <= count; i++) {
            auto rs = read_helper(i, expret);
            MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        }
        return PDS_RET_OK;
    }
};
#endif // __FTL_SCALE_HPP__
