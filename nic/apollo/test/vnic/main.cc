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
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/mapping.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vcn.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/subnet.hpp"

const char *g_cfg_file = "hal.json";
int g_batch_epoch = 1;    // running batch epoch
namespace api_test {

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
        pds_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

/// \brief Create a VNIC
/// Create a VNIC and test the following
/// Configure both ipv4 and ipv6 addresses and run the packet test
/// Configure min to max of vlan ids and verify with packet
/// Configure varying range of mpls slots and verify with packet
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_create) {
    pds_batch_params_t batch_params = {0};
    const char *my_ip = "100.1.1.1";
    const char *my_mac = "00:00:64:01:01:01";
    const char *my_gw = "100.1.1.2";
    std::string vcn_cidr = "aaaa:0001:0001::/48";
    std::string sub_cidr = "aaaa:0001:0001:0001::/64";
    pds_vnic_id_t vnic_id = 100;
    uint16_t vlan_tag = 100;
    uint16_t mpls_slot = 200;
    const char *vnic_mac = "00:00:0a:01:01:03";
    const char *vnic_ip = "aaaa:0001:0001:0001::0003";    // Part of the subnet
    const char *sub_vr_ip = "aaaa:0001:0001:0001::0001";
    const char *sub_vr_mac = "00:00:0a:01:01:01";
    std::string sub_rem_cidr = "aaaa:0001:0001:0002::/64";
    const char *sub_rem_gw = "100.0.0.4";    // Part of my-tep subnet
    pds_vcn_id_t vcn_id = 1;
    pds_subnet_id_t sub_id = 1;
    pds_route_table_id_t rt_id = 1;
    pds_route_table_spec_t rt_tbl;
    rule_t *rule;
    struct in_addr ipaddr;

    batch_params.epoch = 1;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    // Create device
    device_util device(my_ip, my_mac, my_gw);
    ASSERT_TRUE(device.create() == SDK_RET_OK);

    // Create VCN
    vcn_util vcn(PDS_VCN_TYPE_TENANT, vcn_id, vcn_cidr);
    ASSERT_TRUE(vcn.create() == SDK_RET_OK);

    // Create remote tep
    tep_util rtep(sub_rem_gw, PDS_TEP_ENCAP_TYPE_VNIC);
    ASSERT_TRUE(rtep.create() == SDK_RET_OK);

    // Create ipv6 route for remote subnet
    rt_tbl.key.id = rt_id;
    rt_tbl.af = IP_AF_IPV6;
    rt_tbl.num_routes = 1;
    rt_tbl.routes = (pds_route_t *)calloc(1, sizeof(pds_route_t));
    ASSERT_TRUE(str2ipv6pfx((char *)sub_rem_cidr.c_str(),
                            &rt_tbl.routes[0].prefix) == SDK_RET_OK);
    rt_tbl.routes[0].nh_ip.af = IP_AF_IPV4;
    inet_aton(sub_rem_gw, &ipaddr);
    rt_tbl.routes[0].nh_ip.addr.v4_addr = ntohl(ipaddr.s_addr);
    rt_tbl.routes[0].nh_type = PDS_NH_TYPE_REMOTE_TEP;
    rt_tbl.routes[0].vcn_id = PDS_VCN_ID_INVALID;
    ASSERT_TRUE(pds_route_table_create(&rt_tbl) == SDK_RET_OK);

    // Configure and ipv4 route.
    // TODO check with Sarat on the need for this
    rt_tbl.key.id = ++rt_id;
    rt_tbl.af = IP_AF_IPV4;
    rt_tbl.num_routes = 1;
    rt_tbl.routes = (pds_route_t *)calloc(1, sizeof(pds_route_t));
    ASSERT_TRUE(str2ipv4pfx((char *)"20.1.0.0/16",
                            &rt_tbl.routes[0].prefix) == SDK_RET_OK);
    rt_tbl.routes[0].nh_ip.af = IP_AF_IPV4;
    inet_aton(sub_rem_gw, &ipaddr);
    rt_tbl.routes[0].nh_ip.addr.v4_addr = ntohl(ipaddr.s_addr);
    rt_tbl.routes[0].nh_type = PDS_NH_TYPE_REMOTE_TEP;
    rt_tbl.routes[0].vcn_id = PDS_VCN_ID_INVALID;
    ASSERT_TRUE(pds_route_table_create(&rt_tbl) == SDK_RET_OK);

    // Create Subnet on the VCN
    subnet_util sub(vcn_id, sub_id, sub_cidr);
    sub.vr_ip = sub_vr_ip;
    sub.vr_mac = sub_vr_mac;
    sub.v6_route_table = 1;
    sub.v4_route_table = 2;
    ASSERT_TRUE(sub.create() == SDK_RET_OK);

    // Create vnic
    vnic_util vnic(vcn_id, vnic_id, sub_id, vnic_mac);
    vnic.vlan_tag = vlan_tag;
    vnic.mpls_slot = mpls_slot;
    vnic.rsc_pool_id = 0;
    //ASSERT_TRUE(vnic.create() == SDK_RET_OK);

    vnic_util vnic1(vcn_id, ++vnic_id, sub_id, vnic_mac);
    vnic1.vlan_tag = ++vlan_tag;
    vnic1.mpls_slot = ++mpls_slot;
    vnic1.rsc_pool_id = 0;
    //ASSERT_TRUE(vnic1.create() == SDK_RET_OK);

    // Completed the configuration
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create many VNICs
/// Create max number of VNICs and test the following
/// Configure varying range of vlan tags & mpls slots and verify with packet
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_many_create) {}

