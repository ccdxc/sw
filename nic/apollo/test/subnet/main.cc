//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all subnet test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/vcn.hpp"
#include <getopt.h>
#include <gtest/gtest.h>
#include <stdio.h>

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// GLobals
char *g_cfg_file = NULL;
int g_batch_epoch = 1;
int g_vcn_id = 1;
int g_subnet_id = 1;
pds_subnet_key_t g_key = {0};
pds_subnet_info_t g_info = {0};

//----------------------------------------------------------------------------
// Subnet test class
//----------------------------------------------------------------------------

class subnet : public ::pds_test_base {
protected:
    subnet() {}
    virtual ~subnet() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// Subnet test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SUBNET_TEST
/// @{

#if 0
/// \brief Create a subnet
/// Detailed description
TEST_F(subnet, subnet_create) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "10.0.0.0/8");
    v1.create();
    subnet_util s = subnet_util(g_vcn_id, g_subnet_id, "10.1.1.0/16");
    s.create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Delete a Subnet
/// Detailed description
TEST_F(subnet, subnet_delete) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "10.0.0.0/8");
    subnet_util s = subnet_util(g_vcn_id, g_subnet_id, "10.1.1.0/16");
    s.del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create and delete a subnet in the same batch
/// Create and delete a subnet in the same batch. It should be a NO-OP
TEST_F(subnet, create_delete_in_same_batch) {
    pds_batch_params_t batch_params = {0};

    g_vcn_id++;
    g_subnet_id++;
    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "10.0.0.0/8");
    v1.create();
    subnet_util s = subnet_util(g_vcn_id, g_subnet_id, "10.1.1.0/16");
    s.create();
    s.del();
    v1.del();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create an existing subnet
/// Try to create a subnet which exists already. It should fail
TEST_F(subnet, subnet_create_existing) {
    // Validation test case; we can assume agent takes care of it.
    // Leaving it empty
}

/// \brief Read a subnet
/// Read a subnet
TEST_F(subnet, subnet_read) {
    g_key.id = 1;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET key:%d status:%d\n", g_key.id, g_info.status.hw_id);
}

/// \brief Delete a non-existing subnet
/// Delete a non-existing subnet
TEST_F(subnet, subnet_delete_non_existing) {
    // Validation test case; we can assume agent takes care of it.
    // Leaving it empty
}

/// \brief Read a non-existing subnet
/// Read a non-existing subnet
TEST_F(subnet, subnet_read_non_existing) {
    // Validation test case; we can assume agent takes care of it.
    // Leaving it empty
}

/// \brief Create multiple subnets in a single VCN in a single batch
/// Create and Read multiple subnets in a single VCN in a single batch
TEST_F(subnet, multiple_subnets_single_VCN_single_batch_workflow) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    g_vcn_id++;
    g_subnet_id++;
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "10.0.0.0/8");
    v1.create();
    subnet_util s1 = subnet_util(g_vcn_id, g_subnet_id, "11.1.1.0/16");
    g_subnet_id++;
    subnet_util s2 = subnet_util(g_vcn_id, g_subnet_id, "12.1.1.0/16");
    g_subnet_id++;
    subnet_util s3 = subnet_util(g_vcn_id, g_subnet_id, "13.1.1.0/16");
    s1.create();
    s2.create();
    s3.create();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    // Read
    g_key.id = g_subnet_id--;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_key.id = g_subnet_id--;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_key.id = g_subnet_id--;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    // Delete
}

