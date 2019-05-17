//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all subnet test cases
///
//----------------------------------------------------------------------------

#include <getopt.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
static pds_epoch_t g_batch_epoch = 1; // PDS_EPOCH_INVALID;
constexpr int k_max_subnet = PDS_MAX_SUBNET;

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
        test_case_params_t params;
        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);

        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(1, "10.0.0.0/8");

        BATCH_START();
        ASSERT_TRUE(vpc_obj.create() == sdk::SDK_RET_OK);
        BATCH_COMMIT();
    }
    static void TearDownTestCase() {
        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(1, "10.0.0.0/8");

        BATCH_START();
        ASSERT_TRUE(vpc_obj.del() == sdk::SDK_RET_OK);
        BATCH_COMMIT();

        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Subnet test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SUBNET_TEST
/// @{

/// \brief Create and delete maximum subnets in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(subnet, subnet_workflow_1) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed = {0};

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, k_max_subnet);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed);
    SUBNET_MANY_DELETE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete and create max subnets in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(subnet, subnet_workflow_2) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    pds_vpc_key_t vpc_key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    subnet_util_stepper_seed_t seed = {0};
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, k_max_subnet);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed);
    SUBNET_MANY_DELETE(&seed);
    SUBNET_MANY_CREATE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 subnet should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(subnet, subnet_workflow_3) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "30.0.0.0/16";
    std::string start_addr3 = "60.0.0.0/16";
    uint32_t num_subnets = 20;
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2, seed3;

    // setup
    SUBNET_SEED_INIT(&seed1, key1, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key2, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed3, key3, vpc_key, start_addr3, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_CREATE(&seed2);
    SUBNET_MANY_DELETE(&seed1);
    SUBNET_MANY_CREATE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_OK);
    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed2);
    SUBNET_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete max subnets in two batches
/// The hardware should create and delete subnet correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(subnet, subnet_workflow_4) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed = {};
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_DELETE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
}

/// \brief Create and delete mix and match of subnets in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(subnet, subnet_workflow_5) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "30.0.0.0/16";
    std::string start_addr3 = "60.0.0.0/16";
    uint32_t num_subnets = 20;
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2, seed3;

    // setup
    SUBNET_SEED_INIT(&seed1, key1, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key2, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed3, key3, vpc_key, start_addr3, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_CREATE(&seed2);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);
    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    SUBNET_MANY_CREATE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_OK);
    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed2);
    SUBNET_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, Update and delete maximum subnets in single batch
/// [ Create SetMax, Update SetMax - Update SetMax - Delete SetMax ] - Read
TEST_F(subnet, subnet_workflow_6) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    std::string start_addr3 = "12.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2, seed3;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed1, key, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed3, key, vpc_key, start_addr3, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_UPDATE(&seed2);
    SUBNET_MANY_UPDATE(&seed3);
    SUBNET_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
}

/// \brief Create, delete, create and update maximum subnets in single batch
/// [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax -
//    Update SetMax ] - Read
TEST_F(subnet, subnet_workflow_7) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    std::string start_addr3 = "12.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2, seed3;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed1, key, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed3, key, vpc_key, start_addr3, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_DELETE(&seed2);
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_UPDATE(&seed2);
    SUBNET_MANY_UPDATE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, Update and delete maximum subnets in single batch
/// [ Create SetMax, Update SetMax ] - Read - [ Update SetMax ] - Read -
/// [ Delete SetMax ] - Read
TEST_F(subnet, DISABLED_subnet_workflow_8) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    std::string start_addr3 = "12.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2, seed3;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed1, key, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed3, key, vpc_key, start_addr3, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_UPDATE(&seed2);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_UPDATE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, Update and delete maximum subnets in single batch
/// [ Create SetMax ] - Read - [ Update SetMax Delete SetMax ] - Read
TEST_F(subnet, subnet_workflow_9) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed1, key, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key, vpc_key, start_addr2, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_UPDATE(&seed2);
    SUBNET_MANY_DELETE(&seed2);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
}

