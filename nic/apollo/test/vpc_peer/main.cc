//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vpc peer test cases
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/vpc_peer.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
static std::string g_pipeline("");
static constexpr uint32_t k_max_vpc = PDS_MAX_VPC;
static constexpr uint32_t k_max_vpc_peer = PDS_MAX_VPC/2;

//----------------------------------------------------------------------------
// VPC test class
//----------------------------------------------------------------------------

class vpc_peer : public ::pds_test_base {
protected:
    vpc_peer() {}
    virtual ~vpc_peer() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;
        vpc_stepper_seed_t vpc_seed = {0};
        pds_vpc_key_t vpc_key = {.id = 1};

        params.cfg_file = api_test::g_cfg_file;
        params.pipeline = api_test::g_pipeline;
        params.enable_fte = FALSE;
        pds_test_base::SetUpTestCase(params);

        batch_start();
        VPC_SEED_INIT(&vpc_seed, vpc_key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                      k_max_vpc);
        ASSERT_TRUE(vpc_util::many_create(&vpc_seed) == sdk::SDK_RET_OK);
        batch_commit();
        ASSERT_TRUE(vpc_util::many_read(&vpc_seed) == sdk::SDK_RET_OK);
    }

    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
#if 0
        // TODO - segfault in sdk::table::sltcam::txn_start in batch_commit
        vpc_stepper_seed_t vpc_seed = {0};
        pds_vpc_key_t vpc_key = {.id = 1};

        VPC_SEED_INIT(&vpc_seed, vpc_key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                      k_max_vpc);
        ASSERT_TRUE(vpc_util::many_read(&vpc_seed) == sdk::SDK_RET_OK);
        batch_start();
        ASSERT_TRUE(vpc_util::many_delete(&vpc_seed) == sdk::SDK_RET_OK);
        batch_commit();
        ASSERT_TRUE(vpc_util::many_read(&vpc_seed,
                                        sdk::SDK_RET_ENTRY_NOT_FOUND)
                                             == sdk::SDK_RET_OK);
#endif
    }
};

//----------------------------------------------------------------------------
// VPC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VPC_TEST
/// @{

#if 0
/// \brief VPC WF_1
TEST_F(vpc_peer, vpc_workflow_1) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_1<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// \brief VPC WF_2
TEST_F(vpc_peer, vpc_workflow_2) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_2<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// \brief VPC WF_3
TEST_F(vpc_peer, vpc_workflow_3) {
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 20}, key3 = {.id = 30};
    vpc_peer_stepper_seed_t seed1, seed2, seed3;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 20};
    pds_vpc_peer_key_t vpc4 = {.id = 21};
    pds_vpc_peer_key_t vpc5 = {.id = 40};
    pds_vpc_peer_key_t vpc6 = {.id = 41};

    VPC_PEER_SEED_INIT(&seed1, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2, key2, vpc3, vpc4, 10);
    VPC_PEER_SEED_INIT(&seed3, key3, vpc5, vpc6, 10);
    workflow_3<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VPC WF_4
TEST_F(vpc_peer, vpc_workflow_4) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_4<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// \brief VPC WF_5
TEST_F(vpc_peer, vpc_workflow_5) {
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 20}, key3 = {.id = 30};
    vpc_peer_stepper_seed_t seed1, seed2, seed3;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 20};
    pds_vpc_peer_key_t vpc4 = {.id = 21};
    pds_vpc_peer_key_t vpc5 = {.id = 40};
    pds_vpc_peer_key_t vpc6 = {.id = 41};

    VPC_PEER_SEED_INIT(&seed1, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2, key2, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed3, key3, vpc1, vpc2, 10);
    workflow_5<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VPC WF_6
