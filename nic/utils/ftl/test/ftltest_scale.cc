//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "gen/p4gen/p4/include/ftl.h"
#include "ftltest_base.hpp"
#include "lib/utils/time_profile.hpp"
#include "ftltest_main.hpp"

class scale: public FtlGtestBase {
    sdk_trace_level_e trace_level;

protected:
    sdk::utils::time_profile::time_profile_info t_info;

    scale() {
        trace_level = g_trace_level;
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
        sdk::utils::time_profile::time_profile_enable = true;
    }

    ~scale() {
        g_trace_level = trace_level;
        sdk::utils::time_profile::time_profile_enable = false;
    }

    void SetUp() {
        FtlGtestBase::SetUp();
        t_info.start();
    }

    void TearDown() {
        t_info.stop();
        SDK_TRACE_INFO("Time to %s.%s : %s",
                       ::testing::UnitTest::GetInstance()->
                       current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->
                       current_test_info()->name(),
                       t_info.print_diff().c_str());
        FtlGtestBase::TearDown();
    }
};

TEST_F(scale, num1K) {
    sdk_ret_t rs;
    rs = Insert(1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    Iterate();

    rs = Update(1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num4K) {
    sdk_ret_t rs;
    rs = Insert(4*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(4*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(4*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num16K) {
    sdk_ret_t rs;
    rs = Insert(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Update(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(16*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, num256K) {
    sdk_ret_t rs;
    rs = Insert(256*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = Remove(256*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, insert1M_with_hash) {
    sdk_ret_t rs;
    rs = Insert(1024*1024, sdk::SDK_RET_OK, WITH_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, remove1M_with_hash) {
    sdk_ret_t rs;
    rs = Insert(1024*1024, sdk::SDK_RET_OK, WITH_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    t_info.start();
    rs = Remove(1024*1024, sdk::SDK_RET_OK, WITH_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_insert8M) {
    sdk_ret_t rs;
    rs = Insert(8*1024*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(scale, DISABLED_insert16M) {
    sdk_ret_t rs;
    rs = Insert(16*1024*1024, sdk::SDK_RET_OK, WITHOUT_HASH);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}
