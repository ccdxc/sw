//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all nexthop test cases
///
//----------------------------------------------------------------------------

#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/nh.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
std::string g_pipeline("");
static const std::string k_base_nh_ip = "50.50.1.1";
static constexpr uint64_t k_base_mac = 0x0E0D0A0B0200;
static constexpr uint32_t k_max_nh = PDS_MAX_NEXTHOP;

//----------------------------------------------------------------------------
// NH test class
//----------------------------------------------------------------------------

class nh_test : public pds_test_base {
protected:
    nh_test() {}
    virtual ~nh_test() {}
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
// NH test cases implementation
//----------------------------------------------------------------------------

/// \defgroup NH
/// @{

/// \brief NH WF_1
TEST_F(nh_test, nh_workflow1) {
    nh_stepper_seed_t seed = {};

    NH_SEED_INIT(&seed, k_base_nh_ip);
    workflow_1<nh_util, nh_stepper_seed_t>(&seed);
}

/// \brief NH WF_2
TEST_F(nh_test, nh_workflow2) {
    nh_stepper_seed_t seed = {};

    NH_SEED_INIT(&seed, k_base_nh_ip);
    workflow_2<nh_util, nh_stepper_seed_t>(&seed);
}

/// \brief NH WF_3
TEST_F(nh_test, nh_workflow3) {
    nh_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    NH_SEED_INIT(&seed1, "10.10.1.1", 0x0E010B0A1000, 10, 100);
    NH_SEED_INIT(&seed2, "20.20.1.1", 0x0E010B0A2000, 20, 200);
    NH_SEED_INIT(&seed3, "30.30.1.1", 0x0E010B0A3000, 30, 300);
    workflow_3<nh_util, nh_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief NH WF_4
TEST_F(nh_test, nh_workflow4) {
    nh_stepper_seed_t seed = {};

    NH_SEED_INIT(&seed, k_base_nh_ip);
    workflow_4<nh_util, nh_stepper_seed_t>(&seed);
}

/// \brief NH WF_5
TEST_F(nh_test, nh_workflow5) {
    nh_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    NH_SEED_INIT(&seed1, "10.10.1.1", 0x0E010B0A1000, 10, 100);
    NH_SEED_INIT(&seed2, "20.20.1.1", 0x0E010B0A2000, 20, 200);
    NH_SEED_INIT(&seed3, "30.30.1.1", 0x0E010B0A3000, 30, 300);
    workflow_5<nh_util, nh_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief NH WF_6
TEST_F(nh_test, nh_workflow6) {
    nh_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip);
    // seed1A =  seed1 + different mac
    NH_SEED_INIT(&seed1A, k_base_nh_ip, 0x0E010B0A3000);
    // seed1B =  seed1A + different mac & ip
    NH_SEED_INIT(&seed1B, "30.30.1.1", 0x0E010B0A2000);
    workflow_6<nh_util, nh_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief NH WF_7
TEST_F(nh_test, nh_workflow7) {
    nh_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip);
    // seed1A =  seed1 + different mac
    NH_SEED_INIT(&seed1A, k_base_nh_ip, 0x0E010B0A3000);
    // seed1B =  seed1A + different mac & ip
    NH_SEED_INIT(&seed1B, "30.30.1.1", 0x0E010B0A2000);
    workflow_7<nh_util, nh_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief NH WF_8
TEST_F(nh_test, DISABLED_nh_workflow8) {
    nh_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip);
    // seed1A =  seed1 + different mac
    NH_SEED_INIT(&seed1A, k_base_nh_ip, 0x0E010B0A3000);
    // seed1B =  seed1A + different mac & ip
    NH_SEED_INIT(&seed1B, "30.30.1.1", 0x0E010B0A2000);
    workflow_8<nh_util, nh_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief NH WF_9
TEST_F(nh_test, nh_workflow9) {
    nh_stepper_seed_t seed1 = {}, seed1A = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip);
    // seed1B =  seed1A + different mac & ip
    NH_SEED_INIT(&seed1A, "30.30.1.1", 0x0E010B0A2000);
    workflow_9<nh_util, nh_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief NH WF_10
TEST_F(nh_test, DISABLED_nh_workflow10) {
    nh_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {}, seed4 = {};
    nh_stepper_seed_t seed2A = {}, seed3A = {};

    NH_SEED_INIT(&seed1, "10.10.1.1", 0x0E010B0A1000, 10, 100);
    NH_SEED_INIT(&seed2, "20.200.1.1", 0x0E010B0A2000, 20, 200);
    // seed2A =  seed2 + different mac & ip
    NH_SEED_INIT(&seed2A, "20.20.1.1", 0x0E010B0A2222, 20, 200);
    NH_SEED_INIT(&seed3, "30.300.1.1", 0x0E010B0A3000, 30, 300);
    // seed3A =  seed3 + different mac & ip
    NH_SEED_INIT(&seed3A, "30.30.1.1", 0x0E010B0A3333, 30, 300);
    NH_SEED_INIT(&seed4, "40.40.1.1", 0x0E010B0A4000, 40, 400);
    workflow_10<nh_util, nh_stepper_seed_t>(
                &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief NH WF_N_1
TEST_F(nh_test, nh_workflow_neg_1) {
    nh_stepper_seed_t seed = {};

    NH_SEED_INIT(&seed, k_base_nh_ip);
    workflow_neg_1<nh_util, nh_stepper_seed_t>(&seed);
}

/// \brief NH WF_N_2
TEST_F(nh_test, DISABLED_nh_workflow_neg_2) {
    nh_stepper_seed_t seed = {};

    // TODO: PDS_MAX_NEXTHOP must be 1048576
    NH_SEED_INIT(&seed, k_base_nh_ip, k_base_mac, k_max_nh);
    workflow_neg_2<nh_util, nh_stepper_seed_t>(&seed);
}

/// \brief NH WF_N_3
TEST_F(nh_test, nh_workflow_neg_3) {
    nh_stepper_seed_t seed = {};

    NH_SEED_INIT(&seed, "150.150.1.1");
    workflow_neg_3<nh_util, nh_stepper_seed_t>(&seed);
}

/// \brief NH WF_N_4
TEST_F(nh_test, nh_workflow_neg_4) {
    nh_stepper_seed_t seed1 = {}, seed2 = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip, k_base_mac, 10, 100);
    NH_SEED_INIT(&seed2, "60.60.1.1",  0x0E010B0A2000, 10, 200);
    workflow_neg_4<nh_util, nh_stepper_seed_t>(&seed1, &seed2);
}

/// \brief NH WF_N_5
TEST_F(nh_test, DISABLED_nh_workflow_neg_5) {
    nh_stepper_seed_t seed1 = {}, seed1A = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip);
    // seed1A = seed1 + different IP & MAC
    NH_SEED_INIT(&seed1A, "150.150.1.1", 0x0E010B0A2000);
    workflow_neg_5<nh_util, nh_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief NH WF_N_6
TEST_F(nh_test, DISABLED_nh_workflow_neg_6) {
    nh_stepper_seed_t seed1 = {}, seed1A = {};

    NH_SEED_INIT(&seed1, k_base_nh_ip);
    // seed1A = seed1 + different IP & MAC
    NH_SEED_INIT(&seed1A, "150.150.1.1", 0x0E010B0A2000, k_max_nh+1);
    workflow_neg_6<nh_util, nh_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief NH WF_N_7
TEST_F(nh_test, nh_workflow_neg_7) {
    nh_stepper_seed_t seed1 = {}, seed1A = {}, seed2 = {};

    NH_SEED_INIT(&seed1, "10.10.1.1", 0x0E010B0A1000, 10, 100);
    NH_SEED_INIT(&seed1A, "10.10.1.1", 0x0E010B0A1111, 10, 100);
    NH_SEED_INIT(&seed2, "20.20.1.1", 0x0E010B0A2000, 20, 200);
    workflow_neg_7<nh_util, nh_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief NH WF_N_8
TEST_F(nh_test, nh_workflow_neg_8) {
    nh_stepper_seed_t seed1 = {}, seed2 = {};

    NH_SEED_INIT(&seed1, "10.10.1.1", 0x0E010B0A1000, 10, 100);
    NH_SEED_INIT(&seed2, "20.20.1.1", 0x0E010B0A2000, 20, 200);
    workflow_neg_8<nh_util, nh_stepper_seed_t>(&seed1, &seed2);
}

/// @}

}    // namespace api_test

static inline void
nh_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json> -f <artemis>" << endl;
    return;
}

static inline sdk_ret_t
nh_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    if (!IS_ARTEMIS()) {
        cerr << "Pipeline specified is invalid" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
nh_test_options_parse (int argc, char **argv)
{
    int oc = -1;
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
    return;
}

//----------------------------------------------------------------------------
//// Entry point
////----------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    nh_test_options_parse(argc, argv);
    if (nh_test_options_validate() != sdk::SDK_RET_OK) {
        nh_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}