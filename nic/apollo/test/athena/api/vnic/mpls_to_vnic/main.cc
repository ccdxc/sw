
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mpls label to vnic test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/test/athena/api/include/scale.hpp"
#include "nic/apollo/test/athena/api/vnic/mpls_to_vnic/utils.hpp"

namespace test {
namespace api {

uint32_t num_create, num_update, num_read, num_delete;
uint32_t num_entries;

//----------------------------------------------------------------------------
// MPLS LABEL TO VNIC test class
//----------------------------------------------------------------------------

class mpls_label_to_vnic_test : public pds_test_base {
protected:
    mpls_label_to_vnic_test() {}
    virtual ~mpls_label_to_vnic_test() {}
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
    pds_mpls_label_to_vnic_map_spec_t spec = { 0 };

    fill_mpls_label_to_vnic_key(&spec.key, index);
    fill_mpls_label_to_vnic_data(&spec.data, index);
    auto rs = pds_mpls_label_to_vnic_map_create(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_create++;
        num_entries++;
    }
    return rs;
}

pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
    pds_mpls_label_to_vnic_map_key_t key = { 0 };

    fill_mpls_label_to_vnic_key(&key, index);
    auto rs = pds_mpls_label_to_vnic_map_delete(&key);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_delete++;
        num_entries--;
    }
    return rs;
}

pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
    pds_mpls_label_to_vnic_map_spec_t spec = { 0 };

    fill_mpls_label_to_vnic_key(&spec.key, index);
    update_mpls_label_to_vnic_data(&spec.data, index);
    auto rs = pds_mpls_label_to_vnic_map_update(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_update++;
    }
    return rs;
}

pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
    pds_mpls_label_to_vnic_map_key_t key = { 0 };
    pds_mpls_label_to_vnic_map_info_t info = { 0 };

    fill_mpls_label_to_vnic_key(&key, index);
    auto rs = pds_mpls_label_to_vnic_map_read(&key, &info);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_read++;
    }
    if ((info.spec.data.vnic_id == (index % PDS_VNIC_ID_MAX)) &&
        (info.spec.data.vnic_type == VNIC_TYPE_L3))
        return PDS_RET_OK;
    else if ((info.spec.data.vnic_id == ((index + UPDATE_DELTA) % PDS_VNIC_ID_MAX)) &&
        (info.spec.data.vnic_type == VNIC_TYPE_L2))
        return PDS_RET_OK;
    else
        return PDS_RET_ERR;
}

//----------------------------------------------------------------------------
// MPLS LABEL TO VNIC test cases implementation
//----------------------------------------------------------------------------

/// \brief MPLS LABEL TO VNIC tests
TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_map_crud) {
    pds_mpls_label_to_vnic_map_spec_t spec = { 0 };
    pds_mpls_label_to_vnic_map_key_t key = { 0 };
    pds_mpls_label_to_vnic_map_info_t info = { 0 };

    spec.key.mpls_label = 1;
    spec.data.vnic_id = 1;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == PDS_RET_OK);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = 2;
    spec.data.vnic_id = 2;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == PDS_RET_OK);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = 5;
    spec.data.vnic_id = 20;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == PDS_RET_OK);

    key.mpls_label = 2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_delete(&key) == PDS_RET_OK);

    memset(&key, 0 , sizeof(key));
    key.mpls_label = 2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 0);
    SDK_ASSERT(info.spec.data.vnic_type == 0);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = PDS_MPLS_LABEL_MAX;
    spec.data.vnic_id = 10;
    spec.data.vnic_type = VNIC_TYPE_L3;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == PDS_RET_INVALID_ARG);

    memset(&key, 0 , sizeof(key));
    key.mpls_label = 5;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 20);
    SDK_ASSERT(info.spec.data.vnic_type == VNIC_TYPE_L2);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = 5;
    spec.data.vnic_id = 30;
    spec.data.vnic_type = VNIC_TYPE_L3;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_update(&spec) == PDS_RET_OK);
}

/// \brief MPLS LABEL TO VNIC scale tests
TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_create16)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_delete1K)
{
    pds_ret_t rs;
    rs = create_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_cud_1K)
{
    pds_ret_t rs;
    rs = create_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_crurd_256K)
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

TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_crurd_512K)
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

TEST_F(mpls_label_to_vnic_test, mpls_label_to_vnic_crurd_1M)
{
    pds_ret_t rs;
    rs = create_entries((1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries((1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries((1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries((1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries((1024*1024)-1, PDS_RET_OK);
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
