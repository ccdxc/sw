#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/plugins/cfg/l4lb/l4lb.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/session.pb.h"
#include "gen/proto/l4lb.pb.h"
#include "gen/proto/nw.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/plugins/cfg/mcast/multicast.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/test/hal_calls/hal_calls.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"

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
using endpoint::EndpointSpec;
using endpoint::EndpointResponse;
using endpoint::EndpointUpdateRequest;
using session::SessionSpec;
using session::SessionResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;
using l4lb::L4LbServiceSpec;
using l4lb::L4LbServiceResponse;
using multicast::MulticastEntrySpec;
using multicast::MulticastEntryResponse;
using multicast::MulticastEntryDeleteRequest;
using multicast::MulticastEntryDeleteResponse;


class ep_mgmt_merge_classic_test : public hal_base_test {
protected:
  ep_mgmt_merge_classic_test() {
  }

  virtual ~ep_mgmt_merge_classic_test() {
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
  }
};
#if 0
hal_ret_t
create_uplink(uint32_t if_id, uint32_t port, 
              uint32_t native_l2seg = 0, bool is_oob = false)
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;

    spec.set_type(intf::IF_TYPE_UPLINK);

    spec.mutable_key_or_handle()->set_interface_id(if_id);
    spec.mutable_if_uplink_info()->set_port_num(port);
    spec.mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg);
    spec.mutable_if_uplink_info()->set_is_oob_management(is_oob);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
