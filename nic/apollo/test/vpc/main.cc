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
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"
#include "nic/apollo/test/utils/vpc.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
static std::string g_pipeline("");
static constexpr uint32_t k_max_vpc = PDS_MAX_VPC + 1;

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

        params.cfg_file = api_test::g_cfg_file;
        params.pipeline = api_test::g_pipeline;
        params.enable_fte = FALSE;
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
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_1<vpc_feeder>(feeder);
}

/// \brief VPC WF_2
TEST_F(vpc, vpc_workflow_2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_2<vpc_feeder>(feeder);
}

/// \brief VPC WF_3
TEST_F(vpc, vpc_workflow_3) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    vpc_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "30.0.0.0/16", 20);
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "60.0.0.0/16", 20);
    workflow_3<vpc_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_4
TEST_F(vpc, vpc_workflow_4) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_4<vpc_feeder>(feeder);
}

/// \brief VPC WF_5
TEST_F(vpc, vpc_workflow_5) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70};
    vpc_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16", 20);
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "70.0.0.0/16", 20);
    workflow_5<vpc_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_6
TEST_F(vpc, vpc_workflow_6) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", k_max_vpc);
    workflow_6<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_7
TEST_F(vpc, vpc_workflow_7) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", k_max_vpc);
    workflow_7<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_8
TEST_F(vpc, DISABLED_vpc_workflow_8) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", k_max_vpc);
    workflow_8<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_9
TEST_F(vpc, vpc_workflow_9) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    workflow_9<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_10
TEST_F(vpc, DISABLED_vpc_workflow_10) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40}, key3 = {.id = 70},
                  key4 = {.id = 100};
    vpc_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", 20);
    feeder2A.init(key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", 20);
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "13.0.0.0/16", 20);
    feeder3A.init(key3, PDS_VPC_TYPE_TENANT, "14.0.0.0/16", 20);
    feeder4.init(key4, PDS_VPC_TYPE_TENANT, "15.0.0.0/16", 20);
    workflow_10<vpc_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief VPC WF_N_1
TEST_F(vpc, vpc_workflow_neg_1) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_neg_1<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_2
TEST_F(vpc, vpc_workflow_neg_2) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc+1);
    workflow_neg_2<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_3
TEST_F(vpc, vpc_workflow_neg_3) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "0.0.0.0/0", k_max_vpc);
    workflow_neg_3<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_4
TEST_F(vpc, vpc_workflow_neg_4) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_feeder feeder1, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16", 20);
    workflow_neg_4<vpc_feeder>(feeder1, feeder2);
}

/// \brief VPC WF_N_5
TEST_F(vpc, DISABLED_vpc_workflow_neg_5) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    feeder1A.init( key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc);
    workflow_neg_5<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_6
TEST_F(vpc, vpc_workflow_neg_6) {
    pds_vpc_key_t key = {.id = 1};
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", k_max_vpc + 1);
    workflow_neg_6<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_7
TEST_F(vpc, vpc_workflow_neg_7) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    feeder1A.init(key1, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16", 20);
    workflow_neg_7<vpc_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief VPC WF_N_8
TEST_F(vpc, vpc_workflow_neg_8) {
    pds_vpc_key_t key1 = {.id = 10}, key2 = {.id = 40};
    vpc_feeder feeder1, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 20);
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16", 20);
    workflow_neg_8<vpc_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
vpc_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json> -f <apollo|artemis>" << endl;
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
    api_test::g_pipeline = api_test::pipeline_get();
    if (!IS_APOLLO() && !IS_ARTEMIS()) {
        cerr << "Pipeline specified is invalid" << endl;
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
