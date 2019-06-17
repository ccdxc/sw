//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policy test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/policy.hpp"

namespace api_test {

// Globals
static constexpr uint16_t g_num_policy = PDS_MAX_SECURITY_POLICY;

//----------------------------------------------------------------------------
// Policy test class
//----------------------------------------------------------------------------

class policy : public ::pds_test_base {
protected:
    policy() {}
    virtual ~policy() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
        batch_start();
        sample_vpc_setup(PDS_VPC_TYPE_TENANT);
        batch_commit();
    }
    static void TearDownTestCase() {
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
        batch_start();
        sample_vpc_teardown(PDS_VPC_TYPE_TENANT);
        batch_commit();
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Policy test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SUBNET_TEST
/// @{

/// \brief Create and delete maximum policies in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(policy, policy_workflow_1) {
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    POLICY_MANY_DELETE(&seed);
    batch_commit();
    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete and create max policies in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(policy, policy_workflow_2) {
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    POLICY_MANY_DELETE(&seed);
    POLICY_MANY_CREATE(&seed);
    batch_commit();
    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 policy should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(policy, policy_workflow_3) {
    policy_seed_stepper_t seed1, seed2, seed3;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4,"10.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4,"11.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed3, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
                     num_policy + 1);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed1);
    POLICY_MANY_CREATE(&seed2);
    POLICY_MANY_DELETE(&seed1);
    POLICY_MANY_CREATE(&seed3);
    batch_commit();

    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);

    POLICY_MANY_READ(&seed2, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed2);
    POLICY_MANY_DELETE(&seed3);
    batch_commit();

    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    POLICY_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete max policies in two batches
/// The hardware should create and delete VPC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(policy, policy_workflow_4) {
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit();
    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete mix and match of policies in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(policy, policy_workflow_5) {
    policy_seed_stepper_t seed1, seed2, seed3;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed3, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
                     num_policy + 1);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed1);
    POLICY_MANY_CREATE(&seed2);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_DELETE(&seed1);
    POLICY_MANY_CREATE(&seed3);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    POLICY_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed2);
    POLICY_MANY_DELETE(&seed3);
    batch_commit();

    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    POLICY_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}


/// \brief Create, update and delete maximum number of policies
/// [ Create SetMax, Update SetMax - Update SetMax - Delete SetMax ] - Read
TEST_F(policy, policy_workflow_6) {
    policy_seed_stepper_t seed, seed1, seed2;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    POLICY_MANY_UPDATE(&seed1);
    POLICY_MANY_UPDATE(&seed2);
    POLICY_MANY_DELETE(&seed2);
    batch_commit();
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
}

/// \brief Create, update and delete maximum number of policies
/// [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax -
///   Update SetMax] - Read
TEST_F(policy, policy_workflow_7) {
    policy_seed_stepper_t seed, seed1, seed2;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    POLICY_MANY_DELETE(&seed);
    POLICY_MANY_CREATE(&seed);
    POLICY_MANY_UPDATE(&seed1);
    POLICY_MANY_UPDATE(&seed2);
    batch_commit();
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed2);
    batch_commit();

    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete maximum policies in multiple batches
