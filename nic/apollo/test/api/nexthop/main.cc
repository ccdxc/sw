//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all nexthop test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static const std::string k_base_nh_ip = "50.50.1.1";
static constexpr uint64_t k_base_mac = 0x0E0D0A0B0200;

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
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample_if_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_if_teardown(bctxt);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// NH test cases implementation
//----------------------------------------------------------------------------

/// \defgroup NH Nexthop Tests
/// @{

/// \brief NH WF_B1
/// \ref WF_B1
TEST_F(nh_test, nh_workflow_b1) {
    if (!apulu()) return;

    nexthop_feeder feeder;

    feeder.init("", 0x0E0D0A0B0100, 1, 1, PDS_NH_TYPE_UNDERLAY);
    workflow_b1<nexthop_feeder>(feeder);
}

/// \brief NH WF_B2
/// \ref WF_B2
TEST_F(nh_test, DISABLED_nh_workflow_b2) {
    if (!apulu()) return;

    nexthop_feeder feeder1, feeder1A;

    feeder1.init("", 0x0E0D0A0B0100, 1, 1, PDS_NH_TYPE_UNDERLAY);
    feeder1A.init("", 0x0E0D0A0B0200, 1, 1, PDS_NH_TYPE_UNDERLAY);
    workflow_b2<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_1
/// \ref WF_1
TEST_F(nh_test, nh_workflow_1) {
    nexthop_feeder feeder;

    if (apulu()) {
        feeder.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder.init(k_base_nh_ip);
    }

    workflow_1<nexthop_feeder>(feeder);
}

/// \brief NH WF_2
/// \ref WF_2
TEST_F(nh_test, nh_workflow_2) {
    nexthop_feeder feeder;

    if (apulu()) {
        feeder.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder.init(k_base_nh_ip);
    }

    workflow_2<nexthop_feeder>(feeder);
}

/// \brief NH WF_3
/// \ref WF_3
TEST_F(nh_test, nh_workflow_3) {
    nexthop_feeder feeder1, feeder2, feeder3;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder2.init("", 0x0E0D0A0B0200, 10, 200, PDS_NH_TYPE_UNDERLAY);
        feeder3.init("", 0x0E0D0A0B0300, 10, 300, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
        feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
        feeder3.init("30.30.1.1", 0x0E010B0A3000, 30, 300);
    }
    workflow_3<nexthop_feeder>(feeder1, feeder2, feeder3);
}

/// \brief NH WF_4
/// \ref WF_4
TEST_F(nh_test, nh_workflow_4) {
    nexthop_feeder feeder;

    if (apulu())
        feeder.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    else
        feeder.init(k_base_nh_ip);
    workflow_4<nexthop_feeder>(feeder);
}

/// \brief NH WF_5
/// \ref WF_5
TEST_F(nh_test, nh_workflow_5) {
    nexthop_feeder feeder1, feeder2, feeder3;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder2.init("", 0x0E0D0A0B0200, 10, 200, PDS_NH_TYPE_UNDERLAY);
        feeder3.init("", 0x0E0D0A0B0300, 10, 300, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
        feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
        feeder3.init("30.30.1.1", 0x0E010B0A3000, 30, 300);
    }
    workflow_5<nexthop_feeder>(feeder1, feeder2, feeder3);
}

/// \brief NH WF_6
/// \ref WF_6
TEST_F(nh_test, nh_workflow_6) {
    nexthop_feeder feeder1, feeder1A, feeder1B;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1B.init("", 0x0E0D0A0B0300, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip);
        // feeder1A =  feeder1 + different mac
        feeder1A.init(k_base_nh_ip, 0x0E010B0A3000);
        // feeder1B =  feeder1A + different mac & ip
        feeder1B.init("30.30.1.1", 0x0E010B0A2000);
    }
    workflow_6<nexthop_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief NH WF_7
/// \ref WF_7
TEST_F(nh_test, nh_workflow_7) {
    nexthop_feeder feeder1, feeder1A, feeder1B;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1B.init("", 0x0E0D0A0B0300, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip);
        // feeder1A =  feeder1 + different mac
        feeder1A.init(k_base_nh_ip, 0x0E010B0A3000);
        // feeder1B =  feeder1A + different mac & ip
        feeder1B.init("30.30.1.1", 0x0E010B0A2000);
    }
    workflow_7<nexthop_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief NH WF_8
/// \ref WF_8
TEST_F(nh_test, nh_workflow_8) {
    nexthop_feeder feeder1, feeder1A, feeder1B;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1B.init("", 0x0E0D0A0B0300, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip);
        // feeder1A =  feeder1 + different mac
        feeder1A.init(k_base_nh_ip, 0x0E010B0A3000);
        // feeder1B =  feeder1A + different mac & ip
        feeder1B.init("30.30.1.1", 0x0E010B0A2000);
    }
    workflow_8<nexthop_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief NH WF_9
/// \ref WF_9
TEST_F(nh_test, nh_workflow_9) {
    nexthop_feeder feeder1, feeder1A;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip);
        // feeder1A =  feeder1 + different mac & ip
        feeder1A.init("30.30.1.1", 0x0E010B0A2000);
    }
    workflow_9<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_10
