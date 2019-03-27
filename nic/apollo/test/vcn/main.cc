//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vcn test cases
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vcn.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
constexpr int k_max_vcn = PDS_MAX_VCN;

//----------------------------------------------------------------------------
// VCN test class
//----------------------------------------------------------------------------

class vcn : public ::pds_test_base {
protected:
    vcn() {}
    virtual ~vcn() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// VCN test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VCN_TEST
/// @{

/// \brief Create and delete VCNs in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(vcn, vcn_workflow_1)
{
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    std::string vcn_start_addr = "10.0.0.0/16";

    key.id = 1;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, k_max_vcn) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max VCNs in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(vcn, vcn_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    std::string vcn_start_addr = "10.0.0.0/16";

    // Trigger
    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, k_max_vcn) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, k_max_vcn) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 vcn should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(vcn, vcn_workflow_3) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key1 = {}, key2 = {}, key3 = {};
    std::string vcn_start_addr1 = "10.0.0.0/16";
    std::string vcn_start_addr2 = "30.0.0.0/16";
    std::string vcn_start_addr3 = "60.0.0.0/16";
    uint32_t num_vcns = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key1, vcn_start_addr1, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key2, vcn_start_addr2, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key3, vcn_start_addr3, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key1, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key2, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key3, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key2, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key3, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key2, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key3, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete VCN in two batches
/// The hardware should create and delete VCN correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(vcn, vcn_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    std::string vcn_start_addr = "10.0.0.0/16";

    key.id = 1;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, k_max_vcn) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of VCN in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(vcn, vcn_workflow_5) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key1 = {}, key2 = {}, key3 = {};
    std::string vcn_start_addr1 = "10.0.0.0/16";
    std::string vcn_start_addr2 = "40.0.0.0/16";
    std::string vcn_start_addr3 = "70.0.0.0/16";
    uint32_t num_vcns = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key1, vcn_start_addr1, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key2, vcn_start_addr2, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key1, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key2, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key3, vcn_start_addr3, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key1, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key2, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key3, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key2, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key3, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key2, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key3, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of VCNs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(vcn, vcn_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    std::string vcn_start_addr = "10.0.0.0/16";

    key.id = 1;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, k_max_vcn) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of VCNs supported.
/// [ Create SetMax+1] - Read
TEST_F(vcn, vcn_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    std::string vcn_start_addr = "10.0.0.0/16";

    key.id = 1;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, k_max_vcn + 1,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn + 1, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing VCN should return entry not found.
/// Read NonEx
TEST_F(vcn, vcn_workflow_neg_3a) {
    pds_vcn_key_t key = {};

    key.id = 1;

    // Trigger
    ASSERT_TRUE(vcn_util::many_read(
        key, k_max_vcn, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Deletion of a non-existing VCNs should fail.
/// [Delete NonEx]
TEST_F(vcn, vcn_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};

    key.id = 1;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, k_max_vcn) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(vcn, vcn_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key1 = {}, key2 = {};
    std::string vcn_start_addr1 = "10.0.0.0/16";
    std::string vcn_start_addr2 = "40.0.0.0/16";
    uint32_t num_vcns = 20;

    key1.id = 10;
    key2.id = 40;

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key1, vcn_start_addr1, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key1, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key2, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key1, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(
        key2, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(
        key1, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create a VCN with an id which is not within the range.
TEST_F(vcn, vcn_workflow_corner_case) {}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
vcn_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
}

static void
vcn_test_options_parse (int argc, char **argv)
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
vcn_test_options_validate (void)
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
    vcn_test_options_parse(argc, argv);
    if (vcn_test_options_validate() != SDK_RET_OK) {
        vcn_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
