//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all tep test cases
///
//----------------------------------------------------------------------------

#include <getopt.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/utils.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
static std::string g_pipeline("");
static const char * const k_base_nh_ip = "50.50.1.1";
static const std::string k_base_dipi = "50::50:1:1";
static constexpr int k_max_tep = PDS_MAX_TEP;
static constexpr pds_encap_t k_mplsoudp_encap = {PDS_ENCAP_TYPE_MPLSoUDP, 11};
static constexpr pds_encap_t k_vxlan_encap = {PDS_ENCAP_TYPE_VXLAN, 22};
static constexpr bool k_nat = FALSE;

//----------------------------------------------------------------------------
// TEP test class
//----------------------------------------------------------------------------

class tep_test : public pds_test_base {
protected:
    tep_test() {}
    virtual ~tep_test() {}
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
// TEP test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP
/// @{

/// \brief TEP WF_1
TEST_F(tep_test, tep_workflow1) {
    tep_stepper_seed_t seed = {};

    if (IS_ARTEMIS())
        TEP_SEED_INIT(&seed, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else
        TEP_SEED_INIT(&seed, k_base_nh_ip);
    workflow_1<tep_util, tep_stepper_seed_t>(&seed);
}

/// \brief TEP WF_2
TEST_F(tep_test, tep_workflow2) {
    tep_stepper_seed_t seed = {};

    if (IS_ARTEMIS())
        TEP_SEED_INIT(&seed, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else
        TEP_SEED_INIT(&seed, k_base_nh_ip);
    workflow_2<tep_util, tep_stepper_seed_t>(&seed);
}

/// \brief TEP WF_3
TEST_F(tep_test, tep_workflow3) {
    tep_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        TEP_SEED_INIT(&seed3, "30.30.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20);
        TEP_SEED_INIT(&seed3, "30.30.1.1", 30);
    }
    workflow_3<tep_util, tep_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief TEP WF_4
TEST_F(tep_test, tep_workflow4) {
    tep_stepper_seed_t seed = {};

    if (IS_ARTEMIS())
        TEP_SEED_INIT(&seed, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else
        TEP_SEED_INIT(&seed, k_base_nh_ip);
    workflow_4<tep_util, tep_stepper_seed_t>(&seed);
}

/// \brief TEP WF_5
TEST_F(tep_test, tep_workflow5) {
    tep_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        TEP_SEED_INIT(&seed3, "30.30.1.1", 30, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20);
        TEP_SEED_INIT(&seed3, "30.30.1.1", 30);
    }
    workflow_5<tep_util, tep_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief TEP WF_6
TEST_F(tep_test, tep_workflow6) {
    tep_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // seed1A =  seed1 + different dipi
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        // seed1B =  seed1A + different dipi
        TEP_SEED_INIT(&seed1B, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip);
        // seed1A =  seed1 + different encap
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_vxlan_encap);
        // seed1B =  seed1A + different tunnel type, encap, nat
        TEP_SEED_INIT(&seed1B, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_6<tep_util, tep_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief TEP WF_7
TEST_F(tep_test, tep_workflow7) {
    tep_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // seed1A =  seed1 + different dipi
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        // seed1B =  seed1A + different dipi
        TEP_SEED_INIT(&seed1B, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip);
        // seed1A =  seed1 + different encap
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_vxlan_encap);
        // seed1B =  seed1A + different tunnel type, encap, nat
        TEP_SEED_INIT(&seed1B, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_7<tep_util, tep_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief TEP WF_8
TEST_F(tep_test, DISABLED_tep_workflow8) {
    tep_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // seed1A =  seed1 + different dipi
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        // seed1B =  seed1A + different dipi
        TEP_SEED_INIT(&seed1B, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip);
        // seed1A =  seed1 + different encap
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_vxlan_encap);
        // seed1B =  seed1A + different tunnel type, encap, nat
        TEP_SEED_INIT(&seed1B, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_8<tep_util, tep_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief TEP WF_9
TEST_F(tep_test, tep_workflow9) {
    tep_stepper_seed_t seed1 = {}, seed1A = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // seed1A =  seed1 + different dipi
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip);
        // seed1A =  seed1 + different tunnel type, encap, nat
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_vxlan_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_9<tep_util, tep_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief TEP WF_10
TEST_F(tep_test, DISABLED_tep_workflow10) {
    tep_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {}, seed4 = {};
    tep_stepper_seed_t seed2A = {}, seed3A = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        TEP_SEED_INIT(&seed2A, "20.20.1.1", 20, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:20:21", 0);
        TEP_SEED_INIT(&seed3, "30.30.1.1", 30, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
        TEP_SEED_INIT(&seed3A, "30.30.1.1", 30, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:30:31", 0);
        TEP_SEED_INIT(&seed4, "40.40.1.1", 40, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "40::40:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20);
        TEP_SEED_INIT(&seed2A, "20.20.1.1", 20, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
        TEP_SEED_INIT(&seed3, "30.30.1.1", 30);
        TEP_SEED_INIT(&seed3A, "30.30.1.1", 30, k_mplsoudp_encap, k_nat);
        TEP_SEED_INIT(&seed4, "40.40.1.1", 40);
    }
    workflow_10<tep_util, tep_stepper_seed_t>(
                &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief TEP WF_N_1
TEST_F(tep_test, tep_workflow_neg_1) {
    tep_stepper_seed_t seed = {};

    if (IS_ARTEMIS())
        TEP_SEED_INIT(&seed, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else
        TEP_SEED_INIT(&seed, k_base_nh_ip);
    workflow_neg_1<tep_util, tep_stepper_seed_t>(&seed);
}

/// \brief TEP WF_N_2
TEST_F(tep_test, tep_workflow_neg_2) {
    tep_stepper_seed_t seed = {};

    if (IS_ARTEMIS())
        TEP_SEED_INIT(&seed, k_base_nh_ip, k_max_tep+2, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else
        TEP_SEED_INIT(&seed, k_base_nh_ip, k_max_tep+2);
    workflow_neg_2<tep_util, tep_stepper_seed_t>(&seed);
}

/// \brief TEP WF_N_3
TEST_F(tep_test, tep_workflow_neg_3) {
    tep_stepper_seed_t seed = {};

    if (IS_ARTEMIS())
        TEP_SEED_INIT(&seed, "150.150.1.1", k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "150::150:1:1", 0);
    else
        TEP_SEED_INIT(&seed, "150.150.1.1");
    workflow_neg_3<tep_util, tep_stepper_seed_t>(&seed);
}

/// \brief TEP WF_N_4
TEST_F(tep_test, tep_workflow_neg_4) {
    tep_stepper_seed_t seed1 = {}, seed2 = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        TEP_SEED_INIT(&seed2, "60.60.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, 10);
        TEP_SEED_INIT(&seed2, "60.60.1.1", 10);
    }
    workflow_neg_4<tep_util, tep_stepper_seed_t>(&seed1, &seed2);
}

/// \brief TEP WF_N_5
TEST_F(tep_test, DISABLED_tep_workflow_neg_5) {
    tep_stepper_seed_t seed1 = {}, seed1A = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip);
        // seed1A = seed + different tunnel type, nat
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_neg_5<tep_util, tep_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief TEP WF_N_6
TEST_F(tep_test, tep_workflow_neg_6) {
    tep_stepper_seed_t seed1 = {}, seed1A = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep+1, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, k_base_nh_ip);
        // seed1A = seed1 + different tunnel type, nat
        TEP_SEED_INIT(&seed1A, k_base_nh_ip, k_max_tep+1, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_neg_6<tep_util, tep_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief TEP WF_N_7
TEST_F(tep_test, tep_workflow_neg_7) {
    tep_stepper_seed_t seed1 = {}, seed1A = {}, seed2 = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        TEP_SEED_INIT(&seed1A, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
        TEP_SEED_INIT(&seed1A, "10.10.1.1", 10);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20);
    }
    workflow_neg_7<tep_util, tep_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief TEP WF_N_8
TEST_F(tep_test, tep_workflow_neg_8) {
    tep_stepper_seed_t seed1 = {}, seed2 = {};

    if (IS_ARTEMIS()) {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
    } else {
        TEP_SEED_INIT(&seed1, "10.10.1.1", 10, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
        TEP_SEED_INIT(&seed2, "20.20.1.1", 20);
    }
    workflow_neg_8<tep_util, tep_stepper_seed_t>(&seed1, &seed2);
}

/// @}

}    // namespace api_test

static inline void
tep_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json> -f <apollo|artemis>" << endl;
    return;
}

static inline sdk_ret_t
tep_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    if (!IS_APOLLO() && !IS_ARTEMIS()) {
        cerr << "Pipeline specified is invalid" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
tep_test_options_parse (int argc, char **argv)
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
    tep_test_options_parse(argc, argv);
    if (tep_test_options_validate() != sdk::SDK_RET_OK) {
        tep_test_usage_print(argv);
        exit(1);
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
