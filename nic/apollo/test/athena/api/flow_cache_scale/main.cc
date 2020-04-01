//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/sdk/lib/utils/time_profile.hpp"
#include "ftltest_base.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// FLOW CACHE SCALE test class
//----------------------------------------------------------------------------

class flow_cache_scale: public flow_cache_base {
    sdk_trace_level_e trace_level;

protected:
    sdk::utils::time_profile::time_profile_info t_info;

    flow_cache_scale() {
        trace_level = g_trace_level;
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
        sdk::utils::time_profile::time_profile_enable = true;
    }

    ~flow_cache_scale() {
        g_trace_level = trace_level;
        sdk::utils::time_profile::time_profile_enable = false;
    }

    void SetUp() {
        flow_cache_base::SetUp();
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
        flow_cache_base::TearDown();
    }
};

//----------------------------------------------------------------------------
// Flow cache scale test cases implementation
//----------------------------------------------------------------------------

/// \defgroup FLOW CACHE SCALE Flow cache scale tests
/// @{

/// \brief Flow cache scale tests
TEST_F(flow_cache_scale, cr_16) {
    sdk_ret_t rs;
    rs = create_entries(16, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = read_entries(16, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, cud_16) {
    sdk_ret_t rs;
    rs = create_entries(16, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);

    rs = update_entries(16, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = delete_entries(16, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, cud_1K) {
    sdk_ret_t rs;
    rs = create_entries(1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = update_entries(1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = delete_entries(1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, cud_4K) {
    sdk_ret_t rs;
    rs = create_entries(4*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = update_entries(4*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = delete_entries(4*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, cud_16K) {
    sdk_ret_t rs;
    rs = create_entries(16*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = update_entries(16*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = delete_entries(16*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, cud_256K) {
    sdk_ret_t rs;
    rs = create_entries(256*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = delete_entries(256*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, create1M) {
    sdk_ret_t rs;
    rs = create_entries(1024*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, DISABLED_delete1M) {
    sdk_ret_t rs;
    rs = create_entries(1024*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
    rs = delete_entries(1024*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

TEST_F(flow_cache_scale, DISABLED_create4M) {
    sdk_ret_t rs;
    rs = create_entries(4*1024*1024, sdk::SDK_RET_OK);
    ASSERT_TRUE(rs == sdk::SDK_RET_OK);
}

/// @}

}    // namespace api
}    // namespace test

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