/// \brief Create multiple subnets
/// Create, Read and Delete multiple subnets in multiple VCNS in a single batch
TEST_F(subnet, multiple_subnets_multiple_VCNs_single_batch_workflow) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    g_vcn_id = g_vcn_id + 4;
    g_subnet_id = g_subnet_id + 4;
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "11.0.0.0/8");
    subnet_util v1_s1 = subnet_util(g_vcn_id, g_subnet_id, "11.1.1.0/16");
    g_subnet_id++;
    subnet_util v1_s2 = subnet_util(g_vcn_id, g_subnet_id, "11.2.1.0/16");
    g_subnet_id++;
    subnet_util v1_s3 = subnet_util(g_vcn_id, g_subnet_id, "11.3.1.0/16");
    g_subnet_id++;

    g_vcn_id++;
    vcn_util v2 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "12.0.0.0/8");
    g_subnet_id++;
    subnet_util v2_s1 = subnet_util(g_vcn_id, g_subnet_id, "12.1.1.0/16");
    g_subnet_id++;
    subnet_util v2_s2 = subnet_util(g_vcn_id, g_subnet_id, "12.2.1.0/16");
    g_subnet_id++;
    subnet_util v2_s3 = subnet_util(g_vcn_id, g_subnet_id, "12.3.1.0/16");

    g_vcn_id++;
    vcn_util v3 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "13.0.0.0/8");
    g_subnet_id++;
    subnet_util v3_s1 = subnet_util(g_vcn_id, g_subnet_id, "13.1.1.0/16");
    g_subnet_id++;
    subnet_util v3_s2 = subnet_util(g_vcn_id, g_subnet_id, "13.2.1.0/16");
    g_subnet_id++;
    subnet_util v3_s3 = subnet_util(g_vcn_id, g_subnet_id, "13.3.1.0/16");

    v1.create();
    v2.create();
    v3.create();

    v1_s1.create();
    v1_s2.create();
    v1_s3.create();

    v2_s1.create();
    v2_s2.create();
    v2_s3.create();

    v3_s1.create();
    v3_s2.create();
    v3_s3.create();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
    g_vcn_id = g_vcn_id - 2;

    // Read
    pds_subnet_spec_t g_spec = {0};
    pds_subnet_status_t status = {0};
    pds_subnet_stats_t stats = {};
    g_subnet_id = g_subnet_id - 9;

    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_vcn_id++;
    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_vcn_id++;
    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, status.hw_id);

    // Delete
}

/// \brief Create multiple subnets
/// Create multiple subnets in a single VCN in multiple batches
TEST_F(subnet, multiple_subnets_single_VCN_multiple_batches_workflow) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    g_vcn_id++;
    g_subnet_id++;
    g_subnet_id++;
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "21.0.0.0/8");
    subnet_util v1_s1 = subnet_util(g_vcn_id, g_subnet_id, "21.1.1.0/16");
    g_subnet_id++;
    subnet_util v1_s2 = subnet_util(g_vcn_id, g_subnet_id, "21.2.1.0/16");
    g_subnet_id++;

    v1.create();
    v1_s1.create();
    v1_s2.create();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    subnet_util v1_s3 = subnet_util(g_vcn_id, g_subnet_id, "21.3.1.0/16");
    g_subnet_id++;

    v1_s3.create();

    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    // Read
    g_subnet_id = g_subnet_id - 3;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    // Delete
}

/// \brief Create multiple subnets
/// Create multiple subnets in multiple VCNs in multiple batches
TEST_F(subnet, multiple_subnets_multiple_vcns_multiple_batches_workflow) {
    pds_batch_params_t batch_params = {0};

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    g_vcn_id++;
    g_subnet_id = g_subnet_id + 2;
    vcn_util v1 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "11.0.0.0/8");
    subnet_util v1_s1 = subnet_util(g_vcn_id, g_subnet_id, "11.1.1.0/16");

    v1.create();
    v1_s1.create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    batch_params.epoch = g_batch_epoch++;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    g_vcn_id++;
    vcn_util v2 = vcn_util(PDS_VCN_TYPE_TENANT, g_vcn_id, "12.0.0.0/8");
    g_subnet_id++;
    subnet_util v2_s1 = subnet_util(g_vcn_id, g_subnet_id, "12.1.1.0/16");
    g_subnet_id++;
    subnet_util v2_s2 = subnet_util(g_vcn_id, g_subnet_id, "12.2.1.0/16");

    v2.create();
    v2_s1.create();
    v2_s2.create();
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

    // Read
    g_subnet_id = g_subnet_id - 2;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);

    g_subnet_id++;
    g_key.id = g_subnet_id;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET vcn_id : %u, key : %u, status : %u\n",
           g_info.spec.vcn.id, g_key.id, g_info.status.hw_id);
}

