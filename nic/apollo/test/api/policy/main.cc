//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all policy test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/policy.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static uint16_t g_num_policy;
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
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
        // the scale number should be based on memory profile
        if (apulu()) {
            // for 4G profile, we can only support 64
            g_num_policy = 128;
        } else if (apollo()) {
            g_num_policy = 2048;
        } else {
            g_num_policy = PDS_MAX_SECURITY_POLICY;
        }
        pds_batch_ctxt_t bctxt = batch_start();
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
        pds_batch_ctxt_t bctxt = batch_start();
        sample_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Policy test cases implementation
//----------------------------------------------------------------------------

/// \defgroup POLICY_TEST Policy Tests
/// @{

/// \brief Policy WF_B1
/// \ref WF_B1
TEST_F(policy, policy_workflow_b1) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    // v4 policy
    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.0/16", 1);
    workflow_b1<policy_feeder>(feeder);

    if (!apulu()) {
        // v6 policy
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64", 1);
        workflow_b1<policy_feeder>(feeder);
    }
}

/// \brief Policy WF_B2
/// \ref WF_B2
TEST_F(policy, policy_workflow_b2) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A;

    // v4 policy
    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.0/16", 1);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.0/16", 1);
    workflow_b2<policy_feeder>(feeder1, feeder1A);

    if (!apulu()) {
        // v6 policy
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "1001::1/64", 1);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64", 1);
        workflow_b2<policy_feeder>(feeder1, feeder1A);
    }
}

/// \brief Policy WF_1
/// \ref WF_1
TEST_F(policy, policy_workflow_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.0/16",
                g_num_policy);
    workflow_1<policy_feeder>(feeder);

    if (apulu()) {
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                    g_num_policy);
        workflow_1<policy_feeder>(feeder);
    }
}

/// \brief Policy WF_2
/// \ref WF_2
TEST_F(policy, policy_workflow_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    // setup
    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                g_num_policy);
    // trigger
    workflow_2<policy_feeder>(feeder);

    if (apulu()) {
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                    g_num_policy);
        workflow_2<policy_feeder>(feeder);
    }
}

/// \brief Policy WF_3
/// \ref WF_3
TEST_F(policy, policy_workflow_3) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    policy_feeder feeder1, feeder2, feeder3;

    // setup
    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16", 20);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16", 20);
    feeder3.init(key3, g_num_stateful_rules, IP_AF_IPV4, "12.0.0.1/16", 20);

    // trigger
    workflow_3<policy_feeder>(feeder1, feeder2, feeder3);

    if (apulu()) {
        feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64", 20);
        feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64", 20);
        feeder3.init(key3, g_num_stateful_rules, IP_AF_IPV6, "4001::1/64", 20);
        workflow_3<policy_feeder>(feeder1, feeder2, feeder3);
    }
}

/// \brief Policy WF_4
/// \ref WF_4
TEST_F(policy, policy_workflow_4) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16", 20);
    workflow_4<policy_feeder>(feeder);

    if (apulu()) {
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64", 20);
        workflow_4<policy_feeder>(feeder);
    }
}

/// \brief Policy WF_5
/// \ref WF_5
TEST_F(policy, policy_workflow_5) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    policy_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16", 20);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16", 20);
    feeder3.init(key3, g_num_stateful_rules, IP_AF_IPV4, "12.0.0.1/16", 20);
    workflow_5<policy_feeder>(feeder1, feeder2, feeder3);

    if (apulu()) {
        feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64", 20);
        feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64", 20);
        feeder3.init(key3, g_num_stateful_rules, IP_AF_IPV6, "4001::1/64", 20);
        workflow_5<policy_feeder>(feeder1, feeder2, feeder3);
    }
}

/// \brief Policy WF_6
/// \ref WF_6
TEST_F(policy, policy_workflow_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    feeder1B.init(key, g_num_stateful_rules, IP_AF_IPV4, "12.0.0.1/16",
		  g_num_policy);
    workflow_6<policy_feeder>(feeder1, feeder1A, feeder1B);

#if 0
    if (apulu()) {
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        feeder1B.init(key, g_num_stateful_rules, IP_AF_IPV6, "4001::1/64",
                      g_num_policy);
        workflow_6<policy_feeder>(feeder1, feeder1A, feeder1B);
    }
#endif
}

