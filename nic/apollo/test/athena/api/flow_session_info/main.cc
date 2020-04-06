
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all flow session info test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/test/athena/api/flow_session_info/utils.hpp"
#include "nic/apollo/test/athena/api/include/scale.hpp"

namespace test {
namespace api {

uint32_t num_create, num_update, num_read, num_delete;
uint32_t num_entries;

//----------------------------------------------------------------------------
// SESSION INFO test class
//----------------------------------------------------------------------------

class flow_session_info_test : public pds_test_base {
protected:
    flow_session_info_test() {}
    virtual ~flow_session_info_test() {}
    void SetUp() {
        num_create = 0;
        num_update = 0;
        num_read = 0;
        num_delete = 0;
        num_entries = 0;
    }
    void TearDown() {
        display_gtest_stats();
    }
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
public:
    void display_gtest_stats() {
        SDK_TRACE_INFO("GTest Table Stats: Entries:%d", num_entries);
        SDK_TRACE_INFO("Test API Stats: Create=%d Update=%d Read=%d Delete:%d",
                       num_create, num_update, num_read, num_delete);
        return;
    }
};

pds_ret_t create_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_spec_t spec = { 0 };

    fill_key(&spec.key, index, ((index%2)+1));
    fill_scale_data(&spec.data, index);
    auto rs = pds_flow_session_info_create(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_create++;
        num_entries++;
    }
    return rs;
}

pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_key_t key = { 0 };

    fill_key(&key, index, ((index%2)+1));
    auto rs = pds_flow_session_info_delete(&key);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_delete++;
        num_entries--;
    }
    return rs;
}

pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_spec_t spec = { 0 };

    fill_key(&spec.key, index, ((index%2)+1));
    update_scale_data(&spec.data, index);
    auto rs = pds_flow_session_info_update(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_update++;
    }
    return rs;
}

pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_key_t key = { 0 };
    pds_flow_session_info_t info = { 0 };

    fill_key(&key, index, ((index%2)+1));
    auto rs = pds_flow_session_info_read(&key, &info);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_read++;
    }
    return rs;
}

//----------------------------------------------------------------------------
// Flow session test cases implementation
//----------------------------------------------------------------------------

/// \defgroup FLOW SESSION INFO Flow session info tests
/// @{

/// \brief Flow session info tests
TEST_F(flow_session_info_test, flow_session_info_crud) {
    pds_flow_session_spec_t spec = { 0 };
    pds_flow_session_key_t key = { 0 };
    pds_flow_session_info_t info = { 0 };

    fill_key(&spec.key, 2, HOST_TO_SWITCH);
    fill_data(&spec.data, 2, HOST_TO_SWITCH, CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == PDS_RET_OK);

    fill_key(&key, 2, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 5, SWITCH_TO_HOST);
    fill_data(&spec.data, 5, SWITCH_TO_HOST, CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 5, SWITCH_TO_HOST);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 11, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    fill_data(&spec.data, 11, (HOST_TO_SWITCH | SWITCH_TO_HOST), CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 11, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    fill_key(&key, 5, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_delete(&key) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 5, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) ==
               PDS_RET_ENTRY_NOT_FOUND);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, PDS_FLOW_SESSION_INFO_ID_MAX, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 0, SWITCH_TO_HOST);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, PDS_FLOW_SESSION_INFO_ID_MAX, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 45, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) ==
               PDS_RET_ENTRY_NOT_FOUND);

    memset(&info, 0, sizeof(info));
    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 40, SWITCH_TO_HOST);
    fill_data(&spec.data, 40, SWITCH_TO_HOST, CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == PDS_RET_OK);
    fill_key(&key, 40, SWITCH_TO_HOST);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);
    memset(&spec, 0, sizeof(spec));
    memcpy(&spec.data, &info.spec.data, sizeof(pds_flow_session_data_t)); 
    fill_key(&spec.key, 40, HOST_TO_SWITCH);
    fill_data(&spec.data, 40, HOST_TO_SWITCH, UPDATE);
    SDK_ASSERT(pds_flow_session_info_update(&spec) == PDS_RET_OK);
    memset(&info, 0, sizeof(info));
    fill_key(&key, 40, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    fill_data(&spec.data, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST), CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == PDS_RET_OK);
    memset(&info, 0, sizeof(info));
    fill_key(&key, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);
    memset(&spec, 0, sizeof(spec));
    memcpy(&spec.data, &info.spec.data, sizeof(pds_flow_session_data_t)); 
    fill_key(&spec.key, 25, SWITCH_TO_HOST);
    fill_data(&spec.data, 25, SWITCH_TO_HOST, UPDATE);
    SDK_ASSERT(pds_flow_session_info_update(&spec) == PDS_RET_OK);
    memset(&info, 0, sizeof(info));
    fill_key(&key, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);
}

/// \brief FLOW SESSION REWRITE scale tests
TEST_F(flow_session_info_test, create16)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, delete1K)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, cud_1K)
{
    pds_ret_t rs;
    rs = create_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, crurd_2K)
{
    pds_ret_t rs;
    rs = create_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, crurd_512K)
{
    pds_ret_t rs;
    rs = create_entries(512*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(512*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(512*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(512*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(512*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, crurd_1M)
{
    pds_ret_t rs;
    rs = create_entries(1*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(1*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(1*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(1*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, delete2M)
{
    pds_ret_t rs;
    rs = create_entries(2*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(2*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_info_test, delete4M)
{
    pds_ret_t rs;
    rs = create_entries((4*1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries((4*1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
