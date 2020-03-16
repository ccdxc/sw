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


class ep_mgmt_merge_test : public hal_base_test {
protected:
  ep_mgmt_merge_test() {
  }

  virtual ~ep_mgmt_merge_test() {
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

// ----------------------------------------------------------------------------
// Mgmt Vlan merge test case
// ----------------------------------------------------------------------------
TEST_F(ep_mgmt_merge_test, test1)
{
    uint32_t       test_id = 1;
    uint32_t       uplinkif_id1 = UPLINK_IF_ID_OFFSET + test_id,
                   uplinkif_id2 = uplinkif_id1 + 1, uplinkif_id3 = uplinkif_id1 + 2;
    uint32_t       up_ifid[2] = {0, 0}, ifid[2] = {0, 0}, ifid_count = 0;
    uint32_t       vrf_id_cl_up1 = 18, vrf_id_cl_up2 = 19, vrf_id_cl_up3 = 20,
                   vrf_id_hp1 = 65;
    uint32_t       l2seg_id_cl_up1 = 1, l2seg_id_cl_up2 = 2, l2seg_id_hp1 = 103;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       inb_lifid1 = 66, inb_lifid2 = 67, host_lifid1 = 68, host_lifid2 = 69;
    uint32_t       enic_inb1 = 257, enic_inb2 = 258, enic_wl1 = 200;
    uint32_t       wl_encap1 = 100;

    // Create uplinks
    ASSERT_EQ(create_uplink(uplinkif_id1, up_port1), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id2, up_port2), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id3, up_port3, 0, true), HAL_RET_OK);

    // Create 2 Classic VRFs
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

    // Create customer VRF
    ASSERT_EQ(create_vrf(vrf_id_hp1, types::VRF_TYPE_CUSTOMER, 0), HAL_RET_OK);
    ASSERT_EQ(create_vrf(vrf_id_hp1 + 1, types::VRF_TYPE_CUSTOMER, 0), HAL_RET_OK);
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

    // Create Enic
    ASSERT_EQ(create_enic(enic_wl1, host_lifid1, intf::IF_ENIC_TYPE_USEG,
                          l2seg_id_hp1, wl_encap1, 0), HAL_RET_OK);
    // Create WL EP
    ASSERT_EQ(create_ep(vrf_id_hp1, l2seg_id_hp1, enic_wl1, 0x000020002001), HAL_RET_OK);

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
    // - delete ep
    ASSERT_EQ(delete_ep(vrf_id_hp1, l2seg_id_hp1, 0x000020002001), HAL_RET_OK);
    // - delete enic
    ASSERT_EQ(delete_enic(enic_wl1), HAL_RET_OK);
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
}

/* 
 * All three are same
 *   - Customer Vlan
 *   - Inband Vlan
 *   - SWM Vlan
 */
TEST_F(ep_mgmt_merge_test, test2)
{
    uint32_t       test_id = 1;
    uint32_t       uplinkif_id1 = UPLINK_IF_ID_OFFSET + test_id,
                   uplinkif_id2 = uplinkif_id1 + 1, uplinkif_id3 = uplinkif_id1 + 2;
    uint32_t       up_ifid[2] = {0, 0}, /*ifid[2] = {0, 0},*/ ifid_count = 0;
    uint32_t       vrf_id_cl_up1 = 18, vrf_id_cl_up2 = 19, vrf_id_cl_up3 = 20,
                   vrf_id_hp1 = 65;
    uint32_t       l2seg_id_cl_up1 = 1, l2seg_id_cl_up2 = 2, l2seg_id_cl_up3 = 3, l2seg_id_hp1 = 103;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       inb_lifid1 = 66, inb_lifid2 = 67, host_lifid1 = 68, host_lifid2 = 69, oob_lifid = 70;
    uint32_t       enic_inb1 = 257, enic_inb2 = 258, enic_oob = 259, enic_wl1 = 200;
    uint32_t       wl_encap1 = 100;

    // Create uplinks
    ASSERT_EQ(create_uplink(uplinkif_id1, up_port1), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id2, up_port2), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id3, up_port3, 0, true), HAL_RET_OK);

    // Create 2 Classic VRFs
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

/* 
 *   - Customer Vlan
 *   - Inband Vlan is same as SWM Vlan
 */
TEST_F(ep_mgmt_merge_test, test3)
{
    uint32_t       test_id = 1;
    uint32_t       uplinkif_id1 = UPLINK_IF_ID_OFFSET + test_id,
                   uplinkif_id2 = uplinkif_id1 + 1, uplinkif_id3 = uplinkif_id1 + 2;
    uint32_t       up_ifid[2] = {0, 0}, /*ifid[2] = {0, 0},*/ ifid_count = 0;
    uint32_t       vrf_id_cl_up1 = 18, vrf_id_cl_up2 = 19, vrf_id_cl_up3 = 20,
                   vrf_id_hp1 = 65;
    uint32_t       l2seg_id_cl_up1 = 1, l2seg_id_cl_up2 = 2, l2seg_id_cl_up3 = 3, l2seg_id_hp1 = 103;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       inb_lifid1 = 66, inb_lifid2 = 67, host_lifid1 = 68, host_lifid2 = 69, oob_lifid = 70;
    uint32_t       enic_inb1 = 257, enic_inb2 = 258, enic_oob = 259, enic_wl1 = 200;
    uint32_t       wl_encap1 = 100;

    // Create uplinks
    ASSERT_EQ(create_uplink(uplinkif_id1, up_port1), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id2, up_port2), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id3, up_port3, 0, true), HAL_RET_OK);

    // Create 2 Classic VRFs
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

    // Create customer VRF
    ASSERT_EQ(create_vrf(vrf_id_hp1, types::VRF_TYPE_CUSTOMER, 0), HAL_RET_OK);
    // Create customer L2seg with wire encap as 8192 
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id2;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_hp1, l2seg_id_hp1, 100, up_ifid, ifid_count,
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    // Create Enic
    ASSERT_EQ(create_enic(enic_wl1, host_lifid1, intf::IF_ENIC_TYPE_USEG,
                          l2seg_id_hp1, wl_encap1, 0), HAL_RET_OK);
    // Create WL EP
    ASSERT_EQ(create_ep(vrf_id_hp1, l2seg_id_hp1, enic_wl1, 0x000020002001), HAL_RET_OK);

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

