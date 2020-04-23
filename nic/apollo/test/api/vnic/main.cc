
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all vnic test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/policy.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

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
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample_device_setup(bctxt);
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        sample_policy_setup(bctxt);
        sample_subnet_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_subnet_teardown(bctxt);
        sample_policy_teardown(bctxt);
        sample_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        sample_device_teardown(bctxt);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC Vnic Tests
/// @{

/// \brief VNIC WF_B1
/// \ref WF_B1
TEST_F(vnic_test, vnic_workflow_b1) {
    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_b1<vnic_feeder>(feeder);
}

/// \brief VNIC WF_B2
/// \ref WF_B2
TEST_F(vnic_test, vnic_workflow_b2) {
    if (!apulu()) return;

    vnic_feeder feeder1, feeder1A;

    feeder1.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN, FALSE,
                  TRUE, 0, 0, 5, 0);
    workflow_b2<vnic_feeder>(feeder1, feeder1A);
}

/// \brief VNIC WF_1
/// \ref WF_1
TEST_F(vnic_test, vnic_workflow_1) {
    if (artemis()) return;

    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_1<vnic_feeder>(feeder);
}

/// \brief VNIC WF_2
/// \ref WF_2
TEST_F(vnic_test, vnic_workflow_2) {
    if (artemis()) return;

    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_2<vnic_feeder>(feeder);
}

/// \brief VNIC WF_3
/// \ref WF_3
TEST_F(vnic_test, vnic_workflow_3) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder2, feeder3;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder2.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder3.init(int2pdsobjkey(70), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    workflow_3<vnic_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VNIC WF_4
/// \ref WF_4
TEST_F(vnic_test, vnic_workflow_4) {
    if (artemis()) return;

    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_4<vnic_feeder>(feeder);
}

/// \brief VNIC WF_5
/// \ref WF_5
TEST_F(vnic_test, vnic_workflow_5) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder2, feeder3;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder2.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder3.init(int2pdsobjkey(70), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    workflow_5<vnic_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VNIC WF_6
/// \ref WF_6
TEST_F(vnic_test, vnic_workflow_6) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                  true, true, 0, 0, 5, 0);
    feeder1B.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    workflow_6<vnic_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VNIC WF_7
/// \ref WF_7
TEST_F(vnic_test, vnic_workflow_7) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                  true, true, 0, 0, 5, 0);
    feeder1B.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    workflow_7<vnic_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VNIC WF_8
/// \ref WF_8
TEST_F(vnic_test, vnic_workflow_8) {
    if (!apulu()) return;
    vnic_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    feeder1B.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                  FALSE, TRUE, 0, 0, 5, 0);
    workflow_8<vnic_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VNIC WF_9
/// \ref WF_9
TEST_F(vnic_test, vnic_workflow_9) {
    if (!apulu()) return;

    vnic_feeder feeder1, feeder1A;

    feeder1.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  true, true, 0, 0, 5, 0);
    workflow_9<vnic_feeder>(feeder1, feeder1A);
}

/// \brief VNIC WF_10
/// \ref WF_10
TEST_F(vnic_test, vnic_workflow_10) {
    if (!apulu()) return;

    vnic_feeder feeder1, feeder2, feeder3, feeder4;
    vnic_feeder feeder2A, feeder3A;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder2.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, 0x202020000000,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder2A.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, 0x202020000000,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    feeder3.init(int2pdsobjkey(70), int2pdsobjkey(1), 20, 0x303030000000,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder3A.init(int2pdsobjkey(70), int2pdsobjkey(1), 20, 0x303030000000,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    feeder4.init(int2pdsobjkey(100), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    workflow_10<vnic_feeder>(feeder1, feeder2, feeder2A, feeder3,
                             feeder3A, feeder4);
}

/// \brief VNIC WF_N_1
/// \ref WF_N_1
TEST_F(vnic_test, vnic_workflow_neg_1) {
    if (artemis()) return;

    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_neg_1<vnic_feeder>(feeder);
}

/// \brief VNIC WF_N_2
/// \ref WF_N_2
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_2) {
    if (artemis()) return;

    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic+1,
                k_feeder_mac, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_neg_2<vnic_feeder>(feeder);
}

/// \brief VNIC WF_N_3
/// \ref WF_N_3
TEST_F(vnic_test, vnic_workflow_neg_3) {
    if (artemis()) return;

    vnic_feeder feeder;

    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    workflow_neg_3<vnic_feeder>(feeder);
}

/// \brief VNIC WF_N_4
/// \ref WF_N_4
TEST_F(vnic_test, vnic_workflow_neg_4) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder2;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder2.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, 0x202020000000,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    workflow_neg_4<vnic_feeder>(feeder1, feeder2);
}

/// \brief VNIC WF_N_5
/// \ref WF_N_5
TEST_F(vnic_test, vnic_workflow_neg_5) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder1A;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(10), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    workflow_neg_5<vnic_feeder>(feeder1, feeder1A);
}

