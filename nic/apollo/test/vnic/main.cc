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
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/subnet.hpp"

namespace api_test {

static const char *g_cfg_file = "hal.json";
constexpr uint32_t k_max_vnic = PDS_MAX_VNIC;
constexpr uint64_t k_seed_mac = 0xa010101000000000;
constexpr bool k_src_dst_check = TRUE;
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
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
        batch_start();
        ASSERT_TRUE(k_vpc_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(k_subnet_obj.create() == sdk::SDK_RET_OK);
        batch_commit();
    }
    static void TearDownTestCase() {
        batch_start();
        ASSERT_TRUE(k_vpc_obj.del() == sdk::SDK_RET_OK);
        ASSERT_TRUE(k_subnet_obj.del() == sdk::SDK_RET_OK);
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

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed);
    workflow_1<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_2
TEST_F(vnic_test, vnic_workflow_2) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed);
    workflow_2<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_3
TEST_F(vnic_test, vnic_workflow_3) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    VNIC_SEED_INIT(10, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed2);
    VNIC_SEED_INIT(70, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed3);
    workflow_3<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VNIC WF_4
TEST_F(vnic_test, vnic_workflow_4) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed);
    workflow_4<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_5
TEST_F(vnic_test, vnic_workflow_5) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {};

    VNIC_SEED_INIT(10, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed2);
    VNIC_SEED_INIT(70, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed3);
    workflow_5<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2, &seed3);
}

/// \brief VNIC WF_6
TEST_F(vnic_test, vnic_workflow_6) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_QINQ,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1A);
    VNIC_SEED_INIT(1, k_max_vnic, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed1B);
    workflow_6<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VNIC WF_7
TEST_F(vnic_test, vnic_workflow_7) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_QINQ,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1A);
    VNIC_SEED_INIT(1, k_max_vnic, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed1B);
    workflow_7<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VNIC WF_8
TEST_F(vnic_test, DISABLED_vnic_workflow_8) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed1B = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed1A);
    VNIC_SEED_INIT(1, k_max_vnic, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, FALSE, &seed1B);
    workflow_8<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief VNIC WF_9
TEST_F(vnic_test, vnic_workflow_9) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(1, k_max_vnic, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, k_src_dst_check, &seed1A);
    workflow_9<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VNIC WF_10
TEST_F(vnic_test, DISABLED_vnic_workflow_10) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {}, seed3 = {}, seed4 = {};
    vnic_stepper_seed_t seed2A = {}, seed3A = {};

    VNIC_SEED_INIT(10, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed2);
    VNIC_SEED_INIT(40, 20, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed2A);
    VNIC_SEED_INIT(70, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed3);
    VNIC_SEED_INIT(70, 20, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed3A);
    VNIC_SEED_INIT(100, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed4);
    workflow_10<vnic_util, vnic_stepper_seed_t>(
                &seed1, &seed2, &seed2A, &seed3, &seed3A, &seed4);
}

/// \brief VNIC WF_N_1
TEST_F(vnic_test, vnic_workflow_neg_1) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed);
    workflow_neg_1<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_N_2
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_2) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic+1, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed);
    workflow_neg_2<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_N_3
TEST_F(vnic_test, vnic_workflow_neg_3) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed);
    workflow_neg_3<vnic_util, vnic_stepper_seed_t>(&seed);
}

/// \brief VNIC WF_N_4
TEST_F(vnic_test, vnic_workflow_neg_4) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {};

    VNIC_SEED_INIT(10, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed2);
    workflow_neg_4<vnic_util, vnic_stepper_seed_t>(&seed1, &seed2);
}

/// \brief VNIC WF_N_5
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_5) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(1, k_max_vnic, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed1A);
    workflow_neg_5<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VNIC WF_N_6
TEST_F(vnic_test, vnic_workflow_neg_6) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(1, k_max_vnic+1, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed1A);
    workflow_neg_6<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief VNIC WF_N_7
TEST_F(vnic_test, vnic_workflow_neg_7) {
    vnic_stepper_seed_t seed1 = {}, seed1A = {}, seed2 = {};

    VNIC_SEED_INIT(10, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(10, 20, 0xb010101010101010, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed1A);
    VNIC_SEED_INIT(40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, k_src_dst_check, &seed2);
    workflow_neg_7<vnic_util, vnic_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief VNIC WF_N_8
TEST_F(vnic_test, vnic_workflow_neg_8) {
    vnic_stepper_seed_t seed1 = {}, seed2 = {};

    VNIC_SEED_INIT(10, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_MPLSoUDP, k_src_dst_check, &seed1);
    VNIC_SEED_INIT(40, 20, k_seed_mac, PDS_ENCAP_TYPE_DOT1Q,
                   PDS_ENCAP_TYPE_VXLAN, FALSE, &seed2);
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
    fprintf(stdout, "Usage : %s -c <hal.json> \n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, "hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            api_test::g_cfg_file = optarg;
            if (!api_test::g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
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
