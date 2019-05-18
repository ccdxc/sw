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
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
char *g_cfg_file = NULL;
constexpr uint32_t k_max_subnet = PDS_MAX_SUBNET;
constexpr pds_vpc_key_t k_vpc_key = {.id = 1};

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
        vpc_util vpc_obj(k_vpc_key.id, "10.0.0.0/8");

        batch_start();
        ASSERT_TRUE(vpc_obj.create() == sdk::SDK_RET_OK);
        batch_commit();
    }
    static void TearDownTestCase() {
        vpc_util vpc_obj(k_vpc_key.id, "10.0.0.0/8");

        batch_start();
        ASSERT_TRUE(vpc_obj.del() == sdk::SDK_RET_OK);
        batch_commit();

        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Subnet test cases implementation
//----------------------------------------------------------------------------

/// \defgroup SUBNET_TEST
/// @{

/// \brief Subnet WF_1
TEST_F(subnet, subnet_workflow_1) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed = {0};

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_1<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_2
TEST_F(subnet, subnet_workflow_2) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed = {0};

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_2<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_3
TEST_F(subnet, subnet_workflow_3) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    subnet_stepper_seed_t seed1, seed2, seed3;

    SUBNET_SEED_INIT(&seed1, key1, k_vpc_key, "10.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2, key2, k_vpc_key, "30.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed3, key3, k_vpc_key, "60.0.0.0/16", 20);
    workflow_3<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Subnet WF_4
TEST_F(subnet, subnet_workflow_4) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed = {};

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_4<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_5
TEST_F(subnet, subnet_workflow_5) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    subnet_stepper_seed_t seed1, seed2, seed3;

    SUBNET_SEED_INIT(&seed1, key1, k_vpc_key, "10.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2, key2, k_vpc_key, "30.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed3, key3, k_vpc_key, "60.0.0.0/16", 20);
    workflow_5<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Subnet WF_6
TEST_F(subnet, subnet_workflow_6) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed1, seed2, seed3;

    SUBNET_SEED_INIT(&seed1, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed2, key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed3, key, k_vpc_key, "12.0.0.0/16", k_max_subnet);
    workflow_6<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Subnet WF_7
TEST_F(subnet, subnet_workflow_7) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed1, seed2, seed3;

    SUBNET_SEED_INIT(&seed1, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed2, key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed3, key, k_vpc_key, "12.0.0.0/16", k_max_subnet);
    workflow_7<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Subnet WF_8
TEST_F(subnet, DISABLED_subnet_workflow_8) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed1, seed2, seed3;

    SUBNET_SEED_INIT(&seed1, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed2, key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed3, key, k_vpc_key, "12.0.0.0/16", k_max_subnet);
    workflow_8<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief Subnet WF_9
TEST_F(subnet, subnet_workflow_9) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed1, seed2;

    SUBNET_SEED_INIT(&seed1, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed2, key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    workflow_9<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2);
}

/// \brief Subnet WF_10
TEST_F(subnet, DISABLED_subnet_workflow_10) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                     key4 = {.id = 100};
    subnet_stepper_seed_t seed1, seed2, seed3, seed4, seed2A, seed3A;

    SUBNET_SEED_INIT(&seed1, key1, k_vpc_key, "10.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2, key2, k_vpc_key, "40.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2A, key2, k_vpc_key, "140.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed3, key3, k_vpc_key, "70.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed3A, key3, k_vpc_key, "170.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed4, key4, k_vpc_key, "100.0.0.0/16", 20);
    workflow_10<subnet_util, subnet_stepper_seed_t>(
        &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief Subnet WF_N_1
TEST_F(subnet, subnet_workflow_neg_1) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed;

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_neg_1<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_N_2
TEST_F(subnet, subnet_workflow_neg_2) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed;

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet + 1);
    workflow_neg_2<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_N_3
TEST_F(subnet, subnet_workflow_neg_3) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed;

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_neg_3<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_N_4
TEST_F(subnet, subnet_workflow_neg_4) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    subnet_stepper_seed_t seed1, seed2;

    SUBNET_SEED_INIT(&seed1, key1, k_vpc_key, "10.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2, key2, k_vpc_key, "40.0.0.0/16", 20);
    workflow_neg_4<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2);
}

/// \brief Subnet WF_N_5
TEST_F(subnet, DISABLED_subnet_workflow_neg_5) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed1, seed1A;

    SUBNET_SEED_INIT(&seed1, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed1A, key, k_vpc_key, "11.0.0.0/16", k_max_subnet);
    workflow_neg_5<subnet_util, subnet_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief Subnet WF_N_6
TEST_F(subnet, subnet_workflow_neg_6) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed;

    SUBNET_SEED_INIT(&seed, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    workflow_neg_6<subnet_util, subnet_stepper_seed_t>(&seed);
}

/// \brief Subnet WF_N_7
TEST_F(subnet, DISABLED_subnet_workflow_neg_7) {
    pds_subnet_key_t key = {.id = 1};
    subnet_stepper_seed_t seed1, seed1A;

    SUBNET_SEED_INIT(&seed1, key, k_vpc_key, "10.0.0.0/16", k_max_subnet);
    SUBNET_SEED_INIT(&seed1A, key, k_vpc_key, "11.0.0.0/16", k_max_subnet + 1);
    workflow_neg_7<subnet_util, subnet_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief Subnet WF_N_8
TEST_F(subnet, subnet_workflow_neg_8) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    subnet_stepper_seed_t seed1, seed1A, seed2;

    SUBNET_SEED_INIT(&seed1, key1, k_vpc_key, "10.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed1A, key1, k_vpc_key, "11.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2, key2, k_vpc_key, "12.0.0.0/16", 20);
    workflow_neg_8<subnet_util, subnet_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief Subnet WF_N_9
TEST_F(subnet, subnet_workflow_neg_9) {
    pds_subnet_key_t key1 = {.id = 10}, key2 = {.id = 40};
    subnet_stepper_seed_t seed1, seed2;

    SUBNET_SEED_INIT(&seed1, key1, k_vpc_key, "10.0.0.0/16", 20);
    SUBNET_SEED_INIT(&seed2, key2, k_vpc_key, "11.0.0.0/16", 20);
    workflow_neg_9<subnet_util, subnet_stepper_seed_t>(&seed1, &seed2);
}

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