/// \brief Create 1k subnets in single vcn in single batch
/// Create, Read and Delete 1k subnets in single vcn in single batch
TEST_F(subnet, subnet_create_max_subnets_in_single_vcn_in_single_batch) {}

/// \brief Create 1k subnets in multiple vcns in single batch
/// Create 1k subnets in multiple vcns in single batch
TEST_F(subnet, subnet_create_max_subnets_in_multiple_vcns_in_single_batch) {}

/// \brief Create 1k subnets in single vcn in multiple batches
/// Create 1k subnets in single vcn in multiple batches
TEST_F(subnet, subnet_create_max_subnets_in_single_vcn_in_multiple_batches) {}

/// \brief Create 1k subnets multiple vcns in multiple batches
/// Create 1k subnets in multiple vcns in multiple batches
TEST_F(subnet, subnet_create_max_subnets_in_multiple_vcns_in_multiple_batches) {
}

/// \brief Create an overlapping subnet
/// Create an invalid/overlapping subnet
TEST_F(subnet, subnet_create_invalid_subnet) {}

/// \brief Delete an overlapping subnet
/// Delete an invalid/overlapping subnet
TEST_F(subnet, subnet_delete_invalid_subnet) {}
#endif

/// \brief Create and delete maximum number of subnets supported
/// in the same batch. The create and delete operations are
/// de-duped by framework and is a NO-OP from hardware perspective.
TEST_F(subnet, DISABLED_subnet_workflow_1)
{
    // [ Create SetMax, Delete SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {};
    pds_subnet_info_t info = {};
    std::string subnet_start_addr = "10.0.0.0/16";
    uint32_t num_subnets = 1024;
    pds_vcn_key_t vcn_key = {};

    key.id = 1;
    vcn_key.id = 1;

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, subnet_start_addr, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_delete(key, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max subnets in the same batch.
/// create and delete are de-deduped by framework and subsequent create
/// should result in successful creation:w
TEST_F(subnet, DISABLED_subnet_workflow_2) {
    // [ Create SetMax - Delete SetMax - Create SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {};
    pds_subnet_info_t info = {};
    std::string start_addr = "10.0.0.0/16";
    uint32_t num_subnets = 1024;
    pds_vcn_key_t vcn_key = {};

    key.id = 1;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, start_addr, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_delete(key, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, start_addr, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets) == sdk::SDK_RET_OK);
}

/// \brief Create two sets of subnets viz set1 and set2. Delete set1.
/// Create a new set - set3. Read of a subnet from set1 should fail.
/// Reading subnets from set2 and set3 should be successful.
TEST_F(subnet, DISABLED_subnet_workflow_3) {
    // [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {}, key2 = {}, key3 = {};
    pds_subnet_info_t info = {};
    std::string subnet_start_addr1 = "10.0.0.0/16";
    std::string subnet_start_addr2 = "30.0.0.0/16";
    std::string subnet_start_addr3 = "60.0.0.0/16";
    uint32_t num_subnets = 20;
    pds_vcn_key_t vcn_key = {};

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_create(key1, vcn_key, subnet_start_addr1, num_subnets)
                                         == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key2, vcn_key, subnet_start_addr2, num_subnets)
                                         == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_delete(key1, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key3, vcn_key, subnet_start_addr3, num_subnets)
                                         == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key1, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_read(key2, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_read(key3, num_subnets) == sdk::SDK_RET_OK);
}

