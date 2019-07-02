//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policy test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/policy.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

namespace api_test {

// Globals
static constexpr uint16_t g_num_policy = PDS_MAX_SECURITY_POLICY;
static constexpr uint16_t g_num_stateful_rules = 512;

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

/// \brief POLICY WF_1
TEST_F(policy, policy_workflow_1) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                POLICY_TYPE_FIREWALL,IP_AF_IPV4, "10.0.0.0/16",
		g_num_policy);
    workflow_1<policy_feeder>(feeder);
}

/// \brief POLICY WF_2
TEST_F(policy, policy_workflow_2) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder;

    // setup
    feeder.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		g_num_policy);

    // trigger
    workflow_2<policy_feeder>(feeder);
}

/// \brief POLICY WF_3
TEST_F(policy, policy_workflow_3) {
    pds_policy_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    policy_feeder feeder1, feeder2, feeder3;

    // setup
    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16", 20);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16", 20);
    feeder3.init(key3, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16", 20);

    // trigger
    workflow_3<policy_feeder>(feeder1, feeder2, feeder3);
}

/// \brief POLICY WF_4
TEST_F(policy, policy_workflow_4) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16", 20);
    workflow_4<policy_feeder>(feeder);
}

/// \brief POLICY WF_5
TEST_F(policy, policy_workflow_5) {
    pds_policy_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    policy_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16", 20);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16", 20);
    feeder3.init(key3, g_num_stateful_rules, RULE_DIR_INGRESS,
		 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16", 20);
    workflow_5<policy_feeder>(feeder1, feeder2, feeder3);
}

/// \brief POLICY WF_6
TEST_F(policy, policy_workflow_6) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    feeder1B.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
		  g_num_policy);
    workflow_6<policy_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief POLICY WF_7
TEST_F(policy, policy_workflow_7) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    feeder1B.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
		  g_num_policy);
    workflow_7<policy_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief POLICY WF_8
TEST_F(policy, DISABLED_policy_workflow_8) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    feeder1B.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
		  g_num_policy);
    workflow_8<policy_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief POLICY WF_9
TEST_F(policy, policy_workflow_9) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder1, feeder1A;

    feeder1.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    workflow_9<policy_feeder>(feeder1, feeder1A);
}

/// \brief POLICY WF_10
TEST_F(policy, DISABLED_policy_workflow_10) {
    pds_policy_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                  key4 = {.id = 100};
    policy_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;
    uint32_t num_policy = 20;

    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                 num_policy);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "13.0.0.1/16",
                 num_policy);
    feeder2A.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "14.0.0.1/16",
                  num_policy);
    feeder3.init(key3, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "17.0.0.1/16",
                 num_policy + 1);
    feeder3A.init(key3, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "18.0.0.1/16",
                  num_policy + 1);
    feeder4.init(key4, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "21.0.0.1/16",
                 num_policy);
    workflow_10<policy_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief POLICY WF_N_1
TEST_F(policy, policy_workflow_neg_1) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		g_num_policy);
    workflow_neg_1<policy_feeder>(feeder);
}

/// \brief POLICY WF_N_2
/// \brief Create more than maximum number of policies supported.
/// [ Create SetMax+1] - Read
// @saratk - need to enable this after fixing the max policy value; search
// for g_max_policy assignmnet at the top. If I use PDS_MAX_SUBNET, things
// are crashing, so left it at 1024 for now.
TEST_F(policy, policy_workflow_neg_2) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                g_num_policy + 3);
    workflow_neg_2<policy_feeder>(feeder);
}

/// \brief POLICY WF_N_3
TEST_F(policy, policy_workflow_neg_3) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		g_num_policy);
    workflow_neg_3<policy_feeder>(feeder);
}

/// \brief POLICY WF_N_4
TEST_F(policy, policy_workflow_neg_4) {
    pds_policy_key_t key1 = {.id = 10}, key2 = {.id = 40};
    policy_feeder feeder1, feeder2;
    uint32_t num_policy = 20;

    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16", num_policy);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16", num_policy);
    workflow_neg_4<policy_feeder>(feeder1, feeder2);
}

/// \brief POLICY WF_N_5
TEST_F(policy, DISABLED_policy_workflow_neg_5) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder1, feeder1A;

    feeder1.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    workflow_neg_5<policy_feeder>(feeder1, feeder1A);
}

/// \brief POLICY WF_N_6
TEST_F(policy, policy_workflow_neg_6) {
    pds_policy_key_t key = {.id = 1};
    policy_feeder feeder1, feeder1A;

    feeder1.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    workflow_neg_6<policy_feeder>(feeder1, feeder1A);
}

/// \brief POLICY WF_N_7
TEST_F(policy, policy_workflow_neg_7) {
    pds_policy_key_t key1 = {.id = 10}, key2 = {.id = 40};
    policy_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                  POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "12.0.0.1/16",
		 g_num_policy);
    workflow_neg_7<policy_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief POLICY WF_N_8
TEST_F(policy, policy_workflow_neg_8) {
    pds_policy_key_t key1 = {.id = 10}, key2 = {.id = 40};
    policy_feeder feeder1, feeder2;
    uint32_t num_policy = 20;

    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16", num_policy);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "11.0.0.1/16", num_policy);
    workflow_neg_8<policy_feeder>(feeder1, feeder2);
}

/// \brief POLICY WF_N_9
/// \brief Create a set of policies. Delete the existing set and update a
/// non-existing set of policies in the next batch.
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
TEST_F(policy, policy_workflow_neg_9) {
    policy_feeder feeder1, feeder2;
    uint32_t num_policy = 341;
    pds_policy_key_t key1 = {.id = 1}, key2 = {.id = 1 + num_policy};

    // setup
    feeder1.init(key1, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "10.0.0.1/16",
                 num_policy);
    feeder2.init(key2, g_num_stateful_rules, RULE_DIR_INGRESS,
                 POLICY_TYPE_FIREWALL, IP_AF_IPV4, "13.0.0.1/16",
                 num_policy);

    // trigger
    // workflow_neg_8<policy_feeder>(feeder1, feeder2);
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
