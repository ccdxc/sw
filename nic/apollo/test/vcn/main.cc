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
int g_batch_epoch = 1;    // running batch epoch

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

#if 0
/// \brief Create a VCN
/// Create vcn 1 with 10.0.0.0/8
TEST_F(vcn, vcn_create) {
    pds_batch_params_t batch_params = {0};

    vcn_util *vcn1 = NULL;
    vcn1 = new vcn_util(PDS_VCN_TYPE_TENANT, 1, "10.0.0.0/8");
    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn1->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Update a VCN
/// Update an existing vcn
TEST_F(vcn, vcn_update) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create a VCN
/// Recreate an existing vcn
TEST_F(vcn, vcn_recreate) {}

/// \brief Read a VCN
/// Read an existing vcn
TEST_F(vcn, vcn_read_existing_vcn) {
    pds_vcn_info_t info = {0};
    pds_vcn_key_t key = {0};
    key.id = 1;

    pds_vcn_read(&key, &info);
    printf("READ VCN key : %d status : %d\n", key.id, info.status.hw_id);
}

/// \brief Delete a VCN
/// Delete existing vcn
TEST_F(vcn, vcn_delete_existing_vcn) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn_util *vcn1 = new vcn_util(PDS_VCN_TYPE_TENANT, 1, "10.0.0.0/8");
    vcn1->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Read a VCN
/// Read a non-existing vcn
TEST_F(vcn, vcn_read_non_existing_vcn) {
    pds_vcn_info_t info = {0};
    pds_vcn_key_t key = {0};
    key.id = 1;

    pds_vcn_read(&key, &info);
    printf("READ VCN key : %d status : %d\n", key.id, info.status.hw_id);

    key.id = 2;
    pds_vcn_read(&key, &info);
    printf("READ VCN key : %d status : %d\n", key.id, info.status.hw_id);
}

/// \brief Create and delete VCN in the same batch
/// Create and delete VCN in the same batch
TEST_F(vcn, vcn_create_delete_in_same_batch) {}

/// \brief Delete a non-existing VCN
/// Delete non-existing vcn
TEST_F(vcn, vcn_delete_non_existing_vcn) {}

/// \brief Create a VCN
/// Create a vcn again after delete
TEST_F(vcn, vcn_create_after_delete) {}

/// \brief Create multiple VCNs in the same batch
/// Create multiple vcns in the same batch
TEST_F(vcn, vcn_create_multiple_vcns_single_batch) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn_util *vcn1, *vcn2, *vcn3, *vcn4, *vcn5, *vcn6;

    vcn1 = new vcn_util(PDS_VCN_TYPE_TENANT, 2, "10.0.0.0/8");
    vcn2 = new vcn_util(PDS_VCN_TYPE_TENANT, 3, "11.0.0.0/8");
    vcn3 = new vcn_util(PDS_VCN_TYPE_TENANT, 4, "12.0.0.0/8");
    vcn4 = new vcn_util(PDS_VCN_TYPE_TENANT, 5, "13.0.0.0/8");
    vcn5 = new vcn_util(PDS_VCN_TYPE_TENANT, 6, "14.0.0.0/8");

    vcn1->create();
    vcn2->create();
    vcn3->create();
    vcn4->create();
    vcn5->create();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Read multiple VCNs created in the same batch
/// Read multiple vcns created in the same batch
TEST_F(vcn, vcn_read_multiple_vcns_single_batch) {
    pds_vcn_info_t info = {0};
    pds_vcn_key_t key = {0};
    key.id = 6;

    pds_vcn_read(&key, &info);
    printf("READ VCN key : %d status : %d\n", key.id, info.status.hw_id);

    key.id = 4;
    pds_vcn_read(&key, &info);
    printf("READ VCN key : %d status : %d\n", key.id, info.status.hw_id);

    key.id = 5;
    pds_vcn_read(&key, &info);
    printf("READ VCN key : %d status : %d\n", key.id, info.status.hw_id);
}

/// \brief Delete multiple VCNs in the same batch
/// Delete multiple vcns in the same batch
TEST_F(vcn, vcn_delete_multiple_vcns_single_batch) {
    pds_batch_params_t batch_params = {0};

    vcn_util *vcn1, *vcn2, *vcn3, *vcn4, *vcn5;
    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn1 = new vcn_util(PDS_VCN_TYPE_TENANT, 6, "");
    vcn2 = new vcn_util(PDS_VCN_TYPE_TENANT, 2, "");
    vcn3 = new vcn_util(PDS_VCN_TYPE_TENANT, 3, "");
    vcn4 = new vcn_util(PDS_VCN_TYPE_TENANT, 4, "");
    vcn5 = new vcn_util(PDS_VCN_TYPE_TENANT, 5, "");

    vcn1->del();
    vcn2->del();
    vcn3->del();
    vcn4->del();
    vcn5->del();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create multiple VCNs in multiple batches
/// Create multiple vcns in multiple batches
TEST_F(vcn, vcn_create_multiple_vcns_multiple_batches) {
    pds_batch_params_t batch_params = {0};

    vcn_util *vcn7, *vcn8, *vcn9, *vcn10, *vcn11;
    vcn7 = new vcn_util(PDS_VCN_TYPE_TENANT, 7, "10.0.0.0/8");
    vcn8 = new vcn_util(PDS_VCN_TYPE_TENANT, 8, "11.0.0.0/8");
    vcn9 = new vcn_util(PDS_VCN_TYPE_TENANT, 9, "12.0.0.0/8");
    vcn10 = new vcn_util(PDS_VCN_TYPE_TENANT, 10, "13.0.0.0/8");
    vcn11 = new vcn_util(PDS_VCN_TYPE_TENANT, 11, "14.0.0.0/8");

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn7->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn8->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn9->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn10->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn11->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Delete multiple VCNs in multiple batches
/// Delete multiple vcns in multiple batches
TEST_F(vcn, vcn_delete_multiple_vcns_multiple_batches) {
    pds_batch_params_t batch_params = {0};

    vcn_util *vcn7, *vcn8, *vcn9, *vcn10, *vcn11;
    vcn7 = new vcn_util(PDS_VCN_TYPE_TENANT, 7, "");
    vcn8 = new vcn_util(PDS_VCN_TYPE_TENANT, 8, "");
    vcn9 = new vcn_util(PDS_VCN_TYPE_TENANT, 9, "");
    vcn10 = new vcn_util(PDS_VCN_TYPE_TENANT, 10, "");
    vcn11 = new vcn_util(PDS_VCN_TYPE_TENANT, 11, "");

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn7->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn8->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn9->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn10->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn11->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Read multiple VCNs created in the multiple batches
/// Read multiple vcns in multiple batches
TEST_F(vcn, vcn_read_multiple_vcns_multiple_batches) {}

/// \brief Combination of create/delete/read VCNs in the same batch
/// Combination of create/delete VCNs in the same batch
TEST_F(vcn, vcn_combine_create_delete_single_batch) {
    pds_batch_params_t batch_params = {0};

    vcn_util *vcn12, *vcn13, *vcn14, *vcn15, *vcn16;
    vcn12 = new vcn_util(PDS_VCN_TYPE_TENANT, 12, "12.0.0.0/8");
    vcn13 = new vcn_util(PDS_VCN_TYPE_TENANT, 13, "13.0.0.0/8");
    vcn14 = new vcn_util(PDS_VCN_TYPE_TENANT, 14, "14.0.0.0/8");
    vcn15 = new vcn_util(PDS_VCN_TYPE_TENANT, 15, "15.0.0.0/8");
    vcn16 = new vcn_util(PDS_VCN_TYPE_TENANT, 16, "16.0.0.0/8");

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn12->create();
    vcn13->create();
    vcn12->del();
    vcn14->create();
    vcn13->del();
    vcn15->create();
    vcn16->create();
    vcn14->del();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Combination of create/delete/read VCNs in multiple batches
/// Combination of create/delete VCNs in multiple batches
TEST_F(vcn, vcn_combine_create_delete_multiple_batches) {
    pds_batch_params_t batch_params = {0};

    vcn_util *vcn22, *vcn23, *vcn24, *vcn25, *vcn26;
    vcn22 = new vcn_util(PDS_VCN_TYPE_TENANT, 22, "22.0.0.0/8");
    vcn23 = new vcn_util(PDS_VCN_TYPE_TENANT, 23, "23.0.0.0/8");
    vcn24 = new vcn_util(PDS_VCN_TYPE_TENANT, 24, "24.0.0.0/8");
    vcn25 = new vcn_util(PDS_VCN_TYPE_TENANT, 25, "25.0.0.0/8");
    vcn26 = new vcn_util(PDS_VCN_TYPE_TENANT, 26, "26.0.0.0/8");

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn22->create();
    vcn23->create();
    vcn24->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn25->create();
    vcn22->del();
    vcn23->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn26->create();
    vcn24->del();
    vcn25->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn26->del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create max number of VCNs
/// Create 1k VCNs
TEST_F(vcn, vcn_create_max) {}

/// \brief Create more than max
/// Create more than 1k VCNs
TEST_F(vcn, vcn_create_more_than_max) {}

/// \brief Create vcn with invalid data
/// Try to create a vcn with invalid data
TEST_F(vcn, vcn_create_invalid_vcn) {
    // There is a core seen for this test case in pre_process_create_
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn_util *vcn5;
    vcn5 = new vcn_util(PDS_VCN_TYPE_TENANT, 5, "0.0.0.0/0");
    vcn5->create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}
#endif

/// \brief Create and delete VCNs in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
TEST_F(vcn, DISABLED_vcn_workflow_1)
{
    // [ Create SetMax, Delete SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr = "10.0.0.0/16";
    uint32_t num_vcns = 1024;

    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key, num_vcns, sdk::SDK_RET_OK)
                == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max VCNs in the same batch
/// create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
TEST_F(vcn, DISABLED_vcn_workflow_2) {
    // [ Create SetMax - Delete SetMax - Create SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr = "10.0.0.0/16";
    uint32_t num_vcns = 1024;

    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key, num_vcns, sdk::SDK_RET_OK)
                == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create two sets of vcns viz set1 and set2. Delete set1.
/// Create a new set - set3. Try to read a vcn from each set.
TEST_F(vcn, DISABLED_vcn_workflow_3) {
    // [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key1 = {}, key2 = {}, key3 = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr1 = "10.0.0.0/16";
    std::string vcn_start_addr2 = "30.0.0.0/16";
    std::string vcn_start_addr3 = "60.0.0.0/16";
    uint32_t num_vcns = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;
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

    // Bug in reading
    //ASSERT_TRUE(vcn_util::many_read(key, num_vcns) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key2, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key3, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete VCN in two batches
/// The hardware should create VCN correctly
/// and return entry not found after delete
TEST_F(vcn, DISABLED_vcn_workflow_4) {
    // [ Create SetMax ] - Read - [ Delete SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr = "10.0.0.0/16";
    uint32_t num_vcns = 1024;

    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key, num_vcns,
                                    sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
}

/// \brief Create and delete VCN in two batches
/// The hardware should create VCN correctly
/// and return entry not found after delete
TEST_F(vcn, DISABLED_vcn_workflow_5) {
    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key1 = {}, key2 = {}, key3 = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr1 = "10.0.0.0/16";
    std::string vcn_start_addr2 = "40.0.0.0/16";
    std::string vcn_start_addr3 = "70.0.0.0/16";
    uint32_t num_vcns = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key1, vcn_start_addr1, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key2, vcn_start_addr2, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key1, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key2, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key3, vcn_start_addr3, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key1, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key2, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key3, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key2, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key3, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}


/// \brief Create maximum number of VCNs in two batches
/// The hardware should program VCNs correctly in case of
/// first create and return error in second create operation
TEST_F(vcn, DISABLED_vcn_workflow_neg_1) {
    // [ Create SetMax ] - [ Create SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr = "10.0.0.0/16";
    uint32_t num_vcns = 1024;
    vcn_util vcn_obj(1);

    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of VCNs supported.
TEST_F(vcn, DISABLED_vcn_workflow_neg_2) {
    // [ Create SetMax+1] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr = "10.0.0.0/16";
    uint32_t num_vcns = 1025;
    vcn_util vcn_obj(1);

    key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key, vcn_start_addr, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key, num_vcns, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing VCN should return entry not found.
TEST_F(vcn, vcn_workflow_neg_3a) {
    // Read NonEx
    vcn_util vcn_obj(1);
    pds_vcn_info_t info = {};
    uint32_t num_subnets = 1024;
    pds_vcn_key_t key = {};

    key.id = 1;
    ASSERT_TRUE(vcn_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(vcn_util::many_read(key, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Deletion of a non-existing VCNs should fail.
TEST_F(vcn, vcn_workflow_neg_3b) {
    // [Delete NonEx]
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key = {};
    pds_vcn_info_t info = {};
    int num_vcns = 1024;
    vcn_util vcn_obj(1);

    key.id = 1;

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_obj.del() == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vcn_util::many_delete(key, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create and delete VCN in two batches
/// The hardware should create VCN correctly
/// and return entry not found after delete
TEST_F(vcn, DISABLED_vcn_workflow_neg_4) {
    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Delete Set3] - Read
    pds_batch_params_t batch_params = {0};
    pds_vcn_key_t key1 = {}, key2 = {}, key3 = {};
    pds_vcn_info_t info = {};
    std::string vcn_start_addr1 = "10.0.0.0/16";
    std::string vcn_start_addr2 = "40.0.0.0/16";
    std::string vcn_start_addr3 = "70.0.0.0/16";
    uint32_t num_vcns = 20;

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key1, vcn_start_addr1, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_create(key2, vcn_start_addr2, num_vcns,
                                      PDS_VCN_TYPE_TENANT) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key1, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key2, num_vcns, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key3, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);

    ASSERT_TRUE(vcn_util::many_read(key1, num_vcns,
                          sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key2, num_vcns, sdk::SDK_RET_OK)
                                    == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_read(key3, num_vcns,
                          sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    //Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key1, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vcn_util::many_delete(key2, num_vcns) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create a VCN with an id which is not within the range.
TEST_F(vcn, DISABLED_vcn_workflow_corner_case_4) {}

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
