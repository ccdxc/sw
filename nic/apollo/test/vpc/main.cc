//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vpc test cases
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
constexpr int k_max_vpc = PDS_MAX_VPC;

//----------------------------------------------------------------------------
// VPC test class
//----------------------------------------------------------------------------

class vpc : public ::pds_test_base {
protected:
    vpc() {}
    virtual ~vpc() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;
        params.cfg_file = g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// VPC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VPC_TEST
/// @{

/// \brief Create and delete VPCs in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(vpc, vpc_workflow_1) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {};
    std::string vpc_start_addr = "10.0.0.0/16";

    key.id = 1;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key, k_max_vpc) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max VPCs in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(vpc, vpc_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {};
    std::string vpc_start_addr = "10.0.0.0/16";

    // trigger
    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key, k_max_vpc) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key, k_max_vpc) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 vpc should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(vpc, vpc_workflow_3) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {}, key2 = {}, key3 = {};
    std::string vpc_start_addr1 = "10.0.0.0/16";
    std::string vpc_start_addr2 = "30.0.0.0/16";
    std::string vpc_start_addr3 = "60.0.0.0/16";
    uint32_t num_vpcs = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key1, vpc_start_addr1, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key2, vpc_start_addr2, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key1, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key3, vpc_start_addr3, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key1, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key2, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key3, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key2, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key3, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key2, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key3, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete VPC in two batches
/// The hardware should create and delete VPC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(vpc, vpc_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {};
    std::string vpc_start_addr = "10.0.0.0/16";

    key.id = 1;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key, k_max_vpc) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of VPC in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(vpc, vpc_workflow_5) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {}, key2 = {}, key3 = {};
    std::string vpc_start_addr1 = "10.0.0.0/16";
    std::string vpc_start_addr2 = "40.0.0.0/16";
    std::string vpc_start_addr3 = "70.0.0.0/16";
    uint32_t num_vpcs = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key1, vpc_start_addr1, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key2, vpc_start_addr2, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key1, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key2, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key1, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key3, vpc_start_addr3, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key1, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key2, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key3, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key2, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key3, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key2, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key3, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of VPCs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(vpc, vpc_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {};
    std::string vpc_start_addr = "10.0.0.0/16";

    key.id = 1;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key, k_max_vpc) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of VPCs supported.
/// [ Create SetMax+1] - Read
TEST_F(vpc, vpc_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {};
    std::string vpc_start_addr = "10.0.0.0/16";

    key.id = 1;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key, vpc_start_addr, k_max_vpc + 1,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc + 1, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing VPC should return entry not found.
/// Read NonEx
TEST_F(vpc, vpc_workflow_neg_3a) {
    pds_vpc_key_t key = {};

    key.id = 1;

    // trigger
    ASSERT_TRUE(vpc_util::many_read(
        key, k_max_vpc, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Deletion of a non-existing VPCs should fail.
/// [Delete NonEx]
TEST_F(vpc, vpc_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {};

    key.id = 1;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key, k_max_vpc) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(vpc, vpc_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {}, key2 = {};
    std::string vpc_start_addr1 = "10.0.0.0/16";
    std::string vpc_start_addr2 = "40.0.0.0/16";
    uint32_t num_vpcs = 20;

    key1.id = 10;
    key2.id = 40;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_create(key1, vpc_start_addr1, num_vpcs,
                                      PDS_VPC_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key1, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key1, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key2, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key1, num_vpcs, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_read(
        key2, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vpc_util::many_delete(key1, num_vpcs) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vpc_util::many_read(
        key1, num_vpcs, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create a VPC with an id which is not within the range.
TEST_F(vpc, vpc_workflow_corner_case) {}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
vpc_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
}

static void
vpc_test_options_parse (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        default:    // ignore all other options
            break;
        }
    }
}

static inline sdk_ret_t
vpc_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

int
main (int argc, char **argv)
{
    vpc_test_options_parse(argc, argv);
    if (vpc_test_options_validate() != SDK_RET_OK) {
        vpc_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