TEST_F(vpc_peer, vpc_workflow_6) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed1, seed1A, seed1B;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed1, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1A, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1B, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_6<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VPC WF_7
TEST_F(vpc_peer, vpc_workflow_7) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed1, seed1A, seed1B;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed1, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1A, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1B, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_7<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VPC WF_8
TEST_F(vpc_peer, DISABLED_vpc_workflow_8) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed1, seed1A, seed1B;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed1, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1A, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1B, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_8<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VPC WF_9
TEST_F(vpc_peer, vpc_workflow_9) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed1, seed1A;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed1, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1A, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_9<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VPC WF_10
TEST_F(vpc_peer, DISABLED_vpc_workflow_10) {
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                     key4 = {.id = 100};
    vpc_peer_stepper_seed_t seed1, seed2, seed3, seed4, seed2A, seed3A;

    VPC_PEER_SEED_INIT(&seed1, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2, key2, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2A, key2, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed3, key3, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed3A, key3, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed4, key4, vpc1, vpc2, 10);
    workflow_10<vpc_peer_util, vpc_peer_stepper_seed_t>(
        &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief VPC WF_N_1
TEST_F(vpc_peer, vpc_workflow_neg_1) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_neg_1<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// \brief VPC WF_N_2
TEST_F(vpc_peer, vpc_workflow_neg_2) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed, key, vpc1, vpc2, k_max_vpc_peer+1);
    workflow_neg_2<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// \brief VPC WF_N_3
TEST_F(vpc_peer, vpc_workflow_neg_3) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};

    VPC_PEER_SEED_INIT(&seed, key, vpc1, vpc2, k_max_vpc_peer);
    workflow_neg_3<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// \brief VPC WF_N_4
TEST_F(vpc_peer, vpc_workflow_neg_4) {
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_peer_stepper_seed_t seed1, seed2;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 41};
    pds_vpc_peer_key_t vpc4 = {.id = 42};

    VPC_PEER_SEED_INIT(&seed1, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2, key2, vpc3, vpc4, 10);
    workflow_neg_4<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed2);
}

/// \brief VPC WF_N_5
TEST_F(vpc_peer, DISABLED_vpc_workflow_neg_5) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed1, seed1A;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 41};
    pds_vpc_peer_key_t vpc4 = {.id = 42};

    VPC_PEER_SEED_INIT(&seed1, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1A, key, vpc3, vpc4, k_max_vpc_peer);
    workflow_neg_5<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VPC WF_N_6
TEST_F(vpc_peer, vpc_workflow_neg_6) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed1, seed1A;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 41};
    pds_vpc_peer_key_t vpc4 = {.id = 42};

    VPC_PEER_SEED_INIT(&seed1, key, vpc1, vpc2, k_max_vpc_peer);
    VPC_PEER_SEED_INIT(&seed1A, key, vpc3, vpc4, k_max_vpc_peer + 1);
    workflow_neg_6<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VPC WF_N_7
TEST_F(vpc_peer, vpc_workflow_neg_7) {
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_peer_stepper_seed_t seed1, seed1A, seed2;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 41};
    pds_vpc_peer_key_t vpc4 = {.id = 42};

    VPC_PEER_SEED_INIT(&seed1, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed1A, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2, key2, vpc3, vpc4, 10);
    workflow_neg_7<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief VPC WF_N_8
TEST_F(vpc_peer, vpc_workflow_neg_8) {
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_peer_stepper_seed_t seed1, seed2;
    pds_vpc_peer_key_t vpc1 = {.id = 1};
    pds_vpc_peer_key_t vpc2 = {.id = 2};
    pds_vpc_peer_key_t vpc3 = {.id = 41};
    pds_vpc_peer_key_t vpc4 = {.id = 42};

    VPC_PEER_SEED_INIT(&seed1, key1, vpc1, vpc2, 10);
    VPC_PEER_SEED_INIT(&seed2, key2, vpc3, vpc4, 10);
    workflow_neg_8<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed1, &seed2);
}
#endif

// ARTEMIS workflows
/// \brief VPC WF_11
TEST_F(vpc_peer, vpc_peer_workflow_11) {
    pds_vpc_peer_key_t key = {.id = 1};
    vpc_peer_stepper_seed_t seed;
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    vpc_peer_util::stepper_seed_init(&seed, key, vpc1, vpc2, 32);
    workflow_11<vpc_peer_util, vpc_peer_stepper_seed_t>(&seed);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
vpc_peer_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json> -f <apollo|artemis>" << endl;
}

static void
vpc_peer_test_options_parse (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"feature", required_argument, NULL, 'f'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    while ((oc = getopt_long(argc, argv, ":hc:f:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            break;
        case 'f':
            api_test::g_pipeline = std::string(optarg);
            break;
        default:    // ignore all other options
            break;
        }
    }
}

static inline sdk_ret_t
vpc_peer_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return SDK_RET_ERR;
    }
    if (api_test::g_pipeline != "apollo" &&
        api_test::g_pipeline != "artemis") {
        cerr << "Pipeline specified is invalid" << endl;
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

int
main (int argc, char **argv)
{
    vpc_peer_test_options_parse(argc, argv);
    if (vpc_peer_test_options_validate() != SDK_RET_OK) {
        vpc_peer_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
