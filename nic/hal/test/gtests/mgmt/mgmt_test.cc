// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/nic.pb.h"
#include "nic/hal/plugins/cfg/nw/nic.hpp"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

using google::protobuf::uint64;

class mgmt_test : public hal_base_test {
protected:
  mgmt_test() {
  }

  virtual ~mgmt_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase();
    hal_test_utils_slab_disable_delete();
  }

};

#define LIF_ID_OOB_MNIC 1
#define LIF_ID_INTERNAL_MNIC_1 2
#define LIF_ID_INTERNAL_MNIC_2 3
#define LIF_ID_MANAGEMENT_NIC_1 4
#define LIF_ID_MANAGEMENT_NIC_2 5

#define IF_ID_UPLINK UPLINK_IF_ID_OFFSET + 1
#define IF_ID_OOB_MNIC_ENIC 2
#define IF_ID_INTERNAL_MNIC_ENIC_1 3
#define IF_ID_INTERNAL_MNIC_ENIC_2 4
#define IF_ID_MANAGEMENT_NIC_ENIC_1 5
#define IF_ID_MANAGEMENT_NIC_ENIC_2 6

#define VRF_ID_MGMT 1

#define L2SEG_ID_OOB 2
#define L2SEG_ID_MANAGEMENT_NIC_1 3
#define L2SEG_ID_MANAGEMENT_NIC_2 4

