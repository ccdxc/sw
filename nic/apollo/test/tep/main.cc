//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all tep test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/workflow1.hpp"

namespace api_test {
/// \cond
// Globals
static const char * const k_base_nh_ip = "50.50.1.1";
static const std::string k_base_nh_ipv6 = "e1ba:50::50:1:1";
static const std::string k_base_dipi = "50::50:1:1";
static uint32_t g_mytep_id = 1;
static uint32_t g_tep_id = g_mytep_id + 1;
static constexpr pds_encap_t k_mplsoudp_encap = {PDS_ENCAP_TYPE_MPLSoUDP, 11};
static constexpr pds_encap_t k_vxlan_encap = {PDS_ENCAP_TYPE_VXLAN, 22};
static constexpr bool k_nat = FALSE;

//----------------------------------------------------------------------------
// TEP test class
//----------------------------------------------------------------------------

class tep_test : public ::pds_test_base {
protected:
    tep_test() {}
    virtual ~tep_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        if (apulu()) {
            // TODO: enable once if, nh & nh_group issues are fixed
            // sample_if_setup(bctxt);
            // sample_nexthop_setup(bctxt);
            // sample_nexthop_group_setup(bctxt);
        }
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        if (apulu()) {
            // sample_if_teardown(bctxt);
            // sample_nexthop_teardown(bctxt);
            // sample_nexthop_group_teardown(bctxt);
        }
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};
/// \endcond
//----------------------------------------------------------------------------
// TEP test cases implementation
//----------------------------------------------------------------------------

/// \defgroup TEP TEP Tests
/// @{

TEST_F(tep_test, apulu_tep_workflow_setup) {
    pds_batch_ctxt_t bctxt = batch_start();

    if (!apulu()) return;
    // TODO: temporary until if, nh & nh_group issues are fixed
    sample_if_setup(bctxt);
    sample_nexthop_setup(bctxt);
    batch_commit(bctxt);
}

/// \brief TEP WF_1
/// \ref WF_1
TEST_F(tep_test, tep_workflow_1) {
    tep_feeder feeder;

    if (artemis())
        feeder.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                    k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else if (apulu())
        feeder.init(g_tep_id, k_tep_mac, k_base_nh_ip);
    else
        feeder.init(g_tep_id, k_base_nh_ip);
    workflow_1<tep_feeder>(feeder);
}

/// \brief TEP WF_2
/// \ref WF_2
TEST_F(tep_test, tep_workflow_2) {
    tep_feeder feeder;

    if (artemis())
        feeder.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                    k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else if (apulu())
        feeder.init(g_tep_id, k_tep_mac, k_base_nh_ip);
    else
        feeder.init(g_tep_id, k_base_nh_ip);
    workflow_2<tep_feeder>(feeder);
}

/// \brief TEP WF_3
/// \ref WF_3
TEST_F(tep_test, tep_workflow_3) {
    tep_feeder feeder1, feeder2, feeder3;

    if (artemis()) {
        feeder1.init(10, "10.10.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        feeder2.init(40, "20.20.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        feeder3.init(70, "30.30.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else if (apulu()) {
        feeder1.init(10, k_tep_mac, "10.10.1.1", 10);
        feeder2.init(40, k_tep_mac+100, "20.20.1.1", 20);
        feeder3.init(70, k_tep_mac+200, "30.30.1.1", 30);
    } else {
        feeder1.init(10, "10.10.1.1", 10);
        feeder2.init(40, "20.20.1.1", 20);
        feeder3.init(70, "30.30.1.1", 30);
    }
    workflow_3<tep_feeder>(feeder1, feeder2, feeder3);
}

/// \brief TEP WF_4
/// \ref WF_4
TEST_F(tep_test, tep_workflow_4) {
    tep_feeder feeder;

    if (artemis())
        feeder.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                    k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else if (apulu())
        feeder.init(g_tep_id, k_tep_mac, k_base_nh_ip);
    else
        feeder.init(g_tep_id, k_base_nh_ip);
    workflow_4<tep_feeder>(feeder);
}

/// \brief TEP WF_5
/// \ref WF_5
TEST_F(tep_test, tep_workflow_5) {
    tep_feeder feeder1, feeder2, feeder3;

    if (artemis()) {
        feeder1.init(10, "10.10.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        feeder2.init(40, "20.20.1.1", 20, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        feeder3.init(70, "30.30.1.1", 30, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else if (apulu()) {
        feeder1.init(10, k_tep_mac, "10.10.1.1", 10);
        feeder2.init(40, k_tep_mac+100, "20.20.1.1", 20);
        feeder3.init(70, k_tep_mac+200, "30.30.1.1", 30);
    } else {
        feeder1.init(10, "10.10.1.1", 10);
        feeder2.init(40, "20.20.1.1", 20);
        feeder3.init(70, "30.30.1.1", 30);
    }
    workflow_5<tep_feeder>(feeder1, feeder2, feeder3);
}

/// \brief TEP WF_6
/// \ref WF_6
TEST_F(tep_test, tep_workflow_6) {
    tep_feeder feeder1, feeder1A, feeder1B;

    if (artemis()) {
        feeder1.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // feeder1A =  feeder1 + different dipi
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        // feeder1B =  feeder1A + different dipi
        feeder1B.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else if (apulu()) {
        feeder1.init(g_tep_id, k_tep_mac, k_base_nh_ip);
        // feeder1A =  feeder1 + ipv6 remote ip
        feeder1A.init(g_tep_id, k_tep_mac, k_base_nh_ipv6);
        // feeder1B =  feeder1A + different mac + different nh type
        feeder1B.init(g_tep_id, k_tep_mac+30, k_base_nh_ipv6, k_max_tep,
                      PDS_NH_TYPE_UNDERLAY_ECMP);
    } else {
        feeder1.init(g_tep_id, k_base_nh_ip);
        // feeder1A =  feeder1 + different encap
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_vxlan_encap);
        // feeder1B =  feeder1A + different tunnel type, encap, nat
        feeder1B.init(g_tep_id, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_6<tep_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief TEP WF_7
/// \ref WF_7
TEST_F(tep_test, tep_workflow_7) {
    tep_feeder feeder1, feeder1A, feeder1B;

    if (artemis()) {
        feeder1.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // feeder1A =  feeder1 + different dipi
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        // feeder1B =  feeder1A + different dipi
        feeder1B.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else if (apulu()) {
        // TODO: nh group support
        return;
        feeder1.init(g_tep_id, k_tep_mac, k_base_nh_ip);
        // feeder1A =  feeder1 + ipv6 remote ip
        feeder1A.init(g_tep_id, k_tep_mac, k_base_nh_ipv6);
        // feeder1B =  feeder1A + different mac + different nh type
        feeder1B.init(g_tep_id, k_tep_mac+30, k_base_nh_ipv6, k_max_tep,
                      PDS_NH_TYPE_UNDERLAY_ECMP);
    } else {
        feeder1.init(g_tep_id, k_base_nh_ip);
        // feeder1A =  feeder1 + different encap
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_vxlan_encap);
        // feeder1B =  feeder1A + different tunnel type, encap, nat
        feeder1B.init(g_tep_id, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_7<tep_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief TEP WF_8
/// \ref WF_8
TEST_F(tep_test, DISABLED_tep_workflow_8) {
    tep_feeder feeder1, feeder1A, feeder1B;

    if (artemis()) {
        feeder1.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // feeder1A =  feeder1 + different dipi
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        // feeder1B =  feeder1A + different dipi
        feeder1B.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
    } else if (apulu()) {
        feeder1.init(g_tep_id, k_tep_mac, k_base_nh_ip);
        // feeder1A =  feeder1 + ipv6 remote ip
        feeder1A.init(g_tep_id, k_tep_mac, k_base_nh_ipv6);
        // feeder1B =  feeder1A + different mac + different nh type
        feeder1B.init(g_tep_id, k_tep_mac+30, k_base_nh_ipv6, k_max_tep,
                      PDS_NH_TYPE_UNDERLAY_ECMP);
    } else {
        feeder1.init(g_tep_id, k_base_nh_ip);
        // feeder1A =  feeder1 + different encap
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_vxlan_encap);
        // feeder1B =  feeder1A + different tunnel type, encap, nat
        feeder1B.init(g_tep_id, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_8<tep_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief TEP WF_9
/// \ref WF_9
TEST_F(tep_test, tep_workflow_9) {
    tep_feeder feeder1, feeder1A;

    if (artemis()) {
        feeder1.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        // feeder1A =  feeder1 + different dipi
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
    } else if (apulu()) {
        feeder1.init(g_tep_id, k_tep_mac, k_base_nh_ip);
        // feeder1A =  feeder1 + different remote ip, mac & nh type
        feeder1A.init(g_tep_id, k_tep_mac+10, k_base_nh_ipv6, k_max_tep,
                      PDS_NH_TYPE_UNDERLAY_ECMP);
    } else {
        feeder1.init(g_tep_id, k_base_nh_ip);
        // feeder1A =  feeder1 + different tunnel type, encap, nat
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_vxlan_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_9<tep_feeder>(feeder1, feeder1A);
}

/// \brief TEP WF_10
/// \ref WF_10
TEST_F(tep_test, DISABLED_tep_workflow_10) {
    tep_feeder feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4;

    if (artemis()) {
        feeder1.init(10, "10.10.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        feeder2.init(40, "20.20.1.1", 20, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
        feeder2A.init(40, "20.20.1.1", 20, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "20::20:20:21", 0);
        feeder3.init(70, "30.30.1.1", 30, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "30::30:1:1", 0);
        feeder3A.init(70, "30.30.1.1", 30, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "30::30:30:31", 0);
        feeder4.init(100, "40.40.1.1", 40, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "40::40:1:1", 0);
    } else if (apulu()) {
        feeder1.init(10, k_tep_mac, "10.10.1.1", 10);
        feeder2.init(40, k_tep_mac, "20.20.1.1", 20);
        feeder2A.init(40, k_tep_mac+20, "20:20:1::1", 20);
        feeder3.init(70, k_tep_mac, "30.30.1.1", 30);
        feeder3A.init(70, k_tep_mac+30, "30:30:1::1", 30);
        feeder4.init(100, k_tep_mac, "40.40.1.1", 40);
    } else {
        feeder1.init(10, "10.10.1.1", 10);
        feeder2.init(40, "20.20.1.1", 20);
        feeder2A.init(40, "20.20.1.1", 20, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
        feeder3.init(70, "30.30.1.1", 30);
        feeder3A.init(70, "30.30.1.1", 30, k_mplsoudp_encap, k_nat);
        feeder4.init(100, "40.40.1.1", 40);
    }
    workflow_10<tep_feeder>(feeder1, feeder2, feeder2A,
                            feeder3, feeder3A, feeder4);
}

/// \brief TEP WF_N_1
/// \ref WF_N_1
TEST_F(tep_test, tep_workflow_neg_1) {
    tep_feeder feeder;

    if (artemis())
        feeder.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                    k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else if (apulu())
        feeder.init(g_tep_id, k_tep_mac, k_base_nh_ip);
    else
        feeder.init(g_tep_id, k_base_nh_ip);
    workflow_neg_1<tep_feeder>(feeder);
}

/// \brief TEP WF_N_2
/// \ref WF_N_2
TEST_F(tep_test, tep_workflow_neg_2) {
    tep_feeder feeder;

    if (artemis())
        feeder.init(g_tep_id, k_base_nh_ip, k_max_tep+2, k_zero_encap,
                    k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
    else if (apulu())
        feeder.init(g_tep_id, k_tep_mac, k_base_nh_ip, k_max_tep+2);
    else
        feeder.init(g_tep_id, k_base_nh_ip, k_max_tep+2);
    workflow_neg_2<tep_feeder>(feeder);
}

/// \brief TEP WF_N_3
/// \ref WF_N_3
TEST_F(tep_test, tep_workflow_neg_3) {
    tep_feeder feeder;

    if (artemis())
        feeder.init(g_tep_id, "150.150.1.1", k_max_tep, k_zero_encap,
                    k_nat, PDS_TEP_TYPE_SERVICE, "150::150:1:1", 0);
    else if (apulu())
        feeder.init(g_tep_id, k_tep_mac, "150.150.1.1");
    else
        feeder.init(g_tep_id, "150.150.1.1");
    workflow_neg_3<tep_feeder>(feeder);
}

/// \brief TEP WF_N_4
/// \ref WF_N_4
TEST_F(tep_test, tep_workflow_neg_4) {
    tep_feeder feeder1, feeder2;

    if (artemis()) {
        feeder1.init(10, k_base_nh_ip, 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        feeder2.init(40, "60.60.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
    } else if (apulu()) {
        feeder1.init(10, k_tep_mac, k_base_nh_ip, 10);
        feeder2.init(40, k_tep_mac+10, "60.60.1.1", 10);
    } else {
        feeder1.init(10, k_base_nh_ip, 10);
        feeder2.init(40, "60.60.1.1", 10);
    }
    workflow_neg_4<tep_feeder>(feeder1, feeder2);
}

/// \brief TEP WF_N_5
/// \ref WF_N_5
TEST_F(tep_test, DISABLED_tep_workflow_neg_5) {
    tep_feeder feeder1, feeder1A;

    if (artemis()) {
        feeder1.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
    } else if (apulu()) {
        feeder1.init(g_tep_id, k_tep_mac, k_base_nh_ip);
        // feeder1A = feeder1 + different remote ip, mac & nh type
        feeder1A.init(g_tep_id, k_tep_mac+10, k_base_nh_ipv6, k_max_tep,
                      PDS_NH_TYPE_UNDERLAY_ECMP);
    } else {
        feeder1.init(g_tep_id, k_base_nh_ip);
        // feeder1A = feeder + different tunnel type, nat
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_neg_5<tep_feeder>(feeder1, feeder1A);
}

/// \brief TEP WF_N_6
/// \ref WF_N_6
TEST_F(tep_test, tep_workflow_neg_6) {
    tep_feeder feeder1, feeder1A;

    if (artemis()) {
        feeder1.init(g_tep_id, k_base_nh_ip, k_max_tep, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, k_base_dipi, 0);
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep+1, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
    } else if (apulu()) {
        feeder1.init(g_tep_id, k_tep_mac, k_base_nh_ip);
        // feeder1A = feeder1 + v6 remote ip + different mac
        feeder1A.init(g_tep_id, k_tep_mac+10, k_base_nh_ipv6, k_max_tep+1);
    } else {
        feeder1.init(g_tep_id, k_base_nh_ip);
        // feeder1A = feeder1 + different tunnel type, nat
        feeder1A.init(g_tep_id, k_base_nh_ip, k_max_tep+1, k_mplsoudp_encap,
                      k_nat, PDS_TEP_TYPE_IGW);
    }
    workflow_neg_6<tep_feeder>(feeder1, feeder1A);
}

/// \brief TEP WF_N_7
/// \ref WF_N_7
TEST_F(tep_test, tep_workflow_neg_7) {
    tep_feeder feeder1, feeder1A, feeder2;

    if (artemis()) {
        feeder1.init(10, "10.10.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        feeder1A.init(10, "10.10.1.1", 10, k_zero_encap,
                      k_nat, PDS_TEP_TYPE_SERVICE, "60::60:1:1", 0);
        feeder2.init(40, "20.20.1.1", 20, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
    } else if (apulu()) {
        feeder1.init(10, k_tep_mac, "10.10.1.1", 10);
        feeder1A.init(10, k_tep_mac+10, "10:10:1::1", 10);
        feeder2.init(40, k_tep_mac, "20.20.1.1", 20);
    } else {
        feeder1.init(10, "10.10.1.1", 10, k_mplsoudp_encap,
                     k_nat, PDS_TEP_TYPE_IGW);
        feeder1A.init(10, "10.10.1.1", 10);
        feeder2.init(40, "20.20.1.1", 20);
    }
    workflow_neg_7<tep_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief TEP WF_N_8
/// \ref WF_N_8
TEST_F(tep_test, tep_workflow_neg_8) {
    tep_feeder feeder1, feeder2;

    if (artemis()) {
        feeder1.init(10, "10.10.1.1", 10, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "10::10:1:1", 0);
        feeder2.init(40, "20.20.1.1", 20, k_zero_encap,
                     k_nat, PDS_TEP_TYPE_SERVICE, "20::20:1:1", 0);
    } else if (apulu()) {
        feeder1.init(10, k_tep_mac, "10.10.1.1", 10);
        feeder2.init(40, k_tep_mac+10, "20.20.1.1", 20);
    } else {
        feeder1.init(10, "10.10.1.1", 10, k_mplsoudp_encap,
                     k_nat, PDS_TEP_TYPE_IGW);
        feeder2.init(40, "20.20.1.1", 20);
    }
    workflow_neg_8<tep_feeder>(feeder1, feeder2);
}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
