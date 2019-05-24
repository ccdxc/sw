//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all vnic test cases
///
//----------------------------------------------------------------------------
#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

namespace api_test {

static const char *g_cfg_file = "hal.json";
static std::string g_pipeline("apollo");
static constexpr uint32_t k_max_vnic = PDS_MAX_VNIC;
static const vpc_util k_vpc_obj(1, "10.0.0.0/8");
static const subnet_util k_subnet_obj(1, 1, "10.1.0.0/16");

//----------------------------------------------------------------------------
// VNIC test class
//----------------------------------------------------------------------------

class vnic_test : public pds_test_base {
protected:
    vnic_test() {}
    virtual ~vnic_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;

        params.cfg_file = api_test::g_cfg_file;
        params.pipeline = api_test::g_pipeline;
        params.enable_fte = FALSE;
        pds_test_base::SetUpTestCase(params);
        batch_start();
        VPC_CREATE(k_vpc_obj);
        SUBNET_CREATE(k_subnet_obj);
        batch_commit();
    }
    static void TearDownTestCase() {
        batch_start();
        VPC_DELETE(k_vpc_obj);
        SUBNET_DELETE(k_subnet_obj);
        batch_commit();
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

/// \brief VNIC WF_1
TEST_F(vnic_test, vnic_workflow_1) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(&seed, 1);
    workflow_1<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_2
TEST_F(vnic_test, vnic_workflow_2) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(&seed, 1);
    workflow_2<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_3
TEST_F(vnic_test, vnic_workflow_3) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    VNIC_SEED_INIT(&seed1, 10, 20);
    VNIC_SEED_INIT(&seed2, 40, 20);
    VNIC_SEED_INIT(&seed3, 70, 20);
    workflow_3<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VNIC WF_4
TEST_F(vnic_test, vnic_workflow_4) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(&seed, 1);
    workflow_4<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_5
TEST_F(vnic_test, vnic_workflow_5) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    VNIC_SEED_INIT(&seed1, 10, 20);
    VNIC_SEED_INIT(&seed2, 40, 20);
    VNIC_SEED_INIT(&seed3, 70, 20);
    workflow_5<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VNIC WF_6
TEST_F(vnic_test, vnic_workflow_6) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    VNIC_SEED_INIT(&seed1, 1);
    VNIC_SEED_INIT(&seed1A, 1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_QINQ);
    VNIC_SEED_INIT(&seed1B, 1, k_max_vnic, 0xb010101010101010,
                   PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN, FALSE);
    workflow_6<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VNIC WF_7
TEST_F(vnic_test, vnic_workflow_7) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    VNIC_SEED_INIT(&seed1, 1);
    VNIC_SEED_INIT(&seed1A, 1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_QINQ);
    VNIC_SEED_INIT(&seed1B, 1, k_max_vnic, 0xb010101010101010,
                   PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN, FALSE);
    workflow_7<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VNIC WF_8
TEST_F(vnic_test, DISABLED_vnic_workflow_8) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    VNIC_SEED_INIT(&seed1, 1);
    VNIC_SEED_INIT(&seed1A, 1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE);
    VNIC_SEED_INIT(&seed1B, 1, k_max_vnic, 0xb010101010101010,
                   PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP, FALSE);
    workflow_8<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VNIC WF_9
TEST_F(vnic_test, vnic_workflow_9) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {};

    VNIC_SEED_INIT(&seed1, 1);
    VNIC_SEED_INIT(&seed1A, 1, k_max_vnic, 0xb010101010101010,
                   PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    workflow_9<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VNIC WF_10
TEST_F(vnic_test, DISABLED_vnic_workflow_10) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {}, seed4 = {};
    vnic_stepper_seed_t seed2A = {}, seed3A = {};

    VNIC_SEED_INIT(&seed1, 10, 20);
    VNIC_SEED_INIT(&seed2, 40, 20);
    VNIC_SEED_INIT(&seed2A, 40, 20, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE);
    VNIC_SEED_INIT(&seed3, 70, 20);
    VNIC_SEED_INIT(&seed3A, 70, 20, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE);
    VNIC_SEED_INIT(&seed4, 100, 20);
    workflow_10<vnic_util, vnic_stepper_seed_t>(
                &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief VNIC WF_N_1
TEST_F(vnic_test, vnic_workflow_neg_1) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(&seed, 1);
    workflow_neg_1<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_N_2
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_2) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(&seed, 1, k_max_vnic+1);
    workflow_neg_2<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_N_3
TEST_F(vnic_test, vnic_workflow_neg_3) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(&seed, 1);
    workflow_neg_3<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_N_4
TEST_F(vnic_test, vnic_workflow_neg_4) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {};

    VNIC_SEED_INIT(&seed1, 10, 20);
    VNIC_SEED_INIT(&seed2, 40, 20);
    workflow_neg_4<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2);
}

/// \brief VNIC WF_N_5
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_5) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {};

    VNIC_SEED_INIT(&seed1, 1);
    VNIC_SEED_INIT(&seed1A, 1, k_max_vnic, 0xb010101010101010,
                   PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN, FALSE);
    workflow_neg_5<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VNIC WF_N_6
TEST_F(vnic_test, vnic_workflow_neg_6) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {};

    VNIC_SEED_INIT(&seed1, 1);
    VNIC_SEED_INIT(&seed1A, 1, k_max_vnic+1, 0xb010101010101010,
                   PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN, FALSE);
    workflow_neg_6<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VNIC WF_N_7
TEST_F(vnic_test, vnic_workflow_neg_7) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed2 = {};

    VNIC_SEED_INIT(&seed1, 10, 20);
    VNIC_SEED_INIT(&seed1A, 10, 20, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE);
    VNIC_SEED_INIT(&seed2, 40, 20);
    workflow_neg_7<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief VNIC WF_N_8
TEST_F(vnic_test, vnic_workflow_neg_8) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {};

    VNIC_SEED_INIT(&seed1, 10, 20);
    VNIC_SEED_INIT(&seed2, 40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE);
    workflow_neg_8<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2);
}

/// @}
}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json> -f <apollo|artemis>\n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"feature", required_argument, NULL, 'f'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, "hc:f:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            if (!api_test::g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'f':
            api_test::g_pipeline = std::string(optarg);
            if (api_test::g_pipeline != "apollo" &&
                api_test::g_pipeline != "artemis") {
                fprintf(stderr, "Pipeline specified is invalid\n");
                print_usage(argv);
                exit(1);
            }
            break;

        default:
            // ignore all other options
            break;
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
