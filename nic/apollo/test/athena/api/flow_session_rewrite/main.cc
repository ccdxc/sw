
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all flow session rewrite test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/test/athena/api/include/scale.hpp"
#include "nic/apollo/test/athena/api/flow_session_rewrite/utils.hpp"

namespace test {
namespace api {

uint32_t num_create, num_update, num_read, num_delete;
uint32_t num_entries;

//----------------------------------------------------------------------------
// SESSION REWRITE test class
//----------------------------------------------------------------------------

class flow_session_rewrite_test : public pds_test_base {
protected:
    flow_session_rewrite_test() {}
    virtual ~flow_session_rewrite_test() {}
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
    pds_flow_session_rewrite_spec_t spec = { 0 };

    fill_key(&spec.key, index);
    fill_data_type(&spec.data, index);
    auto rs = pds_flow_session_rewrite_create(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_create++;
        num_entries++;
    }
    return rs;
}

pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_rewrite_key_t key = { 0 };

    fill_key(&key, index);
    auto rs = pds_flow_session_rewrite_delete(&key);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_delete++;
        num_entries--;
    }
    return rs;
}

pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_rewrite_spec_t spec = { 0 };

    fill_key(&spec.key, index);
    update_data_type(&spec.data, index);
    auto rs = pds_flow_session_rewrite_update(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_update++;
    }
    return rs;
}

pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
    pds_flow_session_rewrite_key_t key = { 0 };
    pds_flow_session_rewrite_info_t info = { 0 };

    fill_key(&key, index);
    auto rs = pds_flow_session_rewrite_read(&key, &info);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_read++;
    }
    return rs;
}

//----------------------------------------------------------------------------
// Flow session rewrite test cases implementation
//----------------------------------------------------------------------------

/// \defgroup FLOW SESSION REWRITE Flow session rewrite tests
/// @{

/// \brief Flow session rewrite tests
TEST_F(flow_session_rewrite_test, flow_session_rewrite_crud) {
    pds_flow_session_rewrite_spec_t spec = { 0 };
    pds_flow_session_rewrite_key_t key = { 0 };
    pds_flow_session_rewrite_info_t info = { 0 };

    fill_key(&spec.key, 2);
    fill_data(&spec.data, 2, REWRITE_NAT_TYPE_IPV4_SNAT, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    fill_key(&key, 2);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 5);
    fill_data(&spec.data, 5, REWRITE_NAT_TYPE_IPV4_DNAT, ENCAP_TYPE_MPLSOUDP);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 5);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    fill_key(&key, 5);
    SDK_ASSERT(pds_flow_session_rewrite_delete(&key) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 5);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) ==
               PDS_RET_ERR);

    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 20);
    fill_data(&spec.data, 20, REWRITE_NAT_TYPE_IPV6_SNAT, ENCAP_TYPE_MPLSOUDP);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 20);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 32);
    fill_data(&spec.data, 32, REWRITE_NAT_TYPE_NONE, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 32);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, PDS_FLOW_SESSION_REWRITE_ID_MAX);
    fill_data(&spec.data, PDS_FLOW_SESSION_REWRITE_ID_MAX,
              REWRITE_NAT_TYPE_NONE, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 0);
    fill_data(&spec.data, 0, REWRITE_NAT_TYPE_NONE, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 11);
    fill_data(&spec.data, 11, REWRITE_NAT_TYPE_IPV4_SDPAT, ENCAP_TYPE_GENEVE);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 11);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 11);
    fill_data(&spec.data, 11, REWRITE_NAT_TYPE_IPV4_DNAT, ENCAP_TYPE_L2);
    SDK_ASSERT(pds_flow_session_rewrite_update(&spec) == 
            PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 11);
    SDK_ASSERT(pds_flow_session_rewrite_read(&key, &info) == 
            PDS_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_rewrite_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 24);
    fill_data(&spec.data, 24, REWRITE_NAT_TYPE_MAX, ENCAP_TYPE_GENEVE);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 34);
    fill_data(&spec.data, 34, REWRITE_NAT_TYPE_IPV6_SNAT, ENCAP_TYPE_MAX);
    SDK_ASSERT(pds_flow_session_rewrite_create(&spec) == 
            PDS_RET_INVALID_ARG);
}

/// \brief FLOW SESSION REWRITE scale tests
TEST_F(flow_session_rewrite_test, create16)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_rewrite_test, delete1K)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_rewrite_test, cud_1K)
{
    pds_ret_t rs;
    rs = create_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(flow_session_rewrite_test, crurd_2K)
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

TEST_F(flow_session_rewrite_test, crurd_512K)
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

TEST_F(flow_session_rewrite_test, crurd_1M)
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

TEST_F(flow_session_rewrite_test, delete2M)
{
    pds_ret_t rs;
    rs = create_entries((2*1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries((2*1024*1024)-1, PDS_RET_OK);
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
