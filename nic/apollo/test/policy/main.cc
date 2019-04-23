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
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/policy.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
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
    }
    static void TearDownTestCase() {
        pds_batch_params_t batch_params = {0};
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
    pds_batch_params_t batch_params = {0};
    pds_policy_key_t key = {};
    rule_t *rule = NULL;
    rule_t *rules = NULL;
    uint32_t num_rules = 10;
    std::string pfx = "10.0.0.1/24";
    int policy_id = 1;

    key.id = 1;

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    rules =
        (rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                             num_rules * sizeof(rule_t));
    printf("%s:%d rules:%p\n", __func__, __LINE__, rules);
    for (uint32_t i = 0; i < num_rules; i++) {
        rules[i].stateful = TRUE;
        rules[i].match.l3_match.ip_proto = 1;
        SDK_ASSERT(str2ipv4pfx((char*)pfx.c_str(),
                               &rules[i].match.l3_match.ip_pfx) == 0);
        rules[i].match.l4_match.icmp_type = 1;
        rules[i].match.l4_match.icmp_code = 1;
        rules[i].action_data.fw_action.action = SECURITY_RULE_ACTION_ALLOW;
    }
    policy_util policy_obj(policy_id, num_rules, rules);
    ASSERT_TRUE(policy_obj.create() == sdk::SDK_RET_OK);

    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    //ASSERT_TRUE(policy_util::many_read(
    //    key, k_max_policy, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);


}

/// \brief Create, delete and create max policies in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(policy, DISABLED_policy_workflow_2) {
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 policy should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(policy, policy_workflow_3) {
}

/// \brief Create and delete max policies in two batches
/// The hardware should create and delete VPC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(policy, policy_workflow_4) {

}

/// \brief Create and delete mix and match of policies in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(policy, policy_workflow_5) {

}

/// \brief Create maximum number of VPCs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(policy, policy_workflow_neg_1) {

}

/// \brief Create more than maximum number of policies supported.
/// [ Create SetMax+1] - Read
// @saratk - need to enable this after fixing the max policy value; search
// for g_max_policy assignmnet at the top. If I use PDS_MAX_SUBNET, things
// are crashing, so left it at 1024 for now.
TEST_F(policy, policy_workflow_neg_2) {

}

/// \brief Read of a non-existing policy should return entry not found.
/// Read NonEx
TEST_F(policy, policy_workflow_neg_3a) {
}

/// \brief Deletion of a non-existing policies should fail.
/// [Delete NonEx]
TEST_F(policy, policy_workflow_neg_3b) {
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(policy, policy_workflow_neg_4) {
}

/// \brief Policy workflow corner case 4
///
/// [ Create SetCorner ] - Read
TEST_F(policy, policy_workflow_corner_case_4) {}

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