/* 
 * SWM vlan different from Inband vlan
 * Customer Vlan same as SWM vlan
 * Customer Vlan same as Inband Vlan
 */
TEST_F(ep_mgmt_merge_test, test4)
{
    uint32_t       test_id = 1;
    uint32_t       uplinkif_id1 = UPLINK_IF_ID_OFFSET + test_id,
                   uplinkif_id2 = uplinkif_id1 + 1, uplinkif_id3 = uplinkif_id1 + 2;
    uint32_t       up_ifid[2] = {0, 0}, /*ifid[2] = {0, 0},*/ ifid_count = 0;
    uint32_t       vrf_id_cl_up1 = 18, vrf_id_cl_up2 = 19, vrf_id_cl_up3 = 20,
                   vrf_id_hp1 = 65;
    uint32_t       l2seg_id_cl_up1 = 1, l2seg_id_cl_up2 = 2, l2seg_id_cl_up3 = 3, 
                   l2seg_id_hp1 = 103, l2seg_id_cl_swm_up1 = 4, l2seg_id_cl_swm_oob = 5;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       inb_lifid1 = 66, inb_lifid2 = 67, host_lifid1 = 68, host_lifid2 = 69, oob_lifid = 70;
    uint32_t       enic_inb1 = 257, enic_inb2 = 258, enic_oob = 259, enic_wl1 = 200;
    uint32_t       wl_encap1 = 100;
    uint32_t       swm_vlan = 1000;

    // Create uplinks
    ASSERT_EQ(create_uplink(uplinkif_id1, up_port1), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id2, up_port2), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id3, up_port3, 0, true), HAL_RET_OK);

    // Create 2 Classic VRFs
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

    // Create customer VRF
    ASSERT_EQ(create_vrf(vrf_id_hp1, types::VRF_TYPE_CUSTOMER, 0), HAL_RET_OK);
    // Create customer L2seg with wire encap as 8192 
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id2;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_hp1, l2seg_id_hp1, swm_vlan, up_ifid, ifid_count,
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    // Create Enic
    ASSERT_EQ(create_enic(enic_wl1, host_lifid1, intf::IF_ENIC_TYPE_USEG,
                          l2seg_id_hp1, wl_encap1, 0), HAL_RET_OK);
    // Create WL EP
    ASSERT_EQ(create_ep(vrf_id_hp1, l2seg_id_hp1, enic_wl1, 0x000020002001), HAL_RET_OK);

    // Create swm vlans
    up_ifid[0] = uplinkif_id1;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_swm_up1, swm_vlan, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);
    up_ifid[0] = uplinkif_id3;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up3, l2seg_id_cl_swm_oob, swm_vlan, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, true), HAL_RET_OK);

    // Enable SWM on oob l2seg and first uplink
    up_ifid[0] = uplinkif_id1;
    up_ifid[1] = uplinkif_id3;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_swm_up1, swm_vlan, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           true, false), HAL_RET_OK);
    up_ifid[0] = uplinkif_id3;
    up_ifid[1] = uplinkif_id1;
    ifid_count = 2;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up3, l2seg_id_cl_swm_oob, swm_vlan, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           true, false), HAL_RET_OK);

    ASSERT_EQ(create_ep(vrf_id_cl_up1, l2seg_id_cl_swm_up1, uplinkif_id3,  0x000010002004), HAL_RET_OK);

    // Delete EP
    ASSERT_EQ(delete_ep(vrf_id_cl_up1, l2seg_id_cl_swm_up1, 0x000010002004), HAL_RET_OK);

    // Disable SWM
    up_ifid[0] = uplinkif_id1;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up1, l2seg_id_cl_swm_up1, swm_vlan, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, false), HAL_RET_OK);
    up_ifid[0] = uplinkif_id3;
    ifid_count = 1;
    ASSERT_EQ(update_l2seg(vrf_id_cl_up3, l2seg_id_cl_swm_oob, swm_vlan, up_ifid, ifid_count, 
                           l2segment::MulticastFwdPolicy::MULTICAST_FWD_POLICY_FLOOD,
                           l2segment::BroadcastFwdPolicy::BROADCAST_FWD_POLICY_FLOOD,
                           false, false), HAL_RET_OK);

    // Delete SWM l2segs
    ASSERT_EQ(delete_l2seg(vrf_id_cl_up1, l2seg_id_cl_swm_up1), HAL_RET_OK);
    ASSERT_EQ(delete_l2seg(vrf_id_cl_up3, l2seg_id_cl_swm_oob), HAL_RET_OK);

#if 0
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