/// \brief VNIC WF_N_6
/// \ref WF_N_6
TEST_F(vnic_test, vnic_workflow_neg_6) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder1A;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), k_max_vnic, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(10), int2pdsobjkey(1), k_max_vnic+1,
                  k_feeder_mac, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    workflow_neg_6<vnic_feeder>(feeder1, feeder1A);
}

/// \brief VNIC WF_N_7
/// \ref WF_N_7
TEST_F(vnic_test, vnic_workflow_neg_7) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder1A, feeder2;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder1A.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                  PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                  FALSE, TRUE, 0, 0, 5, 0);
    feeder2.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, 0x202020000000,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    workflow_neg_7<vnic_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief VNIC WF_N_8
/// \ref WF_N_8
TEST_F(vnic_test, vnic_workflow_neg_8) {
    if (artemis()) return;

    vnic_feeder feeder1, feeder2;

    feeder1.init(int2pdsobjkey(10), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                 true, true, 0, 0, 5, 0);
    feeder2.init(int2pdsobjkey(40), int2pdsobjkey(1), 20, k_feeder_mac,
                 PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN,
                 FALSE, TRUE, 0, 0, 5, 0);
    workflow_neg_8<vnic_feeder>(feeder1, feeder2);
}

//---------------------------------------------------------------------
// Non templatized test cases
//---------------------------------------------------------------------

/// \brief update vnic hostname
TEST_F(vnic_test, vnic_update_hostname) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    strcpy(spec.hostname, "updated");
    vnic_update(feeder, &spec, VNIC_ATTR_HOSTNAME);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update subnet
TEST_F(vnic_test, vnic_update_subnet) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};
    pds_vnic_spec_t old_spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    memcpy(&old_spec, &feeder.spec, sizeof(pds_vnic_spec_t));

    // trigger
    spec.subnet = int2pdsobjkey(2);
    // update should fail as subnet id is immutable attribute
    vnic_update(feeder, &spec, VNIC_ATTR_SUBNET, SDK_RET_ERR);

    // validate
    // as the update fails, rollback the feeder to original values
    memcpy(&feeder.spec, &old_spec, sizeof(pds_vnic_spec_t));
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic encap val
TEST_F(vnic_test, vnic_update_vnic_encap_val) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.vnic_encap = feeder.spec.vnic_encap;
    vnic_feeder_encap_next(&spec.vnic_encap);
    vnic_update(feeder, &spec, VNIC_ATTR_VNIC_ENCAP);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic encap type
TEST_F(vnic_test, DISABLED_vnic_update_vnic_encap_type) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    vnic_feeder_encap_init(1, PDS_ENCAP_TYPE_MPLSoUDP, &spec.vnic_encap);
    vnic_update(feeder, &spec, VNIC_ATTR_VNIC_ENCAP);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update fab encap val
TEST_F(vnic_test, vnic_update_fab_encap_val) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.fabric_encap = feeder.spec.fabric_encap;
    vnic_feeder_encap_next(&spec.fabric_encap);
    vnic_update(feeder, &spec, VNIC_ATTR_FAB_ENCAP);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic encap type
TEST_F(vnic_test, vnic_update_fab_encap_type) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    vnic_feeder_encap_init(1, PDS_ENCAP_TYPE_DOT1Q, &spec.fabric_encap);
    vnic_update(feeder, &spec, VNIC_ATTR_FAB_ENCAP);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mac address
TEST_F(vnic_test, vnic_update_mac_1) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};
    pds_vnic_spec_t old_spec = {0};
    uint64_t mac;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    memcpy(&old_spec, &feeder.spec, sizeof(pds_vnic_spec_t));

    // trigger
    mac = MAC_TO_UINT64(feeder.spec.mac_addr);
    // updated mac should fail as it is immutable attribute
    mac = mac + 2*k_max_vnic;
    MAC_UINT64_TO_ADDR(spec.mac_addr, mac);
    vnic_update(feeder, &spec, VNIC_ATTR_MAC_ADDR, SDK_RET_ERR);

    // validate
    // as the update fails, rollback the feeder to original values
    memcpy(&feeder.spec, &old_spec, sizeof(pds_vnic_spec_t));
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mac address
TEST_F(vnic_test, vnic_update_mac_2) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};
    pds_vnic_spec_t old_spec = {0};
    uint64_t mac;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    memcpy(&old_spec, &feeder.spec, sizeof(pds_vnic_spec_t));

    // trigger
    mac = MAC_TO_UINT64(feeder.spec.mac_addr);
    // reserve fails as we are using same mac for some other vnic
    mac++;
    MAC_UINT64_TO_ADDR(spec.mac_addr, mac);
    // update should fail as mac is used by another vnic
    vnic_update(feeder, &spec, VNIC_ATTR_MAC_ADDR, SDK_RET_ERR);

    // validate
    // as the update fails, rollback the feeder to original values
    memcpy(&feeder.spec, &old_spec, sizeof(pds_vnic_spec_t));
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic binding checks
TEST_F(vnic_test, vnic_update_binding_checks) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.binding_checks_en = FALSE;
    vnic_update(feeder, &spec, VNIC_ATTR_BINDING_CHECKS_EN);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic tx mirror bitmap
