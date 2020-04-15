//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all subnet test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/nexthop_group.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/policy.hpp"
#include "nic/apollo/test/api/utils/route.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static constexpr uint32_t k_max_subnet = PDS_MAX_SUBNET + 1;

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
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);

        pds_batch_ctxt_t bctxt = batch_start();
        sample1_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);
        sample_policy_setup(bctxt);
        if (apulu()) {
            sample_device_setup(bctxt);
            sample_if_setup(bctxt);
            sample_nexthop_setup(bctxt);
            sample_nexthop_group_setup(bctxt);
        }
        sample_tep_setup(bctxt);
        sample_route_table_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_policy_teardown(bctxt);
        if (apulu()) {
            sample_nexthop_teardown(bctxt);
            sample_if_teardown(bctxt);
            sample_nexthop_group_teardown(bctxt);
            sample_device_teardown(bctxt);
        }
        sample_tep_teardown(bctxt);
        sample_route_table_teardown(bctxt);
        sample1_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};


/// \defgroup SUBNET_TEST Subnet Tests
/// @{


//---------------------------------------------------------------------
// Templatized test cases
//---------------------------------------------------------------------

/// \brief Subnet WF_B1
/// \ref WF_B1
TEST_F(subnet, subnet_workflow_b1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01");
    workflow_b1<subnet_feeder>(feeder);
}

/// \brief Subnet WF_B2
/// \ref WF_B2
TEST_F(subnet, subnet_workflow_b2) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01");
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16", "00:02:0A:00:00:01");
    workflow_b2<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_1
/// \ref WF_1
TEST_F(subnet, subnet_workflow_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_subnet);
    workflow_1<subnet_feeder>(feeder);
}

/// \brief Subnet WF_2
/// \ref WF_2
TEST_F(subnet, subnet_workflow_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_subnet);
    workflow_2<subnet_feeder>(feeder);
}

/// \brief Subnet WF_3
/// \ref WF_3
TEST_F(subnet, subnet_workflow_3) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40),
                     key3 = int2pdsobjkey(70);
    subnet_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01", 20);
    feeder2.init(key2, k_vpc_key, "30.0.0.0/16", "00:02:0A:00:00:01", 20);
    feeder3.init(key3, k_vpc_key, "60.0.0.0/16", "00:02:0A:0B:00:01", 20);
    workflow_3<subnet_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Subnet WF_4
/// \ref WF_4
TEST_F(subnet, subnet_workflow_4) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder = {};

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_subnet);
    workflow_4<subnet_feeder>(feeder);
}

/// \brief Subnet WF_5
/// \ref WF_5
TEST_F(subnet, subnet_workflow_5) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40),
                     key3 = int2pdsobjkey(70);
    subnet_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01", 20);
    feeder2.init(key2, k_vpc_key, "30.0.0.0/16", "00:02:0A:00:00:01", 20);
    feeder3.init(key3, k_vpc_key, "60.0.0.0/16", "00:02:0A:0B:00:01", 20);
    workflow_5<subnet_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Subnet WF_6
/// \ref WF_6
TEST_F(subnet, subnet_workflow_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01",  k_max_subnet);
    feeder1B.init(key, k_vpc_key, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", k_max_subnet);
    workflow_6<subnet_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Subnet WF_7
/// \ref WF_7
TEST_F(subnet, subnet_workflow_7) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_subnet);
    feeder1B.init(key, k_vpc_key, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", k_max_subnet);
    workflow_7<subnet_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Subnet WF_8
/// \ref WF_8
TEST_F(subnet, subnet_workflow_8) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_subnet);
    feeder1B.init(key, k_vpc_key, "12.0.0.0/16",
                  "00:02:0A:0B:00:01", k_max_subnet);
    workflow_8<subnet_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Subnet WF_9
/// \ref WF_9
TEST_F(subnet, subnet_workflow_9) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_subnet);
    workflow_9<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_10
/// \ref WF_10
TEST_F(subnet, subnet_workflow_10) {
    if (!apulu()) return;

    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40),
                     key3 = int2pdsobjkey(70), key4 = int2pdsobjkey(100);
    subnet_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, k_vpc_key, "40.0.0.0/16",
                 "00:02:01:00:00:02", 20);
    feeder2A.init(key2, k_vpc_key, "140.0.0.0/16",
                  "00:02:01:00:00:03", 20);
    feeder3.init(key3, k_vpc_key, "70.0.0.0/16",
                 "00:02:01:00:00:0A", 20);
    feeder3A.init(key3, k_vpc_key, "170.0.0.0/16",
                  "00:02:01:00:00:0B", 20);
    feeder4.init(key4, k_vpc_key, "100.0.0.0/16",
                 "00:02:01:00:00:0C", 20);
    workflow_10<subnet_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief Subnet WF_N_1