/// [ Create SetMax, Update SetMax ] - Read - [ Update SetMax ] - Read -
///   [ Delete SetMax ] - Read
TEST_F(policy, DISABLED_policy_workflow_8) {
    policy_seed_stepper_t seed, seed1, seed2;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    POLICY_MANY_UPDATE(&seed1);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_UPDATE(&seed2);
    batch_commit();

    POLICY_MANY_READ(&seed2, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_DELETE(&seed2);
    batch_commit();

    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
}

/// \brief Create, update and delete maximum policies in multiple batches
/// [ Create SetMax ] - Read - [ Update SetMax Delete SetMax ] - Read
TEST_F(policy, policy_workflow_9) {
    policy_seed_stepper_t seed, seed1, seed2;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit();
    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_UPDATE(&seed1);
    POLICY_MANY_DELETE(&seed1);
    batch_commit();

    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
}

/// \brief Create, update and delete a set of policies in multiple batches
/// [ Create Set1, Set2 Set3 - Delete Set1 - Update Set2 ] - Read -
/// [ Update Set3 - Delete Set2 - Create Set4] - Read
TEST_F(policy, DISABLED_policy_workflow_10) {
    policy_seed_stepper_t seed1, seed2, seed3, seed4, seed2_new, seed3_new;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "13.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed2_new, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "14.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed3, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "17.0.0.1/16",
                     num_policy + 1);
    POLICY_SEED_INIT(&seed3_new, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "18.0.0.1/16",
                     num_policy + 1);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed4, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "21.0.0.1/16",
                     num_policy);
    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed1);
    POLICY_MANY_CREATE(&seed2);
    POLICY_MANY_CREATE(&seed3);
    POLICY_MANY_DELETE(&seed1);
    POLICY_MANY_UPDATE(&seed2_new);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    POLICY_MANY_READ(&seed2_new, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed3, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_UPDATE(&seed3_new);
    POLICY_MANY_DELETE(&seed2_new);
    POLICY_MANY_CREATE(&seed4);
    batch_commit();
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    POLICY_MANY_READ(&seed3_new, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed4, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed3_new);
    POLICY_MANY_DELETE(&seed4);
    batch_commit();

    POLICY_MANY_READ(&seed3_new, sdk::SDK_RET_ENTRY_NOT_FOUND);
    POLICY_MANY_READ(&seed4, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of policies in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(policy, policy_workflow_neg_1) {
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit_fail();
    batch_abort();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create more than maximum number of policies supported.
/// [ Create SetMax+1] - Read
// @saratk - need to enable this after fixing the max policy value; search
// for g_max_policy assignmnet at the top. If I use PDS_MAX_SUBNET, things
// are crashing, so left it at 1024 for now.
TEST_F(policy, policy_workflow_neg_2) {
    policy_seed_stepper_t seed;
    uint32_t num_policy = PDS_MAX_SECURITY_POLICY + 3;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit_fail();
    batch_abort();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Read of a non-existing policy should return entry not found.
/// Read NonEx and Delete NonEx
TEST_F(policy, policy_workflow_neg_3) {
    policy_seed_stepper_t seed;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16", 1);

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    batch_start();
    POLICY_MANY_DELETE(&seed);
    batch_commit_fail();
    batch_abort();
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(policy, policy_workflow_neg_4) {
    policy_seed_stepper_t seed1, seed2;
    uint32_t num_policy = 512;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed1);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_DELETE(&seed1);
    POLICY_MANY_DELETE(&seed2);
    batch_commit_fail();
    batch_abort();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed1);
    batch_commit();

    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of policies. Delete and update in the next
/// batch.
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
TEST_F(policy, DISABLED_policy_workflow_neg_5) {
    policy_seed_stepper_t seed, seed1;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_DELETE(&seed);
    POLICY_MANY_UPDATE(&seed1);
    batch_commit_fail();
    batch_abort();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update non existing policies
/// [ Update SetMax ] - Read
TEST_F(policy, policy_workflow_neg_6) {
    policy_seed_stepper_t seed, seed1;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_UPDATE(&seed);
    batch_commit_fail();
    batch_abort();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
}

/// \brief Create maximum number of policies. Update maximum number plus one
/// policy in the next batch.
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
TEST_F(policy, DISABLED_policy_workflow_neg_7) {
    policy_seed_stepper_t seed, seed1;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
                     num_policy + 1);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_UPDATE(&seed1);
    batch_commit_fail();
    batch_abort();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed);
    batch_commit();

    POLICY_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create a set of policies. Update the existing set and a non-existing
/// set of policies in the next batch.
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
TEST_F(policy, policy_workflow_neg_8) {
    policy_seed_stepper_t seed1, seed1_new, seed2;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    POLICY_SEED_INIT(&seed1_new, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);

    policy_id += num_policy;
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "13.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed1);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_UPDATE(&seed1_new);
    POLICY_MANY_UPDATE(&seed2);
    batch_commit_fail();
    batch_abort();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed1);
    batch_commit();

    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create a set of policies. Delete the existing set and update a
/// non-existing set of policies in the next batch.
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
TEST_F(policy, policy_workflow_neg_9) {
    policy_seed_stepper_t seed1, seed2, seed3, seed4, seed2_new, seed3_new;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    POLICY_SEED_INIT(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                     num_policy);
    policy_id += num_policy;
    POLICY_SEED_INIT(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                     POLICY_TYPE_FIREWALL, IP_AF_IPV4, "13.0.0.1/16",
                     num_policy);

    // trigger
    batch_start();
    POLICY_MANY_CREATE(&seed1);
    batch_commit();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);

    batch_start();
    POLICY_MANY_DELETE(&seed1);
    POLICY_MANY_UPDATE(&seed2);
    batch_commit_fail();
    batch_abort();
    POLICY_MANY_READ(&seed1, sdk::SDK_RET_OK);
    POLICY_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // teardown
    batch_start();
    POLICY_MANY_DELETE(&seed1);
    batch_commit();

    POLICY_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    api_test_program_run(argc, argv);
}
