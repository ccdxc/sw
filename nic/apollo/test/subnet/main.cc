//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all subnet test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/include/api/pds_batch.hpp"
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

/// \brief Create a subnet
///
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
///
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
///
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
///
/// Try to create a subnet which exists already. It should fail
TEST_F(subnet, subnet_create_existing) {
    // Validation test case; we can assume agent takes care of it.
    // Leaving it empty
}

/// \brief Read a subnet
///
/// Read a subnet
TEST_F(subnet, subnet_read) {
    g_key.id = 1;
    pds_subnet_read(&g_key, &g_info);
    printf("READ SUBNET key:%d status:%d\n", g_key.id, g_info.status.hw_id);
}

/// \brief Delete a non-existing subnet
///
/// Delete a non-existing subnet
TEST_F(subnet, subnet_delete_non_existing) {
    // Validation test case; we can assume agent takes care of it.
    // Leaving it empty
}

/// \brief Read a non-existing subnet
///
/// Read a non-existing subnet
TEST_F(subnet, subnet_read_non_existing) {
    // Validation test case; we can assume agent takes care of it.
    // Leaving it empty
}

/// \brief Create multiple subnets in a single VCN in a single batch
///
/// Create and Read multiple subnets in a single VCN in a single batch
TEST_F(subnet, multiple_subnets_single_VCN_single_batch_workflow) {
    // Create
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
///
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
///
/// Create multiple subnets in a single VCN in multiple batches
TEST_F(subnet, multiple_subnets_single_VCN_multiple_batches_workflow) {
    // Create
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
///
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
///
/// Create, Read and Delete 1k subnets in single vcn in single batch
TEST_F(subnet, subnet_create_max_subnets_in_single_vcn_in_single_batch) {}

/// \brief Create 1k subnets in multiple vcns in single batch
///
/// Create 1k subnets in multiple vcns in single batch
TEST_F(subnet, subnet_create_max_subnets_in_multiple_vcns_in_single_batch) {}

/// \brief Create 1k subnets in single vcn in multiple batches
///
/// Create 1k subnets in single vcn in multiple batches
TEST_F(subnet, subnet_create_max_subnets_in_single_vcn_in_multiple_batches) {}

/// \brief Create 1k subnets multiple vcns in multiple batches
///
/// Create 1k subnets in multiple vcns in multiple batches
TEST_F(subnet, subnet_create_max_subnets_in_multiple_vcns_in_multiple_batches) {
}

/// \brief Create an overlapping subnet
///
/// Create an invalid/overlapping subnet
/// Validation test case
TEST_F(subnet, subnet_create_invalid_subnet) {}

/// \brief Delete an overlapping subnet
///
/// Delete an invalid/overlapping subnet
/// Validation test case
TEST_F(subnet, subnet_delete_invalid_subnet) {}

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