/// \brief Get VNIC
/// Configure the vnic and make-sure the parameters are configured properly
/// by getting the configured values back
// TODO : Enable this after fixing the read issue
//        Failure is because reading of EGRESS_LOCAL_VNIC_INFO_RX table
//        entry returns all 0s
TEST_F(vnic_test, DISABLED_vnic_get) {
    pds_vnic_info_t vnic_info;
    pds_vcn_id_t vcn_id = 1;
    pds_vnic_id_t vnic_id = 100;
    uint16_t vlan_tag = 100;
    uint16_t mpls_slot = 200;
    const char *vnic_mac = "00:00:0a:01:01:03";
    pds_subnet_id_t sub_id = 1;

    //vnic_util vnic(vcn_id, vnic_id, vnic_mac);
    vnic_util vnic(vcn_id, vnic_id, sub_id, vnic_mac);
    vnic.vlan_tag = vlan_tag;
    vnic.mpls_slot = mpls_slot;
    vnic.rsc_pool_id = 0;
    // Read vnic info and compare the configuration packet count
    ASSERT_TRUE(vnic.read(&vnic_info) == SDK_RET_OK);
    cout << "VNIC - vcn : " << vnic_info.spec.vcn.id << ", subnet : " <<
        vnic_info.spec.subnet.id << ", vnic : " << vnic_info.spec.key.id <<
        ", vlan : " << vnic_info.spec.wire_vlan << ", fabric_encap : " <<
        vnic_info.spec.fabric_encap.val.mpls_tag << ", mac : " <<
        vnic_info.spec.mac_addr << ", pool : " << vnic_info.spec.rsc_pool_id <<
        ", src_dst : " << vnic_info.spec.src_dst_check;
    memset(&vnic_info, 0, sizeof(vnic_info));
    vnic_util vnic1(vcn_id, ++vnic_id);
    ASSERT_TRUE(vnic.read(&vnic_info) == SDK_RET_OK);
    cout << "VNIC - vcn : " << vnic_info.spec.vcn.id << ", subnet : " <<
        vnic_info.spec.subnet.id << ", vnic : " << vnic_info.spec.key.id <<
        ", vlan : " << vnic_info.spec.wire_vlan << ", fabric_encap : " <<
        vnic_info.spec.fabric_encap.val.mpls_tag << ", mac : " <<
        vnic_info.spec.mac_addr << ", pool : " << vnic_info.spec.rsc_pool_id <<
        ", src_dst : " << vnic_info.spec.src_dst_check;
}