update_uplink(uint32_t if_id, uint32_t port, 
              uint32_t native_l2seg = 0, bool is_oob = false)
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;

    spec.set_type(intf::IF_TYPE_UPLINK);

    spec.mutable_key_or_handle()->set_interface_id(if_id);
    spec.mutable_if_uplink_info()->set_port_num(port);
    spec.mutable_if_uplink_info()->set_native_l2segment_id(native_l2seg);
    spec.mutable_if_uplink_info()->set_is_oob_management(is_oob);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(spec, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
create_vrf(uint32_t vrf_id, types::VrfType type, uint32_t des_if_id)
{
    hal_ret_t ret;
    VrfSpec spec;
    VrfResponse rsp;

    spec.mutable_key_or_handle()->set_vrf_id(vrf_id);
    spec.set_vrf_type(type);
    spec.mutable_designated_uplink()->set_interface_id(des_if_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(spec, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
update_l2seg(uint32_t vrf_id, uint32_t l2seg_id, uint32_t encap,
             uint32_t up_ifid[], uint32_t ifid_count, 
             l2segment::MulticastFwdPolicy mcast_pol,
             l2segment::BroadcastFwdPolicy bcast_pol, 
             bool is_swm,
             bool create)
{
    hal_ret_t           ret;
    L2SegmentSpec       spec;
    L2SegmentResponse   rsp;

    spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    spec.mutable_key_or_handle()->set_segment_id(l2seg_id);
    spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    spec.mutable_wire_encap()->set_encap_value(encap);
    spec.set_mcast_fwd_policy(mcast_pol);
    spec.set_bcast_fwd_policy(bcast_pol);
    spec.set_single_wire_management(is_swm);
    for (int i = 0; i < ifid_count; i++) {
        spec.add_if_key_handle()->set_interface_id(up_ifid[i]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    if (create) {
        ret = hal::l2segment_create(spec, &rsp);
    } else {
        ret = hal::l2segment_update(spec, &rsp);
    }
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
delete_l2seg (uint32_t vrf_id, uint32_t l2seg_id)
{
    hal_ret_t ret;
    L2SegmentDeleteRequest  l2seg_del_req;
    L2SegmentDeleteResponse l2seg_del_rsp;

    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    l2seg_del_req.mutable_key_or_handle()->set_segment_id(l2seg_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
create_lif(uint32_t lif_id, uint32_t if_id, types::LifType type, string name,
           bool is_oob = false, bool is_int = false, bool recv_bc = false, 
           bool recv_allmc = false, bool recv_prom = false, bool vlan_strip = false, 
           bool vlan_ins = false)
{
    hal_ret_t ret;
    LifSpec spec;
    LifResponse rsp;

    spec.mutable_key_or_handle()->set_lif_id(lif_id);
    spec.set_type(type);
    spec.set_hw_lif_id(lif_id);
    spec.mutable_pinned_uplink_if_key_handle()->set_interface_id(if_id);
    spec.set_name(name);
    spec.mutable_packet_filter()->set_receive_broadcast(recv_bc);
    spec.mutable_packet_filter()->set_receive_all_multicast(recv_allmc);
    spec.mutable_packet_filter()->set_receive_promiscuous(recv_prom);
    spec.set_vlan_strip_en(vlan_strip);
    spec.set_vlan_insert_en(vlan_ins);
    spec.set_is_management(is_oob || is_int);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
create_enic(uint32_t if_id, uint32_t lif_id, intf::IfEnicType type,
            uint32_t l2seg_id, uint32_t encap, // smart enic fields
            uint32_t native_l2seg_id) // classic fields
{
    hal_ret_t           ret;
    InterfaceSpec       enicif_spec;
    InterfaceResponse   enicif_rsp;

    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(lif_id);
    enicif_spec.mutable_key_or_handle()->set_interface_id(if_id);
    enicif_spec.mutable_if_enic_info()->set_enic_type(type);
    if (type == intf::IF_ENIC_TYPE_CLASSIC) {
        enicif_spec.mutable_if_enic_info()->mutable_classic_enic_info()->
            set_native_l2segment_id(native_l2seg_id);
    } else {
        enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
        enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(encap);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
delete_enic (uint32_t if_id)
{
    hal_ret_t                ret;
    InterfaceDeleteRequest   req;
    InterfaceDeleteResponse  rsp;

    req.mutable_key_or_handle()->set_interface_id(if_id);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(req, &rsp);
    hal::hal_cfg_db_close();

    return ret;
}

hal_ret_t
create_ep(uint32_t vrf_id, uint32_t l2seg_id, uint32_t if_id, uint64_t mac)
{
    hal_ret_t ret;
    EndpointSpec             ep_spec;
    EndpointResponse         ep_rsp;

    ep_spec.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(if_id);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
delete_ep (uint32_t vrf_id, uint32_t l2seg_id, uint64_t mac)
{
    hal_ret_t ret;
    EndpointDeleteRequest       ep_del_req;
    EndpointDeleteResponse      ep_del_rsp;

    ep_del_req.mutable_vrf_key_handle()->set_vrf_id(vrf_id);
    ep_del_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    ep_del_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(mac);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_delete(ep_del_req, &ep_del_rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
create_mcast (uint32_t l2seg_id, uint64_t mac, uint32_t ifid[], uint32_t ifid_count)
{
    hal_ret_t              ret;
    MulticastEntrySpec     spec;
    MulticastEntryResponse rsp;

    spec.mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    for (int i = 0; i < ifid_count; i++) {
        spec.add_oif_key_handles()->set_interface_id(ifid[i]);
    }
    spec.mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::multicastentry_create(spec, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
delete_mcast (uint32_t l2seg_id, uint64_t mac)
{
    hal_ret_t ret;
    MulticastEntryDeleteRequest  req;
    MulticastEntryDeleteResponse rsp;

    req.mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(l2seg_id);
    req.mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(mac);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::multicastentry_delete(req, &rsp);
    hal::hal_cfg_db_close();
    return ret;
}
#endif


// ----------------------------------------------------------------------------
// Creating a endpoint
// ----------------------------------------------------------------------------
TEST_F(ep_mgmt_merge_classic_test, test1)
{
    uint32_t       test_id = 1;
    uint32_t       uplinkif_id1 = UPLINK_IF_ID_OFFSET + test_id,
                   uplinkif_id2 = uplinkif_id1 + 1, uplinkif_id3 = uplinkif_id1 + 2;
    uint32_t       up_ifid[2] = {0, 0}, /*ifid[2] = {0, 0},*/ ifid_count = 0;
    uint32_t       vrf_id_cl_up1 = 18, vrf_id_cl_up2 = 19, vrf_id_cl_up3 = 20/*,
                   vrf_id_hp1 = 65*/;
    uint32_t       l2seg_id_cl_up1 = 1, l2seg_id_cl_up2 = 2, l2seg_id_cl_up3 = 3 /*,l2seg_id_hp1 = 103*/;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       inb_lifid1 = 66, inb_lifid2 = 67, host_lifid1 = 68, host_lifid2 = 69, oob_lifid = 70;
    uint32_t       enic_inb1 = 257, enic_inb2 = 258, enic_oob = 259/*, enic_wl1 = 200*/;
    // uint32_t       wl_encap1 = 100;


    // Create uplinks
    ASSERT_EQ(create_uplink(uplinkif_id1, up_port1), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id2, up_port2), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id3, up_port3, 0, true), HAL_RET_OK);

    // Create 3 Classic VRFs
    ASSERT_EQ(create_vrf(vrf_id_cl_up1, types::VRF_TYPE_INBAND_MANAGEMENT, uplinkif_id1), HAL_RET_OK);
    ASSERT_EQ(create_vrf(vrf_id_cl_up2, types::VRF_TYPE_INBAND_MANAGEMENT, uplinkif_id2), HAL_RET_OK);
    ASSERT_EQ(create_vrf(vrf_id_cl_up3, types::VRF_TYPE_OOB_MANAGEMENT, uplinkif_id3), HAL_RET_OK);

    // Create 4 LIFs
    ASSERT_EQ(create_lif(inb_lifid1, uplinkif_id1, types::LIF_TYPE_MNIC_INBAND_MANAGEMENT,
                         "inb_mnic0"), HAL_RET_OK);
    ASSERT_EQ(create_lif(inb_lifid2, uplinkif_id2, types::LIF_TYPE_MNIC_INBAND_MANAGEMENT,
                         "inb_mnic1"), HAL_RET_OK);
    ASSERT_EQ(create_lif(host_lifid1, uplinkif_id1, types::LIF_TYPE_HOST,
                         "eth0"), HAL_RET_OK);
    ASSERT_EQ(create_lif(host_lifid2, uplinkif_id2, types::LIF_TYPE_HOST,
                         "eth1"), HAL_RET_OK);
    ASSERT_EQ(create_lif(oob_lifid, uplinkif_id3, types::LIF_TYPE_MNIC_OOB_MANAGEMENT,
                         "oob_mnic0"), HAL_RET_OK);

    // Create Classic L2segs in each Classic VRF with respective uplink
    up_ifid[0] = uplinkif_id1;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    up_ifid[0] = uplinkif_id2;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up2, l2seg_id_cl_up2, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    up_ifid[0] = uplinkif_id3;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up3, l2seg_id_cl_up3, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);

    // Update uplinks with native l2seg
    ASSERT_EQ(update_uplink(uplinkif_id1, up_port1, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(update_uplink(uplinkif_id2, up_port2, l2seg_id_cl_up2), HAL_RET_OK);
    ASSERT_EQ(update_uplink(uplinkif_id3, up_port3, l2seg_id_cl_up3), HAL_RET_OK);

    // Create Classic ENICs
    ASSERT_EQ(create_enic(enic_inb1, inb_lifid1, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_inb2, inb_lifid2, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up2), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_oob, oob_lifid, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up3), HAL_RET_OK);

    // Create Classic EPs
    ASSERT_EQ(create_ep(vrf_id_cl_up1, l2seg_id_cl_up1, enic_inb1, 0x000010002001), HAL_RET_OK);
    ASSERT_EQ(create_ep(vrf_id_cl_up2, l2seg_id_cl_up2, enic_inb2, 0x000010002002), HAL_RET_OK);
    ASSERT_EQ(create_ep(vrf_id_cl_up3, l2seg_id_cl_up3, enic_oob,  0x000010002003), HAL_RET_OK);

    // Enable SWM on oob l2seg and first uplink
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id3;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           true, false), HAL_RET_OK);
    up_ifid[0] = uplinkif_id3;
    up_ifid[1] = uplinkif_id1;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up3, l2seg_id_cl_up3, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           true, false), HAL_RET_OK);

    ASSERT_EQ(create_ep(vrf_id_cl_up1, l2seg_id_cl_up1, uplinkif_id3,  0x000010002004), HAL_RET_OK);

    // Delete EP
    ASSERT_EQ(delete_ep(vrf_id_cl_up1, l2seg_id_cl_up1, 0x000010002004), HAL_RET_OK);

    // Disable SWM
    up_ifid[0] = uplinkif_id1;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, false), HAL_RET_OK);
    up_ifid[0] = uplinkif_id3;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up3, l2seg_id_cl_up3, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, false), HAL_RET_OK);

#if 0
    // Create customer VRF
    ASSERT_EQ(create_vrf(vrf_id_hp1, types::VRF_TYPE_CUSTOMER, 0), HAL_RET_OK);
    // Create customer L2seg with wire encap as 8192 
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id2;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_hp1, l2seg_id_hp1, 8192, up_ifid, ifid_count,
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    // Create Enic
    ASSERT_EQ(create_enic(enic_wl1, host_lifid1, intf::IF_ENIC_TYPE_USEG,
                          l2seg_id_hp1, wl_encap1, 0), HAL_RET_OK);
    // Create WL EP
    ASSERT_EQ(create_ep(vrf_id_hp1, l2seg_id_hp1, enic_wl1, 0x000020002001), HAL_RET_OK);

    // ---------- Negative cases ---------

    // Delete customer VRF's l2seg
    // - delete ep
    ASSERT_EQ(delete_ep(vrf_id_hp1, l2seg_id_hp1, 0x000020002001), HAL_RET_OK);
    // - delete enic
    ASSERT_EQ(delete_enic(enic_wl1), HAL_RET_OK);
    // - delete customer l2seg
    ASSERT_EQ(delete_l2seg(vrf_id_hp1, l2seg_id_hp1), HAL_RET_OK);

    // Add customer VRF's l2seg
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id2;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_hp1, l2seg_id_hp1, 8192, up_ifid, ifid_count,
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);

    // Delete Classic l2seg
    // - delete ep
    ASSERT_EQ(delete_ep(vrf_id_cl_up1, l2seg_id_cl_up1, 0x000010002001), HAL_RET_OK);
    ASSERT_EQ(delete_ep(vrf_id_cl_up2, l2seg_id_cl_up2, 0x000010002002), HAL_RET_OK);
    // - delete enic
    ASSERT_EQ(delete_enic(enic_inb1), HAL_RET_OK);
    ASSERT_EQ(delete_enic(enic_inb2), HAL_RET_OK);
    // - delete classic l2seg
    ASSERT_EQ(delete_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(delete_l2seg(vrf_id_cl_up2, l2seg_id_cl_up2), HAL_RET_OK);

    // Add Classic l2seg
    up_ifid[0] = uplinkif_id1;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    up_ifid[0] = uplinkif_id2;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up2, l2seg_id_cl_up2, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);

    // Delete customer VRF's l2seg
    ASSERT_EQ(delete_l2seg(vrf_id_hp1, l2seg_id_hp1), HAL_RET_OK);

    // Delete Classic l2seg
    ASSERT_EQ(delete_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(delete_l2seg(vrf_id_cl_up2, l2seg_id_cl_up2), HAL_RET_OK);

    // Add customer VRF's l2seg
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id2;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_hp1, l2seg_id_hp1, 8192, up_ifid, ifid_count,
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);

    // Add classic l2seg
    up_ifid[0] = uplinkif_id1;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_up1, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);;
    up_ifid[0] = uplinkif_id2;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up2, l2seg_id_cl_up2, 8192, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);

    // Update uplinks with native l2seg
    ASSERT_EQ(update_uplink(uplinkif_id1, up_port1, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(update_uplink(uplinkif_id2, 2, l2seg_id_cl_up2), HAL_RET_OK);

    // Create Classic ENICs
    ASSERT_EQ(create_enic(enic_inb1, inb_lifid1, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_inb2, inb_lifid2, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up2), HAL_RET_OK);

    // Create Classic EPs
    ASSERT_EQ(create_ep(vrf_id_cl_up1, l2seg_id_cl_up1, enic_inb1, 0x000010002001), HAL_RET_OK);
    ASSERT_EQ(create_ep(vrf_id_cl_up2, l2seg_id_cl_up2, enic_inb2, 0x000010002002), HAL_RET_OK);

    // Create mcast
    ifid[0] = enic_inb1;
    ifid_count = 1;
    ASSERT_EQ(create_mcast(l2seg_id_cl_up1, 0x01005e010101, ifid, ifid_count), HAL_RET_OK);
    ifid[0] = enic_inb2;
    ifid_count = 1;
    ASSERT_EQ(create_mcast(l2seg_id_cl_up2, 0x01005e010102, ifid, ifid_count), HAL_RET_OK);

    // Delete mcast
    ASSERT_EQ(delete_mcast(l2seg_id_cl_up1, 0x01005e010101), HAL_RET_OK);
    ASSERT_EQ(delete_mcast(l2seg_id_cl_up2, 0x01005e010102), HAL_RET_OK);
#endif
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
