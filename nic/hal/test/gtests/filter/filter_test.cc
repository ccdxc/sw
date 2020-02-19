#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/filter.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nic.pb.h"
#include "nic/hal/plugins/cfg/nw/nic.hpp"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using vrf::VrfSpec;
using vrf::VrfResponse;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using kh::LifKeyHandle;
using device::DeviceRequest;
using device::DeviceResponseMsg;


class hal_filter_test : public hal_base_test {
protected:
  hal_filter_test() {
  }

  virtual ~hal_filter_test() {
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

// ----------------------------------------------------------------------------
// MAC VLAN filter
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test1)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(100);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 1);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(1);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(10);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(100);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec.mutable_key_or_handle()->set_interface_id(11);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(1);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(10);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0010DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(100);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(1);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Create enic
// MAC filter Add
// Vlan filter Add - Triggers reprogram of Output Map for enic
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test2)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(200);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(2);
    enicif_spec.mutable_key_or_handle()->set_interface_id(20);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(2);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(200);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0001DEADBEEF;
    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(2);
    enicif_spec.mutable_key_or_handle()->set_interface_id(21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(2);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(210);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(200);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(2);
    enicif_spec.mutable_key_or_handle()->set_interface_id(22);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(2);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(211);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(200);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(2);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create VLAN filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(2);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#if 0
    // -ve case
    // Create VLAN filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(2);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
#endif
}

// ----------------------------------------------------------------------------
// Vlan, MAC filter
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test3)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(3);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(3);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(30);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(300);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0000DEADBEEF;
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(3);
    enicif_spec.mutable_key_or_handle()->set_interface_id(30);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(3);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(30);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(300);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(3);
    enicif_spec.mutable_key_or_handle()->set_interface_id(31);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(3);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(31);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(300);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(3);
    enicif_spec.mutable_key_or_handle()->set_interface_id(32);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(3);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(32);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0001DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(300);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create VLAN filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(3);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(30);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(3);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create MAC filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(3);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(3);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(30);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// MAC VLAN filter
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test4)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;
    FilterDeleteRequest      fdelete_spec;
    FilterDeleteResponse     fdelete_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(4);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(4);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(400);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0000DEADBEEF;
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(4);
    enicif_spec.mutable_key_or_handle()->set_interface_id(401);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(4);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(40);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(400);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(4);
    enicif_spec.mutable_key_or_handle()->set_interface_id(402);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(4);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(40);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(400);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(4);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete filter
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(4);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(4);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Vlan, MAC filter with delete
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test5)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;
    FilterDeleteRequest      fdelete_spec;
    FilterDeleteResponse     fdelete_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(50);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(500);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0000DEADBEEF;
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(5);
    enicif_spec.mutable_key_or_handle()->set_interface_id(501);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(50);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(50);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(500);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(5);
    enicif_spec.mutable_key_or_handle()->set_interface_id(502);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(50);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(51);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(500);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(5);
    enicif_spec.mutable_key_or_handle()->set_interface_id(503);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(50);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(52);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(500);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create VLAN filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create MAC filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    // filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Mac-Vlan filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete Mac-Vlan filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete Vlan filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Vlan filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete MAC filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete MAC filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete Vlan filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Vlan filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(5);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// MAC VLAN filter with enic add after the filter
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test6)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    InterfaceDeleteRequest   edel_req;
    InterfaceDeleteResponse  edel_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(6);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(60);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(60);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(600);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(6);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(60);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0000DEADBEEF;
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(6);
    enicif_spec.mutable_key_or_handle()->set_interface_id(601);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(60);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(60);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(600);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(6);
    enicif_spec.mutable_key_or_handle()->set_interface_id(602);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(60);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(61);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(600);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    edel_req.mutable_key_or_handle()->set_interface_id(601);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(edel_req, &edel_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(6);
    enicif_spec.mutable_key_or_handle()->set_interface_id(601);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(60);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(60);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(600);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Vlan, MAC filter with enic creation after filter with delete
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test7)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    InterfaceDeleteRequest   edel_req;
    InterfaceDeleteResponse  edel_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;
    FilterDeleteRequest      fdelete_spec;
    FilterDeleteResponse     fdelete_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(7);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(7);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(70);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(7);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(700);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create VLAN filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0000DEADBEEF;
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(7);
    enicif_spec.mutable_key_or_handle()->set_interface_id(701);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(700);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(7);
    enicif_spec.mutable_key_or_handle()->set_interface_id(702);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(71);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(700);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(7);
    enicif_spec.mutable_key_or_handle()->set_interface_id(703);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(72);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(700);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete enicif
    edel_req.mutable_key_or_handle()->set_interface_id(701);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(edel_req, &edel_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete Vlan filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(7);
    enicif_spec.mutable_key_or_handle()->set_interface_id(701);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(700);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Vlan filter - Should trigger to set egress_en
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete enicif
    edel_req.mutable_key_or_handle()->set_interface_id(701);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(edel_req, &edel_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete MAC filter
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(7);
    enicif_spec.mutable_key_or_handle()->set_interface_id(701);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(70);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(700);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter - Should trigger to set egress_en
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete MAC filter - Should trigger to unset egress_en
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete Vlan filter - No op
    fdelete_spec.Clear();
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create MAC filter  - No op
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x0000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Vlan filter : Should trigger to set egress_en
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(7);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Lif * * filter
// ----------------------------------------------------------------------------
TEST_F(hal_filter_test, test8)
{
    hal_ret_t                ret;
    DeviceRequest            nic_req;
    DeviceResponseMsg        nic_rsp;
    VrfSpec                  vrf_spec;
    VrfResponse              vrf_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    InterfaceSpec            enicif_spec, upif_spec;
    InterfaceResponse        enicif_rsp, upif_rsp;
    FilterSpec               filter_spec;
    FilterResponse           filter_rsp;
    FilterDeleteRequest      fdelete_spec;
    FilterDeleteResponse     fdelete_rsp;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(8);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(8);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(80);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(80);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(8);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(800);
    upif_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t mac = 0x0000DEADBEEF;
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(8);
    enicif_spec.mutable_key_or_handle()->set_interface_id(801);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(80);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(80);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(800);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(8);
    enicif_spec.mutable_key_or_handle()->set_interface_id(802);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(80);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(81);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(mac++);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(800);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create filter
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(8);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_mac_address(0x000DEADBEEF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(8);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create filter
    filter_spec.Clear();
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(8);
    filter_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_create(filter_spec, &filter_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(8);
    enicif_spec.mutable_key_or_handle()->set_interface_id(803);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(80);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(82);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0003DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(800);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete filter
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(8);
    fdelete_spec.mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::filter_delete(fdelete_spec, &fdelete_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec.Clear();
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(8);
    enicif_spec.mutable_key_or_handle()->set_interface_id(804);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(80);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(83);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0004DEADBEEF);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(800);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