/// \brief VNIC source and destination check
/// Configure skip_src_dst_check to true/false and make sure the config by
/// sending both valid and invalid IP addresses.
/// Get the stats and makesure the drop count/reason are as expected
TEST_F(vnic_test, vnic_src_dst_check) {}


/// \brief Delete VNIC
/// Configure the vnic and make-sure the parameters are configured properly
/// by getting the configured values back
// TODO : Enable this after fixing the delete issue
//        Failure is because reading of EGRESS_LOCAL_VNIC_INFO_RX table
//        entry returns all 0s
TEST_F(vnic_test, DISABLED_vnic_delete) {
    pds_batch_params_t batch_params = {0};
    pds_vcn_id_t vcn_id = 1;
    pds_vnic_id_t vnic_id = 100;

    batch_params.epoch = 2;
    vnic_util vnic(vcn_id, vnic_id);
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(vnic.del() == SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create and delete VNICs in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
TEST_F(vnic_test, vnic_workflow_1) {
    // [ Create SetMax, Delete SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t min_key = {};

    key.id = 1;
    min_key.id = 1;
    min_key.vlan_tag = 1;
    min_key.mpls_slot = 1;
    min_key.mac_u64 = 0xa010101000000000;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    sleep(2);

    vnic_util vnic_obj(min_key.id, min_key.vlan_tag, min_key.mpls_slot,
                       min_key.mac_u64);
    vnic_obj.read(&info, FALSE);
    ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND)
                == sdk::SDK_RET_OK);
}

/// \brief Create, delete and re-create max VNICs in the same batch
/// create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
TEST_F(vnic_test, DISABLED_vnic_workflow_2) {
    // [ Create SetMax - Delete SetMax - Create SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t min_key = {};

    key.id = 1;
    min_key.id = 1;
    min_key.vlan_tag = 1;
    min_key.mpls_slot = 1;
    min_key.mac_u64 = 0xa010104000000000;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    sleep(2);

    vnic_util vnic_obj(min_key.id, min_key.vlan_tag, min_key.mpls_slot,
                       min_key.mac_u64);
    ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
                == sdk::SDK_RET_OK);

    // Cleanup
#if 0
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_ENTRY_NOT_FOUND);
#endif
}