/// \brief Policy WF_7
/// \ref WF_7
TEST_F(policy, policy_workflow_7) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
		 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
		  g_num_policy);
    feeder1B.init(key, g_num_stateful_rules, IP_AF_IPV4, "12.0.0.1/16",
		  g_num_policy);
    workflow_7<policy_feeder>(feeder1, feeder1A, feeder1B);

#if 0
    if (apulu()) {
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        feeder1B.init(key, g_num_stateful_rules, IP_AF_IPV6, "4001::1/64",
                      g_num_policy);
        workflow_7<policy_feeder>(feeder1, feeder1A, feeder1B);
    }
#endif
}

/// \brief Policy WF_8
/// \ref WF_8
/// NOTE: this test case is incorrect, we can't update more than 1 policy table
///       in batch because of N+1 update scheme
TEST_F(policy, DISABLED_policy_workflow_8) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
                  g_num_policy);
    feeder1B.init(key, g_num_stateful_rules, IP_AF_IPV4, "12.0.0.1/16",
                  g_num_policy);
    workflow_8<policy_feeder>(feeder1, feeder1A, feeder1B);

#if 0
    if (apulu()) {
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        feeder1B.init(key, g_num_stateful_rules, IP_AF_IPV6, "4001::1/64",
                      g_num_policy);
        workflow_8<policy_feeder>(feeder1, feeder1A, feeder1B);
    }
#endif
}

/// \brief Policy WF_9
/// \ref WF_9
/// NOTE: this test case is incorrect, we can't update more than 1 policy table
///       in batch because of N+1 update scheme
TEST_F(policy, DISABLED_policy_workflow_9) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A;

    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
                  g_num_policy);
    workflow_9<policy_feeder>(feeder1, feeder1A);

#if 0
    if (apulu()) {
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        workflow_9<policy_feeder>(feeder1, feeder1A);
    }
#endif
}

/// \brief Policy WF_10
/// \ref WF_10
TEST_F(policy, DISABLED_policy_workflow_10) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    pds_obj_key_t key4 = int2pdsobjkey(100);
    policy_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;
    uint32_t num_policy = 20;

    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 num_policy);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "13.0.0.1/16",
                 num_policy);
    feeder2A.init(key2, g_num_stateful_rules, IP_AF_IPV4, "14.0.0.1/16",
                  num_policy);
    feeder3.init(key3, g_num_stateful_rules, IP_AF_IPV4, "17.0.0.1/16",
                 num_policy + 1);
    feeder3A.init(key3, g_num_stateful_rules, IP_AF_IPV4, "18.0.0.1/16",
                  num_policy + 1);
    feeder4.init(key4, g_num_stateful_rules, IP_AF_IPV4, "21.0.0.1/16",
                 num_policy);
    workflow_10<policy_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);

#if 0
    if (apulu()) {
        feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     num_policy);
        feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV6, "2101::1/64",
                     num_policy);
        feeder2A.init(key2, g_num_stateful_rules, IP_AF_IPV6, "2201::1/64",
                      num_policy);
        feeder3.init(key3, g_num_stateful_rules, IP_AF_IPV6, "2301::1/64",
                     num_policy + 1);
        feeder3A.init(key3, g_num_stateful_rules, IP_AF_IPV6, "2401::1/64",
                      num_policy + 1);
        feeder4.init(key4, g_num_stateful_rules, IP_AF_IPV6, "2501::1/64",
                     num_policy);
        workflow_10<policy_feeder>(
            feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
    }
#endif
}

/// \brief Policy WF_N_1
/// \ref WF_N_1
TEST_F(policy, policy_workflow_neg_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                g_num_policy);
    workflow_neg_1<policy_feeder>(feeder);

#if 0
    if (apulu()) {
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                    g_num_policy);
        workflow_neg_1<policy_feeder>(feeder);
    }
#endif
}

/// \brief Policy WF_N_2
/// \ref WF_N_2
/// \brief Create more than maximum number of policies supported.
/// [ Create SetMax+1] - Read
// @saratk - need to enable this after fixing the max policy value; search
// for g_max_policy assignmnet at the top. If I use PDS_MAX_SUBNET, things
// are crashing, so left it at 1024 for now.
TEST_F(policy, policy_workflow_neg_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                g_num_policy + 3);
    workflow_neg_2<policy_feeder>(feeder);