// ----------------------------------------------------------------------------
// Setup mgmt network in Classic mode
// ----------------------------------------------------------------------------
TEST_F(mgmt_test, test1)
{
    LifSpec             spec;
    LifResponse         rsp;
    InterfaceSpec       if_spec;
    InterfaceResponse   if_rsp;
    VrfSpec             vrf_spec;
    VrfResponse         vrf_rsp;
    L2SegmentSpec       l2seg_spec;
    L2SegmentResponse   l2seg_rsp;
    EndpointSpec        ep_spec;
    EndpointResponse    ep_rsp;
    hal_ret_t           ret = HAL_RET_OK;

    hal::g_hal_state->set_forwarding_mode(sdk::lib::FORWARDING_MODE_CLASSIC);

    // Create 2 MNIC LIFs, Management LIFs
    // Create OOB MNIC LIF
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_OOB_MNIC);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create internal MNIC LIFs
    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_1);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_2);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Management LIFs
    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_1);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_2);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create OOB uplink
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_UPLINK);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    if_spec.mutable_if_uplink_info()->set_is_oob_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update OOB MNIC lif with pinned uplink
    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_OOB_MNIC);
    spec.set_is_management(true);
    spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(IF_ID_UPLINK);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create ENICs of type classic for MNIC LIFs and Management LIFs
    // Create ENIC on OOB MNIC
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_OOB_MNIC);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OOB_MNIC_ENIC);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create ENIC on internal MNICs
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_1);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_1);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_2);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_2);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create ENICs on Management NICs
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_1);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_1);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_2);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_2);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create VRF
    vrf_spec.mutable_key_or_handle()->set_vrf_id(VRF_ID_MGMT);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create OOB l2seg and multiple internal mgmt l2segs
    // Create OOB l2seg
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_OOB);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segs for Management NICs
    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add OOB l2seg for OOB MNIC.
    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_OOB);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    l2seg_spec.add_if_key_handle()->set_interface_id(IF_ID_UPLINK);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Make OOB l2seg as native on OOB uplink
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_UPLINK);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    if_spec.mutable_if_uplink_info()->set_native_l2segment_id(L2SEG_ID_OOB);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add one l2seg on each of the Mangement Enics.
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_1);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_1);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_2);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_2);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add all internal l2segs on internal management MNIC Enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_1);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_1);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_2);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_2);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create EPs on each of the management NICs and 2 of the MNICs.
    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_1);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(LIF_ID_INTERNAL_MNIC_1);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000001ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(LIF_ID_INTERNAL_MNIC_2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000001ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

#define LIF_ID_OOB_MNIC_HP 21
#define LIF_ID_INTERNAL_MNIC_1_HP 22
#define LIF_ID_INTERNAL_MNIC_2_HP 23
#define LIF_ID_MANAGEMENT_NIC_1_HP 24
#define LIF_ID_MANAGEMENT_NIC_2_HP 25

#define IF_ID_UPLINK_HP 21
#define IF_ID_OOB_MNIC_ENIC_HP 22
#define IF_ID_INTERNAL_MNIC_ENIC_1_HP 23
#define IF_ID_INTERNAL_MNIC_ENIC_2_HP 24
#define IF_ID_MANAGEMENT_NIC_ENIC_1_HP 25
#define IF_ID_MANAGEMENT_NIC_ENIC_2_HP 26

#define VRF_ID_MGMT_HP 21

#define L2SEG_ID_OOB_HP 22
#define L2SEG_ID_MANAGEMENT_NIC_1_HP 23
#define L2SEG_ID_MANAGEMENT_NIC_2_HP 24
// ----------------------------------------------------------------------------
// Setup mgmt network in Host pin mode
// ----------------------------------------------------------------------------
TEST_F(mgmt_test, test2)
{
    LifSpec             spec;
    LifResponse         rsp;
    InterfaceSpec       if_spec;
    InterfaceResponse   if_rsp;
    VrfSpec             vrf_spec;
    VrfResponse         vrf_rsp;
    L2SegmentSpec       l2seg_spec;
    L2SegmentResponse   l2seg_rsp;
    EndpointSpec        ep_spec;
    EndpointResponse    ep_rsp;
    hal_ret_t           ret = HAL_RET_OK;

    hal::g_hal_state->set_forwarding_mode(sdk::lib::FORWARDING_MODE_HOSTPIN);

    // Create 2 MNIC LIFs, Management LIFs
    // Create OOB MNIC LIF
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_OOB_MNIC_HP);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create internal MNIC LIFs
    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_1_HP);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_2_HP);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Management LIFs
    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_1_HP);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_2_HP);
    spec.set_is_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create OOB uplink
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_UPLINK_HP);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    if_spec.mutable_if_uplink_info()->set_is_oob_management(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update OOB MNIC lif with pinned uplink
    spec.Clear();
    spec.mutable_key_or_handle()->set_lif_id(LIF_ID_OOB_MNIC_HP);
    spec.set_is_management(true);
    spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(IF_ID_UPLINK_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create ENICs of type classic for MNIC LIFs and Management LIFs
    // Create ENIC on OOB MNIC
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_OOB_MNIC_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OOB_MNIC_ENIC_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create ENIC on internal MNICs
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_1_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_1_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_2_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_2_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create ENICs on Management NICs
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_1_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_1_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_2_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_2_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create VRF
    vrf_spec.mutable_key_or_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create OOB l2seg and multiple internal mgmt l2segs
    // Create OOB l2seg
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_OOB_HP);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segs for Management NICs
    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1_HP);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2_HP);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add OOB l2seg for OOB MNIC.
    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(L2SEG_ID_OOB_HP);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    l2seg_spec.add_if_key_handle()->set_interface_id(IF_ID_UPLINK_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Make OOB l2seg as native on OOB uplink
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_UPLINK_HP);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    if_spec.mutable_if_uplink_info()->set_native_l2segment_id(L2SEG_ID_OOB_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add one l2seg on each of the Mangement Enics.
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_1_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_1_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_MANAGEMENT_NIC_2_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_2_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add all internal l2segs on internal management MNIC Enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_1_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_1_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(LIF_ID_INTERNAL_MNIC_2_HP);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_INTERNAL_MNIC_ENIC_2_HP);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2_HP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create EPs on each of the management NICs and 2 of the MNICs.
    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1_HP);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_1_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2_HP);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_MANAGEMENT_NIC_ENIC_2_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_1_HP);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(LIF_ID_INTERNAL_MNIC_1_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000001ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec.Clear();
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(VRF_ID_MGMT_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(L2SEG_ID_MANAGEMENT_NIC_2_HP);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(LIF_ID_INTERNAL_MNIC_2_HP);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000001ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