/// \brief Create, Update and delete maximum subnets in single batch
/// [ Create Set1, Set2 Set3 - Delete Set1 - Update Set2 ] - Read -
/// [ Update Set3 - Delete Set2 - Create Set4] - Read
TEST_F(subnet, DISABLED_subnet_workflow_10) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                     key4 = {.id = 100};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "40.0.0.0/16";
    std::string start_addr2_new = "140.0.0.0/16";
    std::string start_addr3 = "70.0.0.0/16";
    std::string start_addr3_new = "170.0.0.0/16";
    std::string start_addr4 = "100.0.0.0/16";
    uint32_t num_subnets = 20;
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2, seed3, seed4, seed2_new, seed3_new;

    // setup
    SUBNET_SEED_INIT(&seed1, key1, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key2, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed2_new, key2, vpc_key, start_addr2_new, num_subnets);
    SUBNET_SEED_INIT(&seed3, key3, vpc_key, start_addr3, num_subnets);
    SUBNET_SEED_INIT(&seed3_new, key3, vpc_key, start_addr3_new, num_subnets);
    SUBNET_SEED_INIT(&seed4, key4, vpc_key, start_addr4, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    SUBNET_MANY_CREATE(&seed2);
    SUBNET_MANY_CREATE(&seed3);
    SUBNET_MANY_DELETE(&seed1);
    SUBNET_MANY_UPDATE(&seed2_new);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed2_new, sdk::SDK_RET_OK);
    SUBNET_MANY_READ(&seed3, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_UPDATE(&seed3_new);
    SUBNET_MANY_DELETE(&seed2_new);
    SUBNET_MANY_CREATE(&seed4);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed3_new, sdk::SDK_RET_OK);
    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed4, sdk::SDK_RET_OK);

    //cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed4);
    SUBNET_MANY_DELETE(&seed3_new);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed4, sdk::SDK_RET_ENTRY_NOT_FOUND);
    SUBNET_MANY_READ(&seed3_new, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of subnets in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(subnet, subnet_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_CREATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create more than maximum number of subnets supported.
/// [ Create SetMax+1] - Read
TEST_F(subnet, subnet_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed;
    uint32_t num_subnets = k_max_subnet + 1;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Read of a non-existing subnet should return entry not found.
/// Read NonEx
TEST_F(subnet, subnet_workflow_neg_3a) {
    pds_subnet_key_t key = {.id = 1};
    subnet_util_stepper_seed_t seed;
    pds_vpc_key_t vpc_key;
    std::string start_addr = "0.0.0.0";
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, num_subnets);

    // trigger
    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Deletion of a non-existing subnet should fail.
/// [Delete NonEx]
TEST_F(subnet, subnet_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed;
    int num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_DELETE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(subnet, subnet_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "40.0.0.0/16";
    uint32_t num_subnets = 20;
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2;

    // setup
    SUBNET_SEED_INIT(&seed1, key1, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key2, vpc_key, start_addr2, num_subnets);

    // trigger
    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    SUBNET_MANY_DELETE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);
    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
TEST_F(subnet, DISABLED_subnet_workflow_neg_5) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed1_new;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed1, key, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed1_new, key, vpc_key, start_addr2, num_subnets);

    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    SUBNET_MANY_UPDATE(&seed1_new);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Update SetMax ] - Read
TEST_F(subnet, subnet_workflow_neg_6) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr = "10.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed, key, vpc_key, start_addr, num_subnets);

    BATCH_START();
    SUBNET_MANY_UPDATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
TEST_F(subnet, DISABLED_subnet_workflow_neg_7) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key = {.id = 1};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed1_new;
    uint32_t num_subnets = k_max_subnet;

    // setup
    SUBNET_SEED_INIT(&seed1, key, vpc_key, start_addr1, num_subnets);
    num_subnets = k_max_subnet + 1;
    SUBNET_SEED_INIT(&seed1_new, key, vpc_key, start_addr2, num_subnets);

    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_UPDATE(&seed1_new);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed1_new, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
TEST_F(subnet, subnet_workflow_neg_8) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    std::string start_addr3 = "12.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed1_new, seed2;
    uint32_t num_subnets = 20;

    // setup
    SUBNET_SEED_INIT(&seed1, key1, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed1_new, key1, vpc_key, start_addr2, num_subnets);
    SUBNET_SEED_INIT(&seed2, key2, vpc_key, start_addr3, num_subnets);

    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_UPDATE(&seed1_new);
    SUBNET_MANY_UPDATE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
TEST_F(subnet, subnet_workflow_neg_9) {
    pds_batch_params_t batch_params = {0};
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    std::string start_addr1 = "10.0.0.0/16";
    std::string start_addr2 = "11.0.0.0/16";
    pds_vpc_key_t vpc_key = {.id = 1};
    subnet_util_stepper_seed_t seed1, seed2;
    uint32_t num_subnets = 20;

    // setup
    SUBNET_SEED_INIT(&seed1, key1, vpc_key, start_addr1, num_subnets);
    SUBNET_SEED_INIT(&seed2, key2, vpc_key, start_addr2, num_subnets);

    BATCH_START();
    SUBNET_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    SUBNET_MANY_UPDATE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    SUBNET_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    SUBNET_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Subnet workflow corner case 4
///
/// [ Create SetCorner ] - Read
TEST_F(subnet, subnet_workflow_corner_case_4) {}

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