/// \brief Create two sets of VNICs viz set1 and set2. Delete set1.
/// Create a new set - set3. Try to read a VNIC from each set.
TEST_F(vnic_test, vnic_workflow_3) {
    // [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 20;
    vnic_stepper_seed_t min_key1 = {};
    vnic_stepper_seed_t min_key2 = {};
    vnic_stepper_seed_t min_key3 = {};

    key.id = 1;
    min_key1.id = 10;
    min_key1.vlan_tag = 10;
    min_key1.mpls_slot = 10;
    min_key1.mac_u64 = 0xa010101000000000;
    min_key2.id = 40;
    min_key2.vlan_tag = 40;
    min_key2.mpls_slot = 40;
    min_key2.mac_u64 = 0xa010104000000000;
    min_key3.id = 70;
    min_key3.vlan_tag = 70;
    min_key3.mpls_slot = 70;
    min_key3.mac_u64 = 0xa010107000000000;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(min_key2, num_vnics) == sdk::SDK_RET_OK);
    key.id = 10;
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(min_key3, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    vnic_util vnic_obj(min_key1.id, min_key1.vlan_tag, min_key1.mpls_slot,
                       min_key1.mac_u64);
    //ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    key.id = 40;
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    key.id = 70;
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete VNICs in two batches
/// The hardware should create VNIC correctly
/// and return entry not found after delete
TEST_F(vnic_test, DISABLED_vnic_workflow_4) {
    // [ Create SetMax ] - Read - [ Delete SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t min_key = {};

    key.id = 1;
    min_key.id = 1;
    min_key.vlan_tag = 1;
    min_key.mpls_slot = 1;
    min_key.mac_u64 = 0xa010101000000000;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    vnic_util vnic_obj(min_key.id, min_key.vlan_tag, min_key.mpls_slot,
                       min_key.mac_u64);
    ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK) ==
                sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(key, num_vnics,
                           sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete VNICs in two batches
/// The hardware should create VNICs correctly
/// and return entry not found after delete
TEST_F(vnic_test, vnic_workflow_5) {
    // [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 20;
    vnic_stepper_seed_t min_key1 = {};
    vnic_stepper_seed_t min_key2 = {};
    vnic_stepper_seed_t min_key3 = {};

    key.id = 1;
    min_key1.id = 10;
    min_key1.vlan_tag = 10;
    min_key1.mpls_slot = 10;
    min_key1.mac_u64 = 0xa010101000000000;
    min_key2.id = 40;
    min_key2.vlan_tag = 40;
    min_key2.mpls_slot = 40;
    min_key2.mac_u64 = 0xa010104000000000;
    min_key3.id = 70;
    min_key3.vlan_tag = 70;
    min_key3.mpls_slot = 70;
    min_key3.mac_u64 = 0xa010107000000000;

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(min_key2, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    vnic_util vnic_obj(min_key1.id, min_key1.vlan_tag, min_key1.mpls_slot,
                       min_key1.mac_u64);
    //ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    key.id = 10;
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(min_key3, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    //ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    key.id = 40;
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    key.id = 70;
    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of VNICs in two batches
/// The hardware should program VNICs correctly in case of
/// first create and return error in second create operation
TEST_F(vnic_test, vnic_workflow_neg_1) {
    // [ Create SetMax ] - [ Create SetMax ] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t min_key = {};

    key.id = 1;
    min_key.id = 1;
    min_key.vlan_tag = 1;
    min_key.mpls_slot = 1;
    min_key.mac_u64 = 0xa010101000000000;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    vnic_util vnic_obj(min_key.id, min_key.vlan_tag, min_key.mpls_slot,
                       min_key.mac_u64);
    ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK) ==
    //                                   sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics,
    //                       sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
#if 0
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(key, num_vnics,
                           sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
#endif
}

/// \brief Create more than maximum number of VNICs supported.
TEST_F(vnic_test, vnic_workflow_neg_2) {
    // [ Create SetMax+1] - Read
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1025;
    vnic_stepper_seed_t min_key = {};

    key.id = 1;
    min_key.id = 1;
    min_key.vlan_tag = 1;
    min_key.mpls_slot = 1;
    min_key.mac_u64 = 0xa010101000000000;
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_create(min_key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    vnic_util vnic_obj(min_key.id, min_key.vlan_tag, min_key.mpls_slot,
                       min_key.mac_u64);
    vnic_obj.read(&info, FALSE);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing VNIC should return entry not found.
TEST_F(vnic_test, vnic_workflow_neg_3a) {
    // Read NonEx
    // [ Create SetMax, Delete SetMax ] - Read
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1024;

    key.id = 1;

    vnic_util vnic_obj(1, 0, 0, 1);
    vnic_obj.read(&info, FALSE);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics, sdk::SDK_RET_OK)
    //            == sdk::SDK_RET_OK);
}

/// \brief Deletion of a non-existing VNICs should fail.
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_3b) {
    // [Delete NonEx]
    pds_batch_params_t batch_params = {0};
    pds_vnic_key_t key = {};
    pds_vnic_info_t info = {};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t min_key = {};

    key.id = 1;
    min_key.id = 1;
    min_key.vlan_tag = 1;
    min_key.mpls_slot = 1;
    min_key.mac_u64 = 0xa010101000000000;
    batch_params.epoch = ++g_batch_epoch;

    vnic_util vnic_obj(min_key.id, min_key.vlan_tag, min_key.mpls_slot,
                       min_key.mac_u64);
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);

    //ASSERT_TRUE(vnic_util::many_delete(key, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    //ASSERT_TRUE(vnic_obj.read(&info) == sdk::SDK_RET_OK);
    //ASSERT_TRUE(vnic_util::many_read(key, num_vnics,
    //                       sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

}

/// \brief Create a VNICs with an id which is not within the range.
TEST_F(vnic_test, DISABLED_vnic_workflow_corner_case_4) {}

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
            g_cfg_file = optarg;
            if (!g_cfg_file) {
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