/// \ref WF_10
TEST_F(nh_test, nh_workflow_10) {
    nexthop_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder2.init("", 0x0E0D0A0B0200, 10, 200, PDS_NH_TYPE_UNDERLAY);
        feeder2A.init("", 0x0E0D0A0B0210, 10, 200, PDS_NH_TYPE_UNDERLAY);
        feeder3.init("", 0x0E0D0A0B0300, 10, 300, PDS_NH_TYPE_UNDERLAY);
        feeder3A.init("", 0x0E0D0A0B0210, 10, 300, PDS_NH_TYPE_UNDERLAY);
        feeder4.init("", 0x0E0D0A0B0400, 10, 400, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
        feeder2.init("20.200.1.1", 0x0E010B0A2000, 20, 200);
        // feeder2A =  feeder2 + different mac & ip
        feeder2A.init("20.20.1.1", 0x0E010B0A2222, 20, 200);
        feeder3.init("30.300.1.1", 0x0E010B0A3000, 30, 300);
        // feeder3A =  feeder3 + different mac & ip
        feeder3A.init("30.30.1.1", 0x0E010B0A3333, 30, 300);
        feeder4.init("40.40.1.1", 0x0E010B0A4000, 40, 400);
    }
    workflow_10<nexthop_feeder>(feeder1, feeder2, feeder2A,
                           feeder3, feeder3A, feeder4);
}

/// \brief NH WF_N_1
/// \ref WF_N_1
TEST_F(nh_test, nh_workflow_neg_1) {
    nexthop_feeder feeder;

    if (apulu())
        feeder.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    else
        feeder.init(k_base_nh_ip);
    workflow_neg_1<nexthop_feeder>(feeder);
}

/// \brief NH WF_N_2
/// \ref WF_N_2
TEST_F(nh_test, DISABLED_nh_workflow_neg_2) {
    nexthop_feeder feeder;

    // TODO: PDS_MAX_NEXTHOP must be 1048576
    if (apulu())
        feeder.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    else
        feeder.init(k_base_nh_ip, k_base_mac, k_max_nh);
    workflow_neg_2<nexthop_feeder>(feeder);
}

/// \brief NH WF_N_3
/// \ref WF_N_3
TEST_F(nh_test, nh_workflow_neg_3) {
    nexthop_feeder feeder;

    if (apulu())
        feeder.init("", 0x0E0D0A0B0200, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    else
        feeder.init("150.150.1.1");
    workflow_neg_3<nexthop_feeder>(feeder);
}

/// \brief NH WF_N_4
/// \ref WF_N_4
TEST_F(nh_test, nh_workflow_neg_4) {
    nexthop_feeder feeder1, feeder2;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder2.init("", 0x0E0D0A0B0200, 10, 200, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip, k_base_mac, 10, 100);
        feeder2.init("60.60.1.1",  0x0E010B0A2000, 10, 200);
    }
    workflow_neg_4<nexthop_feeder>(feeder1, feeder2);
}

/// \brief NH WF_N_5
/// \ref WF_N_5
TEST_F(nh_test, nh_workflow_neg_5) {
    nexthop_feeder feeder1, feeder1A;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip);
        // seed1A = seed1 + different IP & MAC
        feeder1A.init("150.150.1.1", 0x0E010B0A2000);
    }
    workflow_neg_5<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_N_6
/// \ref WF_N_6
TEST_F(nh_test, DISABLED_nh_workflow_neg_6) {
    nexthop_feeder feeder1, feeder1A;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, k_max_nh, 1, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0100, k_max_nh+1, 1, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init(k_base_nh_ip);
        // seed1A = seed1 + different IP & MAC
        feeder1A.init("150.150.1.1", 0x0E010B0A2000, k_max_nh+1);
    }
    workflow_neg_6<nexthop_feeder>(feeder1, feeder1A);
}

/// \brief NH WF_N_7
/// \ref WF_N_7
TEST_F(nh_test, nh_workflow_neg_7) {
    nexthop_feeder feeder1, feeder1A, feeder2;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder1A.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder2.init("", 0x0E0D0A0B0200, 10, 200, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
        feeder1A.init("10.10.1.1", 0x0E010B0A1111, 10, 100);
        feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
    }
    workflow_neg_7<nexthop_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief NH WF_N_8
/// \ref WF_N_8
TEST_F(nh_test, nh_workflow_neg_8) {
    nexthop_feeder feeder1, feeder2;

    if (apulu()) {
        feeder1.init("", 0x0E0D0A0B0100, 10, 100, PDS_NH_TYPE_UNDERLAY);
        feeder2.init("", 0x0E0D0A0B0200, 10, 200, PDS_NH_TYPE_UNDERLAY);
    } else {
        feeder1.init("10.10.1.1", 0x0E010B0A1000, 10, 100);
        feeder2.init("20.20.1.1", 0x0E010B0A2000, 20, 200);
    }
    workflow_neg_8<nexthop_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