/// \brief Create and delete maximum number of subnets in two batches
/// Creation should be successful and read should return appropriately.
TEST_F(subnet, DISABLED_subnet_workflow_4) {
    // [ Create SetMax ] - Read - [ Delete SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {};
    pds_subnet_info_t info = {};
    std::string start_addr = "10.0.0.0/16";
    uint32_t num_subnets = 1024;
    pds_vcn_key_t vcn_key = {};

    key.id = 1;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, start_addr,
                                         num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_delete(key, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
}

/// \brief Create two sets of subnets viz set1 and set2 in one batch.
/// In another batch delete set1 and create a new set set3.
/// Reading subnets from set1 should fail and from set2 & set3 should be
/// successful.
TEST_F(subnet, DISABLED_subnet_workflow_5) {
    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {}, key2 = {}, key3 = {};
    pds_subnet_info_t info = {};
    std::string subnet_start_addr1 = "10.0.0.0/16";
    std::string subnet_start_addr2 = "30.0.0.0/16";
    std::string subnet_start_addr3 = "60.0.0.0/16";
    uint32_t num_subnets = 20;
    pds_vcn_key_t vcn_key = {};

    key1.id = 10;
    key2.id = 40;
    key3.id = 70;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_create(key1, vcn_key, subnet_start_addr1, num_subnets)
                                         == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key2, vcn_key, subnet_start_addr2, num_subnets)
                                         == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key1, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_read(key2, num_subnets) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_delete(key1, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key3, vcn_key, subnet_start_addr3, num_subnets)
                                         == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key1, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_read(key3, num_subnets) == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of subnets in two batches
/// The hardware should program subnets correctly in case of
/// first create and return error in second create operation
TEST_F(subnet, DISABLED_subnet_workflow_neg_1) {
    // [ Create SetMax ] - [ Create SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {};
    pds_subnet_info_t info = {};
    std::string start_addr = "10.0.0.0/16";
    uint32_t num_subnets = 1024;
    pds_vcn_key_t vcn_key = {};

    key.id = 1;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, start_addr, num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, start_addr, num_subnets) != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of subnets supported.
TEST_F(subnet, DISABLED_subnet_workflow_neg_2) {
    // [ Create SetMax+1] - Read
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {};
    pds_subnet_info_t info = {};
    std::string start_addr = "10.0.0.0/16";
    uint32_t num_subnets = 1025;
    pds_vcn_key_t vcn_key = {};

    key.id = 1;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_util::many_create(key, vcn_key, start_addr,
                                         num_subnets) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(subnet_util::many_read(key, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
                sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing subnet should return entry not found.
TEST_F(subnet, subnet_workflow_neg_3a) {
    // Read NonEx
    pds_subnet_key_t key = {};
    uint32_t num_subnets = 1024;
    pds_subnet_info_t info = {};
    subnet_util subnet_obj(1);

    key.id  = 1;
    ASSERT_TRUE(subnet_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(subnet_util::many_read(key, num_subnets, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Deletion of a non-existing subnet should fail.
TEST_F(subnet, subnet_workflow_neg_3b) {
    // [Delete NonEx]
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {};
    std::string start_addr = "10.0.0.0/16";
    uint32_t num_subnets = 1024;
    pds_vcn_key_t vcn_key = {};
    subnet_util subnet_obj(1);

    key.id = 1;
    vcn_key.id = 1;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(subnet_obj.del() == sdk::SDK_RET_OK);
    //ASSERT_TRUE(subnet_util::many_delete(key, num_subnets) != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

}
/// \brief Subnet workflow corner case 4
///
/// [ Create SetCorner ] - Read
TEST_F(subnet, DISABLED_subnet_workflow_corner_case_4) {}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
subnet_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
}

static void
subnet_test_options_parse (int argc, char **argv)
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
subnet_test_options_validate (void)
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
    subnet_test_options_parse(argc, argv);
    if (subnet_test_options_validate() != SDK_RET_OK) {
        subnet_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