/// \ref WF_N_1
TEST_F(subnet, subnet_workflow_neg_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_subnet);
    workflow_neg_1<subnet_feeder>(feeder);
}

/// \brief Subnet WF_N_2
/// \ref WF_N_2
TEST_F(subnet, DISABLED_subnet_workflow_neg_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_subnet + 1);
    workflow_neg_2<subnet_feeder>(feeder);
}

/// \brief Subnet WF_N_3
/// \ref WF_N_3
TEST_F(subnet, subnet_workflow_neg_3) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", k_max_subnet);
    workflow_neg_3<subnet_feeder>(feeder);
}

/// \brief Subnet WF_N_4
/// \ref WF_N_4
TEST_F(subnet, subnet_workflow_neg_4) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    subnet_feeder feeder1, feeder2;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, k_vpc_key, "40.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    workflow_neg_4<subnet_feeder>(feeder1, feeder2);
}

/// \brief Subnet WF_N_5
/// \ref WF_N_5
TEST_F(subnet, subnet_workflow_neg_5) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_subnet);
    workflow_neg_5<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_N_6
/// \ref WF_N_6
TEST_F(subnet, DISABLED_subnet_workflow_neg_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    subnet_feeder feeder1, feeder1A;

    feeder1.init(key, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", k_max_subnet);
    feeder1A.init(key, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01", k_max_subnet + 1);
    workflow_neg_6<subnet_feeder>(feeder1, feeder1A);
}

/// \brief Subnet WF_N_7
/// \ref WF_N_7
TEST_F(subnet, subnet_workflow_neg_7) {
    if (!apulu()) return;

    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    subnet_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder1A.init(key1, k_vpc_key, "11.0.0.0/16",
                  "00:02:0A:00:00:01", 20);
    feeder2.init(key2, k_vpc_key, "12.0.0.0/16",
                 "00:02:0A:0B:00:01", 20);
    workflow_neg_7<subnet_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Subnet WF_N_8
/// \ref WF_N_8
TEST_F(subnet, subnet_workflow_neg_8) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    subnet_feeder feeder1, feeder2;

    feeder1.init(key1, k_vpc_key, "10.0.0.0/16",
                 "00:02:01:00:00:01", 20);
    feeder2.init(key2, k_vpc_key, "11.0.0.0/16",
                 "00:02:0A:00:00:01", 20);
    workflow_neg_8<subnet_feeder>(feeder1, feeder2);
}

//---------------------------------------------------------------------
// Non templatized test cases
//---------------------------------------------------------------------

/// \brief update vpc in subnet should be rejected
TEST_F(subnet, subnet_update_vpc) {
    if (!apulu()) return;

    subnet_feeder feeder;
    pds_subnet_spec_t spec = {0};
    pds_obj_key_t key = int2pdsobjkey(1);

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01");
    subnet_create(feeder);

    // trigger
    spec.vpc = int2pdsobjkey(2);
    subnet_update(feeder, &spec, SUBNET_ATTR_VPC, SDK_RET_ERR);

    // validate
    feeder.init(key, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01");
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update of vr ip inside subnet should be rejected
TEST_F(subnet, subnet_update_vr_ip) {
    if (!apulu()) return;

    subnet_feeder feeder;
    pds_subnet_spec_t spec = {0};
    pds_obj_key_t key = int2pdsobjkey(1);

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01");
    subnet_create(feeder);

    // trigger
    spec.v4_vr_ip = feeder.spec.v4_vr_ip + 1;
    subnet_update(feeder, &spec, SUBNET_ATTR_V4_VRIP, SDK_RET_ERR);

    // validate
    feeder.init(key, k_vpc_key, "10.0.0.0/16", "00:02:01:00:00:01");
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update of vr mac inside subnet
TEST_F(subnet, subnet_update_vr_mac) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    mac = MAC_TO_UINT64(feeder.spec.vr_mac);
    mac++;
    MAC_UINT64_TO_ADDR(spec.vr_mac, mac);
    subnet_update(feeder, &spec, SUBNET_ATTR_VR_MAC);

    // validate
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update of routetable in the subnet
TEST_F(subnet, subnet_update_route_table) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    subnet_spec_route_table_fill(&spec,
                                 pdsobjkey2int(feeder.spec.v4_route_table) + 1);
    subnet_update(feeder, &spec, SUBNET_ATTR_V4_RTTBL);

    // validate
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update of hostif in the subnet
TEST_F(subnet, DISABLED_subnet_update_hostif) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    spec.host_if = int2pdsobjkey(pdsobjkey2int(feeder.spec.host_if) + 1);
    subnet_update(feeder, &spec, SUBNET_ATTR_HOST_IF);

    // validate
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update of dhcp policy in the subnet
TEST_F(subnet, subnet_update_dhcp_policy) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    spec.num_dhcp_policy = 1;
    spec.dhcp_policy[0] =
        int2pdsobjkey(pdsobjkey2int(feeder.spec.dhcp_policy[0]) + 1);
    subnet_update(feeder, &spec, SUBNET_ATTR_DHCP_POL);

    // validate
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update of tos in the subnet
TEST_F(subnet, subnet_update_tos) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    spec.tos = feeder.spec.tos + 1;
    subnet_update(feeder, &spec, SUBNET_ATTR_TOS);

    // validate
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update fabric encap type
TEST_F(subnet, subnet_update_fab_encap_type) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    spec.fabric_encap = feeder.spec.fabric_encap;
    if (spec.fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        spec.fabric_encap.val.vnid = 1;
    } else if (spec.fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        spec.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        spec.fabric_encap.val.mpls_tag = 1;
    }
    subnet_update(feeder, &spec, SUBNET_ATTR_FAB_ENCAP, SDK_RET_ERR);

    // validate
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update fabric encap val
TEST_F(subnet, subnet_update_fab_encap_val) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;
    uint64_t mac;

    // init
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    // trigger
    spec.fabric_encap = feeder.spec.fabric_encap;
    if (spec.fabric_encap.type == PDS_ENCAP_TYPE_MPLSoUDP) {
        spec.fabric_encap.val.mpls_tag++;
    } else if (spec.fabric_encap.type == PDS_ENCAP_TYPE_VXLAN) {
        spec.fabric_encap.val.vnid++;
    }
    subnet_update(feeder, &spec, SUBNET_ATTR_FAB_ENCAP, SDK_RET_ERR);

    // validate
    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_read(feeder);

    // cleanup
    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach one policy P1 to subnet S1 and then update
///        S1 by adding P2 to S1 (so S1 ends up with P1 and P2)
TEST_F(subnet, subnet_update_policy1) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 1;
    uint8_t start_pol_index = 0;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);

    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    num_policies = 2;
    start_pol_index = 0;
    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder);

    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach policy P1 to subnet and then update S1
///        with no policy
TEST_F(subnet, subnet_update_policy2) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 1;
    uint8_t start_pol_index = 0;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", 1, 1);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    num_policies = 0;
    start_pol_index = 0;
    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder, SDK_RET_OK);

    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach policy P1, P2 to subnet and update S1
///        with P1 only (disassociating P2) and then with no policies
TEST_F(subnet, subnet_update_policy3) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", 1, 1);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder);

    num_policies = 1;
    start_pol_index = 0;
    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder);

    memset(&spec, 0, sizeof(spec));
    num_policies = 0;
    start_pol_index = 0;
    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder);

    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - Attach policy P1, P2 to subnet and update S1
///        with P3, P4
TEST_F(subnet, subnet_update_policy4) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    num_policies = 2;
    start_pol_index = 2;
    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder);

    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update policy - attach policy P1, P2 to subnet and update S1
///        with P2, P3
TEST_F(subnet, subnet_update_policy5) {
    if (!apulu()) return;

    sdk_ret_t ret;
    pds_obj_key_t key = int2pdsobjkey(10);
    pds_subnet_spec_t spec = {0};
    subnet_feeder feeder;
    pds_batch_ctxt_t bctxt = batch_start();
    uint8_t num_policies = 2;
    uint8_t start_pol_index = 0;

    feeder.init(key, k_vpc_key, "10.0.0.0/16",
                "00:02:01:00:00:01", num_policies, start_pol_index);
    subnet_create(feeder);
    subnet_read(feeder, SDK_RET_OK);

    num_policies = 2;
    start_pol_index = 1;
    subnet_spec_policy_fill(&spec, num_policies, start_pol_index);
    subnet_update(feeder, &spec, SUBNET_ATTR_POL);
    subnet_read(feeder);

    subnet_delete(feeder);
    subnet_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
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
