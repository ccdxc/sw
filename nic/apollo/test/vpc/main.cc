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
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
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

/// \brief VPC WF_1
TEST_F(vpc, vpc_workflow_1) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_1<vpc_util, vpc_stepper_seed_t>(&seed);
}

/// \brief VPC WF_2
TEST_F(vpc, vpc_workflow_2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_2<vpc_util, vpc_stepper_seed_t>(&seed);
}

/// \brief VPC WF_3
TEST_F(vpc, vpc_workflow_3) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    vpc_stepper_seed_t seed1, seed2, seed3;

    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, "30.0.0.0/16", 20);
    VPC_SEED_INIT(&seed3, key3, PDS_VPC_TYPE_TENANT, "60.0.0.0/16", 20);
    workflow_3<vpc_util, vpc_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VPC WF_4
TEST_F(vpc, vpc_workflow_4) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_4<vpc_util, vpc_stepper_seed_t>(&seed);
}

/// \brief VPC WF_5
TEST_F(vpc, vpc_workflow_5) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    vpc_stepper_seed_t seed1, seed2, seed3;

    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16", 20);
    VPC_SEED_INIT(&seed3, key3, PDS_VPC_TYPE_TENANT, "70.0.0.0/16", 20);
    workflow_5<vpc_util, vpc_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VPC WF_6
TEST_F(vpc, vpc_workflow_6) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed1, seed1A, seed1B;

    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1A, key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1B, key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", k_max_vpc);
    workflow_6<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VPC WF_7
TEST_F(vpc, vpc_workflow_7) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed1, seed1A, seed1B;

    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1A, key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1B, key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", k_max_vpc);
    workflow_7<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VPC WF_8
TEST_F(vpc, DISABLED_vpc_workflow_8) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed1, seed1A, seed1B;

    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1A, key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1B, key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", k_max_vpc);
    workflow_8<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VPC WF_9
TEST_F(vpc, vpc_workflow_9) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed1, seed1A;

    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1A, key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    workflow_9<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VPC WF_10
TEST_F(vpc, DISABLED_vpc_workflow_10) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                     key4 = {.id = 100};
    vpc_stepper_seed_t seed1, seed2, seed3, seed4, seed2A, seed3A;

    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2A, key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", 20);
    VPC_SEED_INIT(&seed3, key3, PDS_VPC_TYPE_TENANT, "13.0.0.0/16", 20);
    VPC_SEED_INIT(&seed3A, key3, PDS_VPC_TYPE_TENANT, "14.0.0.0/16", 20);
    VPC_SEED_INIT(&seed4, key4, PDS_VPC_TYPE_TENANT, "15.0.0.0/16", 20);
    workflow_10<vpc_util, vpc_stepper_seed_t>(
        &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief VPC WF_N_1
TEST_F(vpc, vpc_workflow_neg_1) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_neg_1<vpc_util, vpc_stepper_seed_t>(&seed);
}

/// \brief VPC WF_N_2
TEST_F(vpc, vpc_workflow_neg_2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc+1);
    workflow_neg_2<vpc_util, vpc_stepper_seed_t>(&seed);
}

/// \brief VPC WF_N_3
TEST_F(vpc, vpc_workflow_neg_3) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed;

    VPC_SEED_INIT(&seed, key, PDS_VPC_TYPE_TENANT, "0.0.0.0/0", k_max_vpc);
    workflow_neg_3<vpc_util, vpc_stepper_seed_t>(&seed);
}

/// \brief VPC WF_N_4
TEST_F(vpc, vpc_workflow_neg_4) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_stepper_seed_t seed1, seed2;

    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16", 20);
    workflow_neg_4<vpc_util, vpc_stepper_seed_t>(&seed1, &seed2);
}

/// \brief VPC WF_N_5
TEST_F(vpc, DISABLED_vpc_workflow_neg_5) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed1, seed1A;

    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1A, key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    workflow_neg_5<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VPC WF_N_6
TEST_F(vpc, vpc_workflow_neg_6) {
    pds_vpc_key_t key = {.id = 1};
    vpc_stepper_seed_t seed1, seed1A;

    VPC_SEED_INIT(&seed1, key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    VPC_SEED_INIT(&seed1A, key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc + 1);
    workflow_neg_6<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VPC WF_N_7
TEST_F(vpc, vpc_workflow_neg_7) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_stepper_seed_t seed1, seed1A, seed2;

    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    VPC_SEED_INIT(&seed1A, key1, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", 20);
    workflow_neg_7<vpc_util, vpc_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief VPC WF_N_8
TEST_F(vpc, vpc_workflow_neg_8) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_stepper_seed_t seed1, seed2;

    VPC_SEED_INIT(&seed1, key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    VPC_SEED_INIT(&seed2, key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", 20);
    workflow_neg_8<vpc_util, vpc_stepper_seed_t>(&seed1, &seed2);
}

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