TEST_F(vnic_test, vnic_update_tx_mirror_session) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.tx_mirror_session_bmap = 1;
    vnic_update(feeder, &spec, VNIC_ATTR_TX_MIRROR);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic rx mirror bitmap
TEST_F(vnic_test, vnic_update_rx_mirror_sessioon) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.rx_mirror_session_bmap = 1;
    vnic_update(feeder, &spec, VNIC_ATTR_RX_MIRROR);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update v4 meter
TEST_F(vnic_test, vnic_update_v4_meter) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.v4_meter = int2pdsobjkey(pdsobjkey2int(feeder.spec.v4_meter) + 1);
    vnic_update(feeder, &spec, VNIC_ATTR_V4_METER);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic v6 meter
TEST_F(vnic_test, vnic_update_v6_meter) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.v6_meter = int2pdsobjkey(pdsobjkey2int(feeder.spec.v6_meter) + 1);
    vnic_update(feeder, &spec, VNIC_ATTR_V6_METER);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vnic switch flag
TEST_F(vnic_test, vnic_update_switch_vnic) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger switch_vnic update (switch vnics should have binding checks
    // disable and no VLAN encap)
    spec.switch_vnic = true;
    spec.binding_checks_en = false;
    spec.vnic_encap.type = PDS_ENCAP_TYPE_NONE;
    vnic_update(feeder, &spec,
                VNIC_ATTR_SWITCH_VNIC | VNIC_ATTR_VNIC_ENCAP |
                    VNIC_ATTR_BINDING_CHECKS_EN);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach one policy P1 to vnic V1 and then update
///        V1 by adding P2 to V1 (so V1 ends up with P1 and P2)
TEST_F(vnic_test, vnic_update_policy1) {
    if (!apulu()) return;

    pds_vnic_spec_t spec = {0};
    vnic_feeder feeder;
    uint8_t num_policies = 1;
    uint8_t start_pol_index = 0;
    uint64_t chg_bmap = 0;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, num_policies, start_pol_index);
    vnic_create(feeder);

    // trigger
    num_policies = 2;
    start_pol_index = 0;
    vnic_spec_policy_fill(&spec, num_policies, start_pol_index);
    chg_bmap = ( VNIC_ATTR_V4_INGPOL | VNIC_ATTR_V6_INGPOL |
                 VNIC_ATTR_V4_EGRPOL | VNIC_ATTR_V6_EGRPOL);
    vnic_update(feeder, &spec, chg_bmap);

    // validate
    vnic_read(feeder);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach policy P1 to vnic V1 and then update V1
///        with no policy
TEST_F(vnic_test, vnic_update_policy2) {
    if (!apulu()) return;

    pds_vnic_spec_t spec = {0};
    vnic_feeder feeder;
    uint8_t num_policies = 1;
    uint8_t start_pol_index = 0;
    uint64_t chg_bmap = 0;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, num_policies, start_pol_index);
    vnic_create(feeder);

    // trigegr
    num_policies = 0;
    start_pol_index = 0;
    vnic_spec_policy_fill(&spec, num_policies, start_pol_index);
    chg_bmap = ( VNIC_ATTR_V4_INGPOL | VNIC_ATTR_V6_INGPOL |
                 VNIC_ATTR_V4_EGRPOL | VNIC_ATTR_V6_EGRPOL);
    vnic_update(feeder, &spec, chg_bmap);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach policy P1, P2 to vnic V1 and update V1
///        with P1 only (disassociating P2) and then with no policies
TEST_F(vnic_test, vnic_update_policy3) {
    if (!apulu()) return;

    pds_vnic_spec_t spec = {0};
    vnic_feeder feeder;
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t chg_bmap = 0;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, num_policies, start_pol_index);
    vnic_create(feeder);

    // trigger1
    num_policies = 1;
    start_pol_index = 0;
    vnic_spec_policy_fill(&spec, num_policies, start_pol_index);
    chg_bmap = ( VNIC_ATTR_V4_INGPOL | VNIC_ATTR_V6_INGPOL |
                 VNIC_ATTR_V4_EGRPOL | VNIC_ATTR_V6_EGRPOL);
    vnic_update(feeder, &spec, chg_bmap);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // trigger2
    memset(&spec, 0, sizeof(spec));
    num_policies = 0;
    start_pol_index = 0;
    vnic_spec_policy_fill(&spec, num_policies, start_pol_index);
    chg_bmap = ( VNIC_ATTR_V4_INGPOL | VNIC_ATTR_V6_INGPOL |
                 VNIC_ATTR_V4_EGRPOL | VNIC_ATTR_V6_EGRPOL);
    vnic_update(feeder, &spec, chg_bmap);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - Attach policy P1, P2 to vnic V1 and update V1
///        with P3, P4
TEST_F(vnic_test, vnic_update_policy4) {
    if (!apulu()) return;

    pds_vnic_spec_t spec = {0};
    vnic_feeder feeder;
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t chg_bmap = 0;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, num_policies, start_pol_index);
    vnic_create(feeder);


    // trigegr
    num_policies = 2;
    start_pol_index = 2;
    vnic_spec_policy_fill(&spec, num_policies, start_pol_index);
    chg_bmap = ( VNIC_ATTR_V4_INGPOL | VNIC_ATTR_V6_INGPOL |
                 VNIC_ATTR_V4_EGRPOL | VNIC_ATTR_V6_EGRPOL);
    vnic_update(feeder, &spec, chg_bmap);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach policy P1, P2 to vnic V1 and update V1
///        with P2, P3
TEST_F(vnic_test, vnic_update_policy5) {
    if (!apulu()) return;

    pds_vnic_spec_t spec = {0};
    vnic_feeder feeder;
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t chg_bmap = 0;

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, num_policies, start_pol_index);
    vnic_create(feeder);


    // trigegr
    num_policies = 2;
    start_pol_index = 1;
    vnic_spec_policy_fill(&spec, num_policies, start_pol_index);
    chg_bmap = ( VNIC_ATTR_V4_INGPOL | VNIC_ATTR_V6_INGPOL |
                 VNIC_ATTR_V4_EGRPOL | VNIC_ATTR_V6_EGRPOL);
    vnic_update(feeder, &spec, chg_bmap);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update hostif
TEST_F(vnic_test, DISABLED_vnic_update_hostif) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.host_if = int2pdsobjkey(pdsobjkey2int(feeder.spec.host_if) + 1);
    vnic_update(feeder, &spec, VNIC_ATTR_HOST_IF);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update tx policer
TEST_F(vnic_test, vnic_update_tx_policer) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.tx_policer = int2pdsobjkey(pdsobjkey2int(feeder.spec.tx_policer) + 1);
    vnic_update(feeder, &spec, VNIC_ATTR_TX_POLICER);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update rx policer
TEST_F(vnic_test, vnic_update_rx_policer) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.rx_policer = int2pdsobjkey(pdsobjkey2int(feeder.spec.rx_policer) + 1);
    vnic_update(feeder, &spec, VNIC_ATTR_RX_POLICER);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update primary flag
TEST_F(vnic_test, vnic_update_primary) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};
    pds_vnic_spec_t old_spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    memcpy(&old_spec, &feeder.spec, sizeof(pds_vnic_spec_t));

    // trigger
    spec.primary = true;
    vnic_update(feeder, &spec, VNIC_ATTR_PRIMARY, SDK_RET_OK);

    // validate
    // as the update fails, rollback the feeder to original values
    memcpy(&feeder.spec, &old_spec, sizeof(pds_vnic_spec_t));
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update max sessions
TEST_F(vnic_test, vnic_update_max_sessions) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.max_sessions++;
    vnic_update(feeder, &spec, VNIC_ATTR_MAX_SESSIONS);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update flow learn enable
TEST_F(vnic_test, vnic_update_flow_learn) {
    if (!apulu()) return;

    vnic_feeder feeder;
    pds_vnic_spec_t spec = {0};

    // init
    feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), 1, k_feeder_mac,
                PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP,
                true, true, 0, 0, 5, 0);
    vnic_create(feeder);

    // trigger
    spec.flow_learn_en = true;
    vnic_update(feeder, &spec, VNIC_ATTR_FLOW_LEARN_EN);

    // validate
    vnic_read(feeder, SDK_RET_OK);

    // cleanup
    vnic_delete(feeder);
    vnic_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
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
