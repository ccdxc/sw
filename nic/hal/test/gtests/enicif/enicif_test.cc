#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
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
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"

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
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;
using device::DeviceRequest;
using device::DeviceResponseMsg;
using endpoint::EndpointSpec;
using endpoint::EndpointResponse;


class enicif_test : public hal_base_test {
protected:
  enicif_test() {
  }

  virtual ~enicif_test() {
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
// Creating a useg enicif
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test1)
{
    hal_ret_t                   ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               enicif_spec;
    InterfaceResponse           enicif_rsp;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    NetworkSpec                 nw_spec;
    NetworkResponse             nw_rsp;
    InterfaceDeleteRequest      del_req;
    InterfaceDeleteResponse     del_rsp;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;
    slab_stats_t                *pre = NULL, *post = NULL;
    bool                        is_leak = false;
    NetworkKeyHandle                *nkh = NULL;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    pre = hal_test_utils_collect_slab_stats();
    // Create enicif
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 1);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(1);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // update lif
    lif_spec.set_vlan_strip_en(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    // hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vlan insert en
    lif_spec.set_vlan_insert_en(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    // hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}


// ----------------------------------------------------------------------------
// Creating a classic enicif
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test2)
{
    hal_ret_t                   ret;
    VrfSpec                     ten_spec;
    VrfResponse                 ten_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               enicif_spec, upif_spec, enicif_spec1;
    InterfaceResponse           enicif_rsp, upif_rsp, enicif_rsp1;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    NetworkSpec                 nw_spec;
    NetworkResponse             nw_rsp;
    InterfaceDeleteRequest      del_req;
    InterfaceDeleteResponse     del_rsp;
    int                         num_l2segs = 10;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;
    uint64_t                    l2seg_hdls[11] = { 0 };
    // NetworkKeyHandle            *nkh = NULL;
    // slab_stats_t             *pre = NULL, *post = NULL;
    // bool                     is_leak = false;

#if 0
    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(3);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    ten_spec.set_vrf_type(types::VRF_TYPE_INBAND_MANAGEMENT);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();
#endif

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 20);
    upif_spec.mutable_if_uplink_info()->set_port_num(1);
    // upif_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t up_hdl = upif_rsp.mutable_status()->if_handle();

    // Create Uplink If -2
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 21);
    upif_spec.mutable_if_uplink_info()->set_port_num(1);
    // upif_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t up_hdl1 = upif_rsp.mutable_status()->if_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec.set_vrf_type(types::VRF_TYPE_INBAND_MANAGEMENT);
    ten_spec.mutable_designated_uplink()->set_interface_id(UPLINK_IF_ID_OFFSET + 20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(21);
    lif_spec.mutable_packet_filter()->set_receive_broadcast(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();
#endif

    // Create l2segments
    // nkh = l2seg_spec.add_network_key_handle();
    // nkh->set_nw_handle(nw_hdl);
    for (int i = 1; i <= num_l2segs; i++) {
        // Create l2segment
        l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
        l2seg_spec.mutable_key_or_handle()->set_segment_id(200 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(200 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        l2seg_hdls[i] = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();
    }


    // pre = hal_test_utils_collect_slab_stats();

    // Create enicif with wrong enic info
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(21);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create classic enic
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_if_handle(up_hdl);
    auto l2kh = enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_l2segment_handle(l2seg_hdls[1]);
    l2kh = enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_l2segment_handle(l2seg_hdls[2]);
    enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_id(203);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vlan insert en
    lif_spec.set_vlan_insert_en(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    // hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update classic enic - Change uplink
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    enicif_spec1.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_if_handle(up_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(enicif_spec1, &enicif_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update vlan strip en
    lif_spec.set_vlan_strip_en(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    enicif_spec1.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_if_handle(up_hdl);
    l2kh = enicif_spec1.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_l2segment_handle(l2seg_hdls[1]);
    l2kh = enicif_spec1.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_l2segment_handle(l2seg_hdls[2]);
    enicif_spec1.mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_id(203);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update classic enic - Change native l2seg
    // Update classic enic - Change l2seg list

    // Set device mode as Host Pinned
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();



}
// ----------------------------------------------------------------------------
// Creating a enicif with lif having pinned uplink
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test3)
{
    hal_ret_t                   ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               enicif_spec, upif_spec, enicif_spec1;
    InterfaceResponse           enicif_rsp, upif_rsp, enicif_rsp1;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    NetworkSpec                 nw_spec;
    NetworkResponse             nw_rsp;
    InterfaceDeleteRequest      del_req;
    InterfaceDeleteResponseMsg  del_rsp;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;
    int                         num_l2segs = 1;
    NetworkKeyHandle                *nkh = NULL;
    // uint64_t                    l2seg_hdls[10] = { 0 };
    // slab_stats_t                *pre = NULL, *post = NULL;
    // bool                        is_leak = false;

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_SWITCH);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(4);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(3);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 30);
    upif_spec.mutable_if_uplink_info()->set_port_num(1);
    // upif_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t up_hdl = upif_rsp.mutable_status()->if_handle();

    // Create Uplink If -2
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 31);
    upif_spec.mutable_if_uplink_info()->set_port_num(1);
    // upif_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t up_hdl1 = upif_rsp.mutable_status()->if_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(31);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segments
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    for (int i = 1; i <= num_l2segs; i++) {
        // Create l2segment
        l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(3);
        l2seg_spec.mutable_key_or_handle()->set_segment_id(300 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(300 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        // l2seg_hdls[i] = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();
    }


    // pre = hal_test_utils_collect_slab_stats();

    // Create enicif with wrong enic info
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(31);
    enicif_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 31);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_PVLAN);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(301);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


}

// ----------------------------------------------------------------------------
// Test classic enicifs and promiscous mode changes on lif
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test4)
{
    hal_ret_t                   ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               enicif_spec, upif_spec, enicif_spec1;
    InterfaceResponse           enicif_rsp, upif_rsp, enicif_rsp1;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    NetworkSpec                 nw_spec;
    NetworkResponse             nw_rsp;
    DeviceRequest               nic_req;
    DeviceResponseMsg           nic_rsp;
    InterfaceDeleteRequest      del_req;
    InterfaceDeleteResponse     del_rsp;
    int                         num_l2segs = 3;
    uint64_t                    l2seg_hdls[10] = { 0 };
    NetworkKeyHandle                *nkh = NULL;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;
    // slab_stats_t                *pre = NULL, *post = NULL;
    // bool                        is_leak = false;

    hal::g_hal_state->set_forwarding_mode(hal::HAL_FORWARDING_MODE_CLASSIC);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(5);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(4);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create Uplink If
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 40);
    upif_spec.mutable_if_uplink_info()->set_port_num(1);
    // upif_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t up_hdl = upif_rsp.mutable_status()->if_handle();

    // Create Uplink If -2
    upif_spec.set_type(intf::IF_TYPE_UPLINK);
    upif_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 41);
    upif_spec.mutable_if_uplink_info()->set_port_num(1);
    // upif_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(upif_spec, &upif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t up_hdl1 = upif_rsp.mutable_status()->if_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(42);
    lif_spec.mutable_packet_filter()->set_receive_promiscuous(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segments
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    for (int i = 1; i <= num_l2segs; i++) {
        // Create l2segment
        l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(4);
        l2seg_spec.mutable_key_or_handle()->set_segment_id(400 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(400 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        l2seg_hdls[i] = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();
    }

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(401);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close();

    // Create classic enic
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(41);
    enicif_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 41);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    enicif_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_if_handle(up_hdl);
    auto l2kh = enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_l2segment_handle(l2seg_hdls[1]);
    l2kh = enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_l2segment_handle(l2seg_hdls[2]);
    enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->set_native_l2segment_id(403);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(402);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 40);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close();

    // Update lif
    lif_spec.mutable_key_or_handle()->set_lif_id(41);
    lif_spec.mutable_packet_filter()->set_receive_promiscuous(false);
    lif_spec.mutable_packet_filter()->set_receive_broadcast(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Set device mode as Smart switch
    nic_req.mutable_device()->set_device_mode(device::DEVICE_MODE_MANAGED_HOST_PIN);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::device_create(&nic_req, &nic_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Set promiscous to true
    lif_spec.mutable_packet_filter()->set_receive_promiscuous(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

}

// ----------------------------------------------------------------------------
// Create lif with pinned uplink and update pinned uplink
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test5)
{
    hal_ret_t                ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    InterfaceSpec            if_spec;
    InterfaceResponse        if_rsp;
    InterfaceDeleteRequest   del_req;
    InterfaceDeleteResponse  del_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;

    hal::g_hal_state->set_forwarding_mode(hal::HAL_FORWARDING_MODE_CLASSIC);

#if 0
    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 50);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If -2
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 51);
    if_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(5);
    ten_spec.set_vrf_type(types::VRF_TYPE_INBAND_MANAGEMENT);
    ten_spec.mutable_designated_uplink()->set_interface_id(UPLINK_IF_ID_OFFSET + 50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    lif_spec.mutable_packet_filter()->set_receive_promiscuous(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(501);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(501);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment - 2
    l2seg_spec.Clear();
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(502);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(502);
    l2seg_spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(51);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 51);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    auto l2kh = if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_segment_id(501);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update classic enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(51);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 51);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    auto l2kh1 = if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh1->set_segment_id(501);
    auto l2kh2 = if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh2->set_segment_id(502);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // change pinned uplink on lif
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // change pinned uplink on lif
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 51);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update rdma sniff
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 51);
    lif_spec.set_rdma_sniff_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update rdma sniff
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 51);
    lif_spec.set_rdma_sniff_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update rdma sniff
    lif_spec.mutable_key_or_handle()->set_lif_id(51);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 51);
    lif_spec.set_rdma_sniff_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete enicif
    del_req.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 51);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(51);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 51);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    auto l2kh3 = if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh3->set_segment_id(501);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Create lif with pinned uplink and update pinned uplink
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test6)
{
    hal_ret_t                ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    InterfaceSpec            if_spec;
    InterfaceResponse        if_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;

    hal::g_hal_state->set_forwarding_mode(hal::HAL_FORWARDING_MODE_SMART_HOST_PINNED);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(61);
    ten_spec.set_vrf_type(types::VRF_TYPE_INBAND_MANAGEMENT);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 60);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If -2
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 61);
    if_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(61);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 60);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(6);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(601);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(601);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(61);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(602);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(602);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create micro seg enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(61);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 61);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(601);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(60);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(61);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 62);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(601);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0001DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(61);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create classic enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(61);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 63);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_CLASSIC);
    if_spec.mutable_if_enic_info()->mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 60);
    auto l2kh = if_spec.mutable_if_enic_info()->mutable_classic_enic_info()->add_l2segment_key_handle();
    l2kh->set_segment_id(602);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update useg vlan of enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(61);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 61);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(601);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(63);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // change pinned uplink on lif
    lif_spec.mutable_key_or_handle()->set_lif_id(61);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 61);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update lif
    lif_spec.mutable_key_or_handle()->set_lif_id(61);
    lif_spec.mutable_packet_filter()->set_receive_promiscuous(true);
    lif_spec.mutable_packet_filter()->set_receive_broadcast(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Create enic without lif
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test7)
{
    hal_ret_t                ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    InterfaceSpec            if_spec;
    InterfaceResponse        if_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    EndpointSpec             ep_spec;
    EndpointResponse         ep_rsp;
    EndpointUpdateRequest    ep_req;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;

    // hal::g_hal_state->set_forwarding_mode(hal::HAL_FORWARDING_MODE_SMART_HOST_PINNED);

#if 0
    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(7);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 70);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If -2
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 71);
    if_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(7);
    ten_spec.set_vrf_type(types::VRF_TYPE_INBAND_MANAGEMENT);
    ten_spec.mutable_designated_uplink()->set_interface_id(UPLINK_IF_ID_OFFSET + 70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(71);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    lif_spec.mutable_key_or_handle()->set_lif_id(72);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 71);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(7);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(701);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(701);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create micro seg enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    // if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(71);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 71);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(701);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(7);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(701);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_OFFSET + 71);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0001DEADBEEF);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(71);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 71);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(701);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(72);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 71);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(701);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(70);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create EP
    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(7);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(701);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_OFFSET + 71);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0000DEADBEEF);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // update Endpoint
    ep_req.mutable_vrf_key_handle()->set_vrf_id(7);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(701);
    ep_req.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_OFFSET + 71);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0000DEADBEEF);
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    ep_req.mutable_endpoint_attrs()->set_useg_vlan(72);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Create enic without lif and call internal update
// ----------------------------------------------------------------------------
TEST_F(enicif_test, test8)
{
    hal_ret_t                ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    InterfaceSpec            if_spec;
    InterfaceResponse        if_rsp;
    LifSpec                  lif_spec;
    LifResponse              lif_rsp;
    L2SegmentSpec            l2seg_spec;
    L2SegmentResponse        l2seg_rsp;
    EndpointSpec             ep_spec;
    EndpointResponse         ep_rsp;
    hal::if_t                *hal_if, *new_hal_if;
    hal::lif_t               *lif;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;

    // hal::g_hal_state->set_forwarding_mode(hal::HAL_FORWARDING_MODE_SMART_HOST_PINNED);

#if 0
    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(8);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 80);
    if_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink If -2
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 81);
    if_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(8);
    ten_spec.set_vrf_type(types::VRF_TYPE_CUSTOMER);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(81);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 80);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    lif_spec.mutable_key_or_handle()->set_lif_id(82);
    lif_spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 81);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(8);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(801);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(801);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create micro seg enic
    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_ENIC);
    // if_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(71);
    if_spec.mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 81);
    if_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(801);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x0000DEADBEEF);
    if_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(80);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(8);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(801);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_OFFSET + 81);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0001DEADBEEF);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update enic
    hal_if = hal::find_if_by_id(IF_ID_OFFSET + 81);
    lif = hal::find_lif_by_id(81);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::enic_update_lif(hal_if, lif, &new_hal_if);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    hal_if = new_hal_if;
    lif = hal::find_lif_by_id(82);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::enic_update_lif(hal_if, lif, &new_hal_if);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    hal_if = new_hal_if;
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::enic_update_lif(hal_if, NULL, &new_hal_if);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create EP
    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(8);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(801);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(IF_ID_OFFSET + 81);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0000DEADBEEF);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