#if 0
    if (apulu()) {
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                    g_num_policy + 3);
        workflow_neg_2<policy_feeder>(feeder);
    }
#endif
}

/// \brief Policy WF_N_3
/// \ref WF_N_3
TEST_F(policy, policy_workflow_neg_3) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder;

    feeder.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                g_num_policy);
    workflow_neg_3<policy_feeder>(feeder);

#if 0
    if (apulu()) {
        feeder.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                    g_num_policy);
        workflow_neg_3<policy_feeder>(feeder);
    }
#endif
}

/// \brief Policy WF_N_4
/// \ref WF_N_4
TEST_F(policy, policy_workflow_neg_4) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    policy_feeder feeder1, feeder2;
    uint32_t num_policy = 20;

    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16", num_policy);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16", num_policy);
    workflow_neg_4<policy_feeder>(feeder1, feeder2);

#if 0
    if (apulu()) {
        feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV6,
                     "2001::1/64", num_policy);
        feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV6,
                     "3001::1/64", num_policy);
        workflow_neg_4<policy_feeder>(feeder1, feeder2);
    }
#endif
}

/// \brief Policy WF_N_5
/// \ref WF_N_5
TEST_F(policy, DISABLED_policy_workflow_neg_5) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A;

    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
                  g_num_policy);
    workflow_neg_5<policy_feeder>(feeder1, feeder1A);

#if 0
    if (apulu()) {
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        workflow_neg_5<policy_feeder>(feeder1, feeder1A);
    }
#endif
}

/// \brief Policy WF_N_6
/// \ref WF_N_6
TEST_F(policy, policy_workflow_neg_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    policy_feeder feeder1, feeder1A;

    feeder1.init(key, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 g_num_policy);
    feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
                  g_num_policy);
    workflow_neg_6<policy_feeder>(feeder1, feeder1A);

#if 0
    if (apulu()) {
        feeder1.init(key, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        workflow_neg_6<policy_feeder>(feeder1, feeder1A);
    }
#endif
}

/// \brief Policy WF_N_7
/// \ref WF_N_7
TEST_F(policy, policy_workflow_neg_7) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    policy_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 g_num_policy);
    feeder1A.init(key1, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
                  g_num_policy);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "12.0.0.1/16",
                 g_num_policy);
    workflow_neg_7<policy_feeder>(feeder1, feeder1A, feeder2);

#if 0
    if (apulu()) {
        feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV6, "2001::1/64",
                     g_num_policy);
        feeder1A.init(key1, g_num_stateful_rules, IP_AF_IPV6, "3001::1/64",
                      g_num_policy);
        feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV6, "4001::1/64",
                     g_num_policy);
        workflow_neg_7<policy_feeder>(feeder1, feeder1A, feeder2);
    }
#endif
}

/// \brief Policy WF_N_8
/// \ref WF_N_8
TEST_F(policy, policy_workflow_neg_8) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    policy_feeder feeder1, feeder2;
    uint32_t num_policy = 20;

    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 num_policy);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "11.0.0.1/16",
                 num_policy);
    workflow_neg_8<policy_feeder>(feeder1, feeder2);

#if 0
    if (apulu()) {
        feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV6,
                     "2001::1/64", num_policy);
        feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV6,
                     "3001::1/64", num_policy);
        workflow_neg_8<policy_feeder>(feeder1, feeder2);
    }
#endif
}

/// \brief Policy WF_N_9
/// \ref WF_N_9
TEST_F(policy, policy_workflow_neg_9) {
    policy_feeder feeder1, feeder2;
    uint32_t num_policy = 341;
    pds_obj_key_t key1 = int2pdsobjkey(1);
    pds_obj_key_t key2 = int2pdsobjkey(1 + num_policy);

    // setup
    feeder1.init(key1, g_num_stateful_rules, IP_AF_IPV4, "10.0.0.1/16",
                 num_policy);
    feeder2.init(key2, g_num_stateful_rules, IP_AF_IPV4, "13.0.0.1/16",
                 num_policy);

    // trigger
    // workflow_neg_8<policy_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
