//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policy test cases
///
//----------------------------------------------------------------------------

#include <getopt.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/policy.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
uint16_t g_num_policy = PDS_MAX_POLICY; // can overwrite using cmd line
static pds_epoch_t g_batch_epoch = 1; // PDS_EPOCH_INVALID;

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
        test_case_params_t params;
        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(1, "10.0.0.0/8");

        batch_params.epoch = ++g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(vpc_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    }
    static void TearDownTestCase() {
        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(1, "10.0.0.0/8");

        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
        batch_params.epoch = ++g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(vpc_obj.del() == sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Policy test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SUBNET_TEST
/// @{

static inline void
policy_seed_stepper_init(policy_seed_stepper_t *seed, uint32_t id,
                         uint32_t stateless_rules, rule_dir_t dir,
                         policy_type_t type, uint8_t af, std::string pfx)
{
    SDK_ASSERT(stateless_rules < PDS_MAX_RULES_PER_SECURITY_POLICY);
    seed->id = id;
    seed->num_rules = PDS_MAX_RULES_PER_SECURITY_POLICY;
    seed->direction = dir;
    seed->type = type;
    seed->af = af;
    seed->stateful_rules = PDS_MAX_RULES_PER_SECURITY_POLICY - stateless_rules;
    seed->pfx = pfx;
}

/// \brief Create and delete maximum policies in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(policy, policy_workflow_1) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max policies in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(policy, policy_workflow_2) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed, num_policy) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 policy should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(policy, policy_workflow_3) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed1, seed2, seed3;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4,"10.0.0.1/16");

    policy_id += num_policy;
    policy_seed_stepper_init(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4,"11.0.0.1/16");

    policy_id += num_policy;
    policy_seed_stepper_init(&seed3, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed3, num_policy + 1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed1, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    ASSERT_TRUE(policy_util::many_read(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed3, num_policy + 1) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed3, num_policy + 1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed2, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed3, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

}

/// \brief Create and delete max policies in two batches
/// The hardware should create and delete VPC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(policy, policy_workflow_4) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed, num_policy) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(policy_util::many_read(&seed, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of policies in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(policy, policy_workflow_5) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed1, seed2, seed3;
    uint32_t num_policy = 341;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    policy_id += num_policy;
    policy_seed_stepper_init(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16");

    policy_id += num_policy;
    policy_seed_stepper_init(&seed3, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed1, num_policy) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed3, num_policy + 1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed1, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed3, num_policy + 1) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed3, num_policy + 1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of policies in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(policy, policy_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed;
    uint32_t num_policy = g_num_policy;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed, num_policy) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed, num_policy) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of policies supported.
/// [ Create SetMax+1] - Read
// @saratk - need to enable this after fixing the max policy value; search
// for g_max_policy assignmnet at the top. If I use PDS_MAX_SUBNET, things
// are crashing, so left it at 1024 for now.
TEST_F(policy, policy_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed;
    uint32_t num_policy = PDS_MAX_POLICY + 1;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing policy should return entry not found.
/// Read NonEx and Delete NonEx
TEST_F(policy, policy_workflow_neg_3) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    ASSERT_TRUE(policy_util::many_read(&seed, 1,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed, 1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(policy, policy_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    policy_seed_stepper_t seed1, seed2;
    uint32_t num_policy = 512;
    uint32_t policy_id = 1;

    // setup
    policy_seed_stepper_init(&seed1, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16");

    policy_id += num_policy;
    policy_seed_stepper_init(&seed2, policy_id, 512, RULE_DIR_INGRESS,
                             POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16");

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_create(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed1, num_policy) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed2, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed2, num_policy,
                                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_read(&seed1, num_policy) == sdk::SDK_RET_OK);

    // teardown
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(policy_util::many_delete(&seed1, num_policy) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
policy_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
}

static void
policy_test_options_parse (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {"num_policy", required_argument, NULL, 'n'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:n:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        case 'n':
            api_test::g_num_policy = atoi(optarg);
        default:    // ignore all other options
            break;
        }
    }
}

static inline sdk_ret_t
policy_test_options_validate (void)
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
    policy_test_options_parse(argc, argv);
    if (policy_test_options_validate() != SDK_RET_OK) {
        policy_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
