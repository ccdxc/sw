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
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;
    std::string start_pfx = "10.0.0.0/16";

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete and create max VPCs in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(vpc, vpc_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    VPC_MANY_DELETE(&seed);
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 vpc should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(vpc, vpc_workflow_3) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "30.0.0.0/16";
    std::string start_pfx3 = "60.0.0.0/16";
    uint32_t num_vpcs = 20;
    vpc_stepper_seed_t seed1, seed2, seed3;

    // setup
    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, start_pfx1, num_vpcs);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, start_pfx2, num_vpcs);
    VPC_SEED_INIT(&seed3, key3, PDS_VPC_TYPE_TENANT, start_pfx3, num_vpcs);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    VPC_MANY_CREATE(&seed2);
    VPC_MANY_DELETE(&seed1);
    VPC_MANY_CREATE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed2, sdk::SDK_RET_OK);
    VPC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed2);
    VPC_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete VPC in two batches
/// The hardware should create and delete VPC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(vpc, vpc_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete mix and match of VPC in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(vpc, vpc_workflow_5) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "40.0.0.0/16";
    std::string start_pfx3 = "70.0.0.0/16";
    uint32_t num_vpcs = 20;
    vpc_stepper_seed_t seed1, seed2, seed3;

    // setup
    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, start_pfx1, num_vpcs);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, start_pfx2, num_vpcs);
    VPC_SEED_INIT(&seed3, key3, PDS_VPC_TYPE_TENANT, start_pfx3, num_vpcs);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    VPC_MANY_CREATE(&seed2);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);
    VPC_MANY_READ(&seed2, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_DELETE(&seed1);
    VPC_MANY_CREATE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed2, sdk::SDK_RET_OK);
    VPC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed2);
    VPC_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete maximum number of vpcs
/// [ Create SetMax, Update SetMax - Update SetMax - Delete SetMax ] - Read
TEST_F(vpc, vpc_workflow_6) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    VPC_MANY_UPDATE(&seed);
    VPC_MANY_UPDATE(&seed);
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
}

/// \brief Create, update and delete maximum number of vpcs
/// [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax -
///   Update SetMax] - Read
TEST_F(vpc, vpc_workflow_7) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "11.0.0.0/16";
    std::string start_pfx3 = "12.0.0.0/16";
    vpc_stepper_seed_t seed1, seed2, seed3;

    // setup
    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, start_pfx1, k_max_vpc);
    VPC_SEED_INIT(&seed2, key, PDS_VPC_TYPE_TENANT, start_pfx2, k_max_vpc);
    VPC_SEED_INIT(&seed3, key, PDS_VPC_TYPE_TENANT, start_pfx3, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    VPC_MANY_DELETE(&seed1);
    VPC_MANY_CREATE(&seed1);
    VPC_MANY_UPDATE(&seed2);
    VPC_MANY_UPDATE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete maximum number of vpcs
/// [ Create SetMax, Update SetMax ] - Read - [ Update SetMax ] - Read -
///   [ Delete SetMax ] - Read
TEST_F(vpc, DISABLED_vpc_workflow_8) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "11.0.0.0/16";
    std::string start_pfx3 = "12.0.0.0/16";
    vpc_stepper_seed_t seed1, seed2, seed3;

    // setup
    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, start_pfx1, k_max_vpc);
    VPC_SEED_INIT(&seed2, key, PDS_VPC_TYPE_TENANT, start_pfx2, k_max_vpc);
    VPC_SEED_INIT(&seed3, key, PDS_VPC_TYPE_TENANT, start_pfx3, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    VPC_MANY_UPDATE(&seed2);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed2, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_UPDATE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
    // cleanup
}

/// \brief Create, update and delete maximum number of vpcs
/// [ Create SetMax ] - Read - [ Update SetMax Delete SetMax ] - Read
TEST_F(vpc, vpc_workflow_9) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "11.0.0.0/16";
    vpc_stepper_seed_t seed1, seed2;

    // setup
    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, start_pfx1, k_max_vpc);
    VPC_SEED_INIT(&seed2, key, PDS_VPC_TYPE_TENANT, start_pfx2, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_UPDATE(&seed2);
    VPC_MANY_DELETE(&seed2);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
}

