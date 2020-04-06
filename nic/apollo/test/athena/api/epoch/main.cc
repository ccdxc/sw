
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all epoch test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_epoch.h"
#include "nic/apollo/test/athena/api/epoch/utils.hpp"
#include "nic/apollo/test/athena/api/include/scale.hpp"

namespace test {
namespace api {

uint32_t num_create, num_update, num_read, num_delete;
uint32_t num_entries;

//----------------------------------------------------------------------------
// EPOCH test class
//----------------------------------------------------------------------------

class epoch_test : public pds_test_base {
protected:
    epoch_test() {}
    virtual ~epoch_test() {}
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
    pds_epoch_spec_t spec = { 0 };

    fill_key(&spec.key, index);
    fill_data(&spec.data, index);
    auto rs = pds_epoch_create(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_create++;
        num_entries++;
    }
    return rs;
}

pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
    pds_epoch_key_t key = { 0 };

    fill_key(&key, index);
    auto rs = pds_epoch_delete(&key);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_delete++;
        num_entries--;
    }
    return rs;
}

pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
    pds_epoch_spec_t spec = { 0 };

    fill_key(&spec.key, index);
    update_data(&spec.data, index);
    auto rs = pds_epoch_update(&spec);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_update++;
    }
    return rs;
}

pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
    pds_epoch_key_t key = { 0 };
    pds_epoch_info_t info = { 0 };

    fill_key(&key, index);
    auto rs = pds_epoch_read(&key, &info);
    MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    if (rs == PDS_RET_OK) {
        num_read++;
    }
    if ((info.spec.data.epoch == (index % 65536)) ||
        (info.spec.data.epoch == ((index + UPDATE_DELTA) % 65536)))
        return PDS_RET_OK;
    else
        return PDS_RET_ERR;
}

//----------------------------------------------------------------------------
// Epoch test cases implementation
//----------------------------------------------------------------------------

/// \defgroup EPOCH Epoch Tests
/// @{

/// \brief Epoch tests
TEST_F(epoch_test, epoch_crud) {
    pds_epoch_spec_t spec = { 0 };
    pds_epoch_key_t key = { 0 };
    pds_epoch_info_t info = { 0 };

    spec.key.epoch_id = 1;
    spec.data.epoch = 100;
    SDK_ASSERT(pds_epoch_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 1;
    SDK_ASSERT(pds_epoch_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 100);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 2;
    spec.data.epoch = 200;
    SDK_ASSERT(pds_epoch_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 2;
    SDK_ASSERT(pds_epoch_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 200);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 1000;
    spec.data.epoch = 1010;
    SDK_ASSERT(pds_epoch_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 1000;
    SDK_ASSERT(pds_epoch_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 1010);

    key.epoch_id = 2;
    SDK_ASSERT(pds_epoch_delete(&key) == PDS_RET_OK);

    key.epoch_id = 2;
    SDK_ASSERT(pds_epoch_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 0);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 0;
    spec.data.epoch = 2000;
    SDK_ASSERT(pds_epoch_create(&spec) == PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = PDS_EPOCH_ID_MAX;
    spec.data.epoch = 55100;
    SDK_ASSERT(pds_epoch_create(&spec) == PDS_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 1000;
    spec.data.epoch = 11100;
    SDK_ASSERT(pds_epoch_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 1000;
    SDK_ASSERT(pds_epoch_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 11100);

}

/// \brief Epoch scale tests
TEST_F(epoch_test, create16)
{
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(epoch_test, delete1K)
{
    pds_ret_t rs;
    rs = create_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(epoch_test, cud_16K)
{
    pds_ret_t rs;
    rs = create_entries(16*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(16*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(16*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(epoch_test, crurd_256K)
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

TEST_F(epoch_test, crurd_512K)
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

TEST_F(epoch_test, delete1M)
{
    pds_ret_t rs;
    rs = create_entries((1*1024*1024)-1, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries((1*1024*1024)-1, PDS_RET_OK);
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
