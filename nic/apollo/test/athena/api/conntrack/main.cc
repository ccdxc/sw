
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all conntrack test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_conntrack.h"
#include "nic/apollo/test/athena/api/conntrack/utils.hpp"
#include "nic/apollo/test/athena/api/include/scale.hpp"

namespace test {
namespace api {

uint32_t num_create, num_update, num_read, num_delete;
uint32_t num_entries;

//----------------------------------------------------------------------------
// CONNTRACK test class
//----------------------------------------------------------------------------

class conntrack_test : public pds_test_base {
protected:
    conntrack_test() {}
    virtual ~conntrack_test() {}
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
    pds_conntrack_spec_t spec = { 0 };

    fill_key(&spec.key, index);
    fill_data(&spec.data, index);
    auto rs = pds_conntrack_state_create(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_create++;
        num_entries++;
    }
    return rs;
}

pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
    pds_conntrack_key_t key = { 0 };

    fill_key(&key, index);
    auto rs = pds_conntrack_state_delete(&key);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_delete++;
        num_entries--;
    }
    return rs;
}

pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
    pds_conntrack_spec_t spec = { 0 };

    fill_key(&spec.key, index);
    update_data(&spec.data, index);
    auto rs = pds_conntrack_state_update(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_update++;
    }
    return rs;
}

pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
    pds_conntrack_key_t key = { 0 };
    pds_conntrack_info_t info = { 0 };

    fill_key(&key, index);
    auto rs = pds_conntrack_state_read(&key, &info);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_read++;
    }
    if ((info.spec.data.flow_type == PDS_FLOW_TYPE_TCP) &&
        (info.spec.data.flow_state == ((index % 11) + 1)))
        return PDS_RET_OK;
    else if ((info.spec.data.flow_type == PDS_FLOW_TYPE_OTHERS) &&
             (info.spec.data.flow_state == UNESTABLISHED))
        return PDS_RET_OK;
    else
        return PDS_RET_ERR;
}

//----------------------------------------------------------------------------
// Conntrack test cases implementation
//----------------------------------------------------------------------------

/// \defgroup CONNTRACK Conntrack Tests
/// @{

/// \brief Conntrack tests
TEST_F(conntrack_test, conntrack_crud) {
    pds_conntrack_spec_t spec = { 0 };
    pds_conntrack_key_t key = { 0 };
    pds_conntrack_info_t info = { 0 };

    fill_key(&spec.key, 1);
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_ICMP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == PDS_RET_OK);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 2);
    spec.data.flow_state = PDS_FLOW_STATE_SYN_SENT;
    spec.data.flow_type = PDS_FLOW_TYPE_TCP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == PDS_RET_OK);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 5);
    spec.data.flow_state = ESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_UDP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == PDS_RET_OK);

    fill_key(&key, 2);
    SDK_ASSERT(pds_conntrack_state_delete(&key) == PDS_RET_OK);

    memset(&key, 0, sizeof(key));
    fill_key(&key, 2);
    SDK_ASSERT(pds_conntrack_state_read(&key, &info) == PDS_RET_ENTRY_NOT_FOUND);
    SDK_ASSERT(info.spec.data.flow_state == 0);
    SDK_ASSERT(info.spec.data.flow_type == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 0);
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_UDP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, PDS_CONNTRACK_ID_MAX);
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_UDP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == PDS_RET_INVALID_ARG);

    memset(&key, 0, sizeof(key));
    fill_key(&key, 5);
    SDK_ASSERT(pds_conntrack_state_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.flow_state == ESTABLISHED);
    SDK_ASSERT(info.spec.data.flow_type == PDS_FLOW_TYPE_UDP);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 5);
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_OTHERS;
    SDK_ASSERT(pds_conntrack_state_update(&spec) == PDS_RET_OK);

    memset(&key, 0 , sizeof(key));
    fill_key(&key, 5);
    SDK_ASSERT(pds_conntrack_state_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.flow_state == UNESTABLISHED);
    SDK_ASSERT(info.spec.data.flow_type == PDS_FLOW_TYPE_OTHERS);
}

/// \brief Conntrack scale tests
TEST_F(conntrack_test, create16)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(conntrack_test, delete1K)
{
    pds_ret_t rs;
    rs = create_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(conntrack_test, cud_16K)
{
    pds_ret_t rs;
    rs = create_entries(16*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(16*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(16*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(conntrack_test, crurd_256K)
{
    pds_ret_t rs;
    rs = create_entries(256*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(256*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(256*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(256*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(256*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(conntrack_test, create1M)
{
    pds_ret_t rs;
    rs = create_entries(1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(conntrack_test, delete2M)
{
    pds_ret_t rs;
    rs = create_entries(2*1024*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(2*1024*1024, PDS_RET_OK);
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