/// \brief Create, update and delete maximum number of vpcs
/// [ Create Set1, Set2 Set3 - Delete Set1 - Update Set2 ] - Read -
/// [ Update Set3 - Delete Set2 - Create Set4] - Read
TEST_F(vpc, DISABLED_vpc_workflow_10) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                  key4 = {.id = 100};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "11.0.0.0/16";
    std::string start_pfx2_new = "12.0.0.0/16";
    std::string start_pfx3 = "13.0.0.0/16";
    std::string start_pfx3_new = "14.0.0.0/16";
    std::string start_pfx4 = "15.0.0.0/16";
    vpc_stepper_seed_t seed1, seed2, seed3, seed4, seed2_new, seed3_new;
    uint32_t num_vpcs = 20;

    // setup
    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, start_pfx1, num_vpcs);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, start_pfx2, num_vpcs);
    VPC_SEED_INIT(&seed2_new, key2, PDS_VPC_TYPE_TENANT, start_pfx2, num_vpcs);
    VPC_SEED_INIT(&seed3, key3, PDS_VPC_TYPE_TENANT, start_pfx3, num_vpcs);
    VPC_SEED_INIT(&seed3_new, key3, PDS_VPC_TYPE_TENANT, start_pfx3, num_vpcs);
    VPC_SEED_INIT(&seed4, key4, PDS_VPC_TYPE_TENANT, start_pfx4, num_vpcs);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    VPC_MANY_CREATE(&seed2);
    VPC_MANY_CREATE(&seed3);
    VPC_MANY_DELETE(&seed1);
    VPC_MANY_UPDATE(&seed2_new);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed2, sdk::SDK_RET_OK);
    VPC_MANY_READ(&seed2_new, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_UPDATE(&seed3_new);
    VPC_MANY_DELETE(&seed2);
    VPC_MANY_CREATE(&seed4);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed3_new, sdk::SDK_RET_OK);
    VPC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed4, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed3);
    VPC_MANY_DELETE(&seed4);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VPC_MANY_READ(&seed4, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of VPCs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(vpc, vpc_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create more than maximum number of VPCs supported.
/// [ Create SetMax+1] - Read
TEST_F(vpc, vpc_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc + 1);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Read of a non-existing VPC should return entry not found.
/// Read NonEx
TEST_F(vpc, vpc_workflow_neg_3a) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "0.0.0.0/0", k_max_vpc);

    // trigger
    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Deletion of a non-existing VPCs should fail.
/// [Delete NonEx]
TEST_F(vpc, vpc_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "0.0.0.0/0", k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(vpc, vpc_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "40.0.0.0/16";
    uint32_t num_vpcs = 20;
    vpc_stepper_seed_t seed1, seed2;

    // setup
    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, start_pfx1, k_max_vpc);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, start_pfx2, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_DELETE(&seed1);
    VPC_MANY_DELETE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of policies. Delete and update in the next
/// batch.
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
TEST_F(vpc, DISABLED_vpc_workflow_neg_5) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    std::string start_pfx_new = "11.0.0.0/16";
    vpc_stepper_seed_t seed, seed_new;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);
    VPC_SEED_INIT(&seed_new, key, PDS_VPC_TYPE_TENANT, start_pfx_new,
                  k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_DELETE(&seed);
    VPC_MANY_UPDATE(&seed_new);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of policies. Delete and update in the next
/// batch.
/// [ Update SetMax ] - Read
TEST_F(vpc, vpc_workflow_neg_6) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    vpc_stepper_seed_t seed;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);

    // trigger
    BATCH_START();
    VPC_MANY_UPDATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
}

/// \brief Create maximum number of policies. Delete and update in the next
/// batch.
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
TEST_F(vpc, vpc_workflow_neg_7) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key = {.id = 1};
    std::string start_pfx = "10.0.0.0/16";
    std::string start_pfx_new = "11.0.0.0/16";
    vpc_stepper_seed_t seed, seed_new;

    // setup
    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, start_pfx, k_max_vpc);
    VPC_SEED_INIT(&seed_new, key, PDS_VPC_TYPE_TENANT, start_pfx_new,
                  k_max_vpc + 1);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_UPDATE(&seed_new);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of policies. Delete and update in the next
/// batch.
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
TEST_F(vpc, vpc_workflow_neg_8) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "11.0.0.0/16";
    std::string start_pfx1_new = "12.0.0.0/16";
    vpc_stepper_seed_t seed1, seed2, seed1_new;
    uint32_t num_vpcs = 20;

    // setup
    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, start_pfx1, num_vpcs);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, start_pfx2, num_vpcs);
    VPC_SEED_INIT(&seed1_new, key1, PDS_VPC_TYPE_TENANT, start_pfx1_new,
                  num_vpcs);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_UPDATE(&seed1_new);
    VPC_MANY_UPDATE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of policies. Delete and update in the next
/// batch.
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
TEST_F(vpc, vpc_workflow_neg_9) {
    pds_batch_params_t batch_params = {0};
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    std::string start_pfx1 = "10.0.0.0/16";
    std::string start_pfx2 = "11.0.0.0/16";
    vpc_stepper_seed_t seed1, seed2;
    uint32_t num_vpcs = 20;

    // setup
    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, start_pfx1, num_vpcs);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, start_pfx2, num_vpcs);

    // trigger
    BATCH_START();
    VPC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    VPC_MANY_DELETE(&seed1);
    VPC_MANY_UPDATE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VPC_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    VPC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
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
