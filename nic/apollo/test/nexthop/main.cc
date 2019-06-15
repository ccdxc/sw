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
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

using std::cerr;
using std::cout;
using std::endl;

namespace api_test {

// Globals
static const char *g_cfg_file = NULL;
static const std::string k_base_nh_ip = "50.50.1.1";
static constexpr uint64_t k_base_mac = 0x0E0D0A0B0200;
static constexpr uint32_t k_max_nh = PDS_MAX_NEXTHOP;

//----------------------------------------------------------------------------
// NH test class
//----------------------------------------------------------------------------

class nh_test : public ::pds_test_base {
protected:
    nh_test() {}
    virtual ~nh_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;

        params.cfg_file = api_test::g_cfg_file;
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
TEST_F(nh_test, nh_workflow_1) {
    nexthop_feeder feeder;

    feeder.init(k_base_nh_ip);
    workflow_1<nexthop_feeder>(feeder);
}

/// \brief NH WF_2
TEST_F(nh_test, nh_workflow_2) {
    nexthop_feeder feeder;

    feeder.init(k_base_nh_ip);
    workflow_2<nexthop_feeder>(feeder);
}

/// \brief NH WF_3
TEST_F(nh_test, nh_workflow_3) {
    nexthop_feeder feeder1, feeder2, feeder3;

    feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
    feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
    feeder3.init("30.30.1.1", 0x0E010B0A3000, 30, 300);
    workflow_3<nexthop_feeder>(feeder1, feeder2, feeder3);
}

/// \brief NH WF_4
TEST_F(nh_test, nh_workflow_4) {
    nexthop_feeder feeder;

    feeder.init(k_base_nh_ip);
    workflow_4<nexthop_feeder>(feeder);
}

/// \brief NH WF_5
TEST_F(nh_test, nh_workflow_5) {
    nexthop_feeder feeder1, feeder2, feeder3;

    feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
    feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
    feeder3.init("30.30.1.1", 0x0E010B0A3000, 30, 300);
    workflow_5<nexthop_feeder>(feeder1, feeder2, feeder3);
}

/// \brief NH WF_6
TEST_F(nh_test, nh_workflow_6) {
    nexthop_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_nh_ip);
    // feeder1A =  feeder1 + different mac
    feeder1A.init(k_base_nh_ip, 0x0E010B0A3000);
    // feeder1B =  feeder1A + different mac & ip
    feeder1B.init("30.30.1.1", 0x0E010B0A2000);
    workflow_6<nexthop_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief NH WF_7
TEST_F(nh_test, nh_workflow_7) {
    nexthop_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_nh_ip);
    // feeder1A =  feeder1 + different mac
    feeder1A.init(k_base_nh_ip, 0x0E010B0A3000);
    // feeder1B =  feeder1A + different mac & ip
    feeder1B.init("30.30.1.1", 0x0E010B0A2000);
    workflow_7<nexthop_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief NH WF_8
TEST_F(nh_test, DISABLED_nh_workflow_8) {
    nexthop_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(k_base_nh_ip);
    // feeder1A =  feeder1 + different mac
    feeder1A.init(k_base_nh_ip, 0x0E010B0A3000);
    // feeder1B =  feeder1A + different mac & ip
    feeder1B.init("30.30.1.1", 0x0E010B0A2000);
    workflow_8<nexthop_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief NH WF_9
TEST_F(nh_test, nh_workflow_9) {
    nexthop_feeder feeder1, feeder1A;

    feeder1.init(k_base_nh_ip);
    // feeder1A =  feeder1 + different mac & ip
    feeder1A.init("30.30.1.1", 0x0E010B0A2000);
    workflow_9<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_10
TEST_F(nh_test, DISABLED_nh_workflow_10) {
    nexthop_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
    feeder2.init("20.200.1.1", 0x0E010B0A2000, 20, 200);
    // feeder2A =  feeder2 + different mac & ip
    feeder2A.init("20.20.1.1", 0x0E010B0A2222, 20, 200);
    feeder3.init("30.300.1.1", 0x0E010B0A3000, 30, 300);
    // feeder3A =  feeder3 + different mac & ip
    feeder3A.init("30.30.1.1", 0x0E010B0A3333, 30, 300);
    feeder4.init("40.40.1.1", 0x0E010B0A4000, 40, 400);
    workflow_10<nexthop_feeder>(feeder1, feeder2, feeder2A,
                           feeder3, feeder3A, feeder4);
}

/// \brief NH WF_N_1
TEST_F(nh_test, nh_workflow_neg_1) {
    nexthop_feeder feeder;

    feeder.init(k_base_nh_ip);
    workflow_neg_1<nexthop_feeder>(feeder);
}

/// \brief NH WF_N_2
TEST_F(nh_test, DISABLED_nh_workflow_neg_2) {
    nexthop_feeder feeder;

    // TODO: PDS_MAX_NEXTHOP must be 1048576
    feeder.init(k_base_nh_ip, k_base_mac, k_max_nh);
    workflow_neg_2<nexthop_feeder>(feeder);
}

/// \brief NH WF_N_3
TEST_F(nh_test, nh_workflow_neg_3) {
    nexthop_feeder feeder;

    feeder.init("150.150.1.1");
    workflow_neg_3<nexthop_feeder>(feeder);
}

/// \brief NH WF_N_4
TEST_F(nh_test, nh_workflow_neg_4) {
    nexthop_feeder feeder1, feeder2;

    feeder1.init(k_base_nh_ip, k_base_mac, 10, 100);
    feeder2.init("60.60.1.1",  0x0E010B0A2000, 10, 200);
    workflow_neg_4<nexthop_feeder>(feeder1, feeder2);
}

/// \brief NH WF_N_5
TEST_F(nh_test, DISABLED_nh_workflow_neg_5) {
    nexthop_feeder feeder1, feeder1A;

    feeder1.init(k_base_nh_ip);
    // seed1A = seed1 + different IP & MAC
    feeder1A.init("150.150.1.1", 0x0E010B0A2000);
    workflow_neg_5<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_N_6
TEST_F(nh_test, DISABLED_nh_workflow_neg_6) {
    nexthop_feeder feeder1, feeder1A;

    feeder1.init(k_base_nh_ip);
    // seed1A = seed1 + different IP & MAC
    feeder1A.init("150.150.1.1", 0x0E010B0A2000, k_max_nh+1);
    workflow_neg_6<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_N_7
TEST_F(nh_test, nh_workflow_neg_7) {
    nexthop_feeder feeder1, feeder1A, feeder2;

    feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
    feeder1A.init("10.10.1.1", 0x0E010B0A1111, 10, 100);
    feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
    workflow_neg_7<nexthop_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief NH WF_N_8
TEST_F(nh_test, nh_workflow_neg_8) {
    nexthop_feeder feeder1, feeder2;

    feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
    feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
    workflow_neg_8<nexthop_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

static inline void
nh_test_usage_print (char **argv)
{
    cout << "Usage : " << argv[0] << " -c <hal.json>" << endl;
    return;
}

static inline sdk_ret_t
nh_test_options_validate (void)
{
    if (!api_test::g_cfg_file) {
        cerr << "HAL config file is not specified" << endl;
        return sdk::SDK_RET_ERR;
    }
    return sdk::SDK_RET_OK;
}

static void
nh_test_options_parse (int argc, char **argv)
{
    int oc = -1;
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
