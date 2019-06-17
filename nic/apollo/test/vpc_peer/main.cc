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
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/vpc_peer.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
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

        params.cfg_file = api_test::g_cfg_file;
        params.enable_fte = FALSE;
        pds_test_base::SetUpTestCase(params);

        batch_start();
        sample1_vpc_setup(PDS_VPC_TYPE_TENANT);
        batch_commit();
        sample1_vpc_setup_validate(PDS_VPC_TYPE_TENANT);
    }

    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();

        batch_start();
        sample1_vpc_teardown(PDS_VPC_TYPE_TENANT);
        batch_commit();
    }
};

//----------------------------------------------------------------------------
// VPC peer test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VPC_TEST
/// @{

/// \brief VPC WF_1
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_1) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_1<vpc_peer_feeder>(feeder);
}

/// \brief VPC WF_2
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_2) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_2<vpc_peer_feeder>(feeder);
}

/// \brief VPC WF_3
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_3) {
    vpc_peer_feeder feeder1, feeder2, feeder3;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30}, key3 = {.id = 50};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    feeder3.init(key3, vpc5, vpc6, 10);
    workflow_3<vpc_peer_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_4
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_4) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_4<vpc_peer_feeder>(feeder);
}

/// \brief VPC WF_5
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_5) {
    vpc_peer_feeder feeder1, feeder2, feeder3;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30}, key3 = {.id = 50};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc1, vpc2, 10);
    feeder3.init(key3, vpc1, vpc2, 10);
    workflow_5<vpc_peer_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_6
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_6) {
    vpc_peer_feeder feeder1, feeder1A, feeder1B;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    feeder1B.init(key, vpc5, vpc6, k_max_vpc_peer);
    workflow_6<vpc_peer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_7
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_7) {
    vpc_peer_feeder feeder1, feeder1A, feeder1B;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    feeder1B.init(key, vpc5, vpc6, k_max_vpc_peer);
    workflow_7<vpc_peer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_8
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_8) {
    vpc_peer_feeder feeder1, feeder1A, feeder1B;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    feeder1B.init(key, vpc5, vpc6, k_max_vpc_peer);
    workflow_8<vpc_peer_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_9
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_9) {
    vpc_peer_feeder feeder1, feeder1A;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    workflow_9<vpc_peer_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_10
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_10) {
    vpc_peer_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                       key4 = {.id = 100};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 21};
    pds_vpc_key_t vpc7 = {.id = 1};
    pds_vpc_key_t vpc8 = {.id = 40};
    pds_vpc_key_t vpc9 = {.id = 1};
    pds_vpc_key_t vpc10 = {.id = 41};
    pds_vpc_key_t vpc11 = {.id = 1};
    pds_vpc_key_t vpc12 = {.id = 60};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    feeder2A.init(key2, vpc5, vpc6, 10);
    feeder3.init(key3, vpc7, vpc8, 10);
    feeder3A.init(key3, vpc9, vpc10, 10);
    feeder4.init(key4, vpc11, vpc12, 10);
    workflow_10<vpc_peer_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief VPC WF_N_1
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_1) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_neg_1<vpc_peer_feeder>(feeder);
}

/// \brief VPC WF_N_2
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_2) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer+1);
    workflow_neg_2<vpc_peer_feeder>(feeder);
}

/// \brief VPC WF_N_3
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_3) {
    vpc_peer_feeder feeder;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};

    feeder.init(key, vpc1, vpc2, k_max_vpc_peer);
    workflow_neg_3<vpc_peer_feeder>(feeder);
}

/// \brief VPC WF_N_4
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_4) {
    vpc_peer_feeder feeder1, feeder2;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    workflow_neg_4<vpc_peer_feeder>(feeder1, feeder2);
}

/// \brief VPC WF_N_5
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_5) {
    vpc_peer_feeder feeder1, feeder1A;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer);
    workflow_neg_5<vpc_peer_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_6
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_6) {
    vpc_peer_feeder feeder1, feeder1A;
    pds_vpc_peer_key_t key = {.id = 1};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key, vpc1, vpc2, k_max_vpc_peer);
    feeder1A.init(key, vpc3, vpc4, k_max_vpc_peer + 1);
    workflow_neg_6<vpc_peer_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_7
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_7) {
    vpc_peer_feeder feeder1, feeder1A, feeder2;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 3};
    pds_vpc_key_t vpc5 = {.id = 1};
    pds_vpc_key_t vpc6 = {.id = 40};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder1A.init(key1, vpc3, vpc4, 10);
    feeder2.init(key2, vpc5, vpc6, 10);
    workflow_neg_7<vpc_peer_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief VPC WF_N_8
TEST_F(vpc_peer, DISABLED_vpc_peer_workflow_neg_8) {
    vpc_peer_feeder feeder1, feeder2;
    pds_vpc_peer_key_t key1 = {.id = 10}, key2 = {.id = 30};
    pds_vpc_key_t vpc1 = {.id = 1};
    pds_vpc_key_t vpc2 = {.id = 2};
    pds_vpc_key_t vpc3 = {.id = 1};
    pds_vpc_key_t vpc4 = {.id = 20};

    feeder1.init(key1, vpc1, vpc2, 10);
    feeder2.init(key2, vpc3, vpc4, 10);
    workflow_neg_8<vpc_peer_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
vpc_peer_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
}

static void
vpc_peer_test_options_parse (int argc, char **argv)
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
vpc_peer_test_options_validate (void)
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
    vpc_peer_test_options_parse(argc, argv);
    if (vpc_peer_test_options_validate() != SDK_RET_OK) {
        vpc_peer_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
