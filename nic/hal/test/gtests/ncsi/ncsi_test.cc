#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/nw/nic.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/ncsi/ncsi.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/nic.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "nic/hal/test/hal_calls/hal_calls.hpp"
#include "nic/hal/svc/nic_svc.hpp"
#include "nic/hal/svc/hal_ext.hpp"
#include "nic/hal/core/event_ipc.hpp"

#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"

#include "gen/hal/svc/telemetry_svc_gen.hpp"
#include "gen/hal/svc/nw_svc_gen.hpp"
#include "gen/hal/svc/rdma_svc_gen.hpp"
#include "gen/hal/svc/vrf_svc_gen.hpp"
#include "gen/hal/svc/ncsi_svc_gen.hpp"
#include "gen/hal/svc/l2segment_svc_gen.hpp"
#include "gen/hal/svc/internal_svc_gen.hpp"
#include "gen/hal/svc/endpoint_svc_gen.hpp"
#include "gen/hal/svc/l4lb_svc_gen.hpp"
#include "gen/hal/svc/nwsec_svc_gen.hpp"
#include "gen/hal/svc/qos_svc_gen.hpp"
#include "gen/hal/svc/acl_svc_gen.hpp"
#include "gen/hal/svc/cpucb_svc_gen.hpp"
#include "gen/hal/svc/multicast_svc_gen.hpp"
using intf::InterfaceSpec;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using kh::LifKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using vrf::VrfSpec;
using vrf::VrfResponse;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponse;
using vrf::VrfDeleteResponse;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using ncsi::VlanFilterRequest;
using ncsi::VlanFilterResponse;

uint16_t g_lif_id = 100; // Use LIFs in the host-lif range

class ncsi_test : public hal_base_test {
protected:
  ncsi_test() {
  }

  virtual ~ncsi_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCaseGrpc();
    hal_test_utils_slab_disable_delete();
  }

};

TEST_F(ncsi_test, test1)
{
    hal_ret_t               ret;
    VlanFilterRequest       req;
    VlanFilterResponse      rsp;
    MacFilterRequest        mac_req;
    MacFilterResponse       mac_rsp;
    BcastFilterRequest      bcast_req;
    BcastFilterResponse     bcast_rsp;
    McastFilterRequest      mcast_req;
    McastFilterResponse     mcast_rsp;
    ChannelRequest          ch_req;
    ChannelResponse         ch_rsp;
    VlanModeRequest         vlan_mode_req;
    VlanModeResponse        vlan_mode_rsp;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       uplinkif_id1 = UPLINK_IF_INDEX1,
                   uplinkif_id2 = UPLINK_IF_INDEX2, uplinkif_id3 = UPLINK_IF_INDEX3;

    ASSERT_EQ(create_uplink(uplinkif_id1, up_port1), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id2, up_port2), HAL_RET_OK);
    ASSERT_EQ(create_uplink(uplinkif_id3, up_port3, 0, true), HAL_RET_OK);

    // notify sysmgr that we are up
    hal::svc::hal_init_done();
    // raise HAL_UP event
    sdk::ipc::broadcast(hal::core::EVENT_ID_HAL_UP, NULL, 0);

    // sleep(5);

    req.set_vlan_id(10);
    req.set_channel(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vlan_filter_create(req, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(rsp.api_status() == types::API_STATUS_OK);

    // Add Vlan
    req.Clear();
    req.set_vlan_id(10);
    req.set_channel(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vlan_filter_create(req, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(rsp.api_status() == types::API_STATUS_OK);

    // Adding mac
    mac_req.set_mac_addr(0x000000010001);
    mac_req.set_channel(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::mac_filter_create(mac_req, &mac_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(mac_rsp.api_status() == types::API_STATUS_OK);

    // Deleting mac
    mac_req.set_mac_addr(0x000000010001);
    mac_req.set_channel(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::mac_filter_delete(mac_req, &mac_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(mac_rsp.api_status() == types::API_STATUS_OK);

    // Adding mac
    mac_req.set_mac_addr(0x000000010001);
    mac_req.set_channel(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::mac_filter_create(mac_req, &mac_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(mac_rsp.api_status() == types::API_STATUS_OK);

    // Adding mac
    mac_req.set_mac_addr(0x000000010002);
    mac_req.set_channel(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::mac_filter_create(mac_req, &mac_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(mac_rsp.api_status() == types::API_STATUS_OK);

    // Bcast filter
    bcast_req.set_enable_arp(true);
    bcast_req.set_channel(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::bcast_filter_create(bcast_req, &bcast_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(bcast_rsp.api_status() == types::API_STATUS_OK);

    // Bcast filter
    bcast_req.Clear();
    bcast_req.set_enable_dhcp_client(true);
    bcast_req.set_channel(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::bcast_filter_create(bcast_req, &bcast_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(bcast_rsp.api_status() == types::API_STATUS_OK);

    // Mcast filter
    mcast_req.set_enable_ipv6_neigh_adv(true);
    mcast_req.set_channel(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::mcast_filter_create(mcast_req, &mcast_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(mcast_rsp.api_status() == types::API_STATUS_OK);

    // Mcast filter
    mcast_req.Clear();
    mcast_req.set_enable_ipv6_router_adv(true);
    mcast_req.set_channel(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::mcast_filter_create(mcast_req, &mcast_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(mcast_rsp.api_status() == types::API_STATUS_OK);

    // Set Rx Enable
    ch_req.set_channel(0);
    ch_req.set_rx_enable(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::channel_create(ch_req, &ch_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(ch_rsp.api_status() == types::API_STATUS_OK);

    // Set Tx Enable
    ch_req.set_channel(0);
    ch_req.set_rx_enable(true);
    ch_req.set_tx_enable(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::channel_update(ch_req, &ch_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(ch_rsp.api_status() == types::API_STATUS_OK);

    // Set Rx Enable
    ch_req.set_channel(1);
    ch_req.set_rx_enable(true);
    ch_req.set_tx_enable(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::channel_create(ch_req, &ch_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(ch_rsp.api_status() == types::API_STATUS_OK);

    // Enable vlan mode
    vlan_mode_req.set_channel(0);
    vlan_mode_req.set_enable(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vlan_mode_update(vlan_mode_req, &vlan_mode_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(ch_rsp.api_status() == types::API_STATUS_OK);

    // Enable vlan mode with only vlan
    vlan_mode_req.set_channel(0);
    vlan_mode_req.set_enable(1);
    vlan_mode_req.set_mode(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vlan_mode_update(vlan_mode_req, &vlan_mode_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(ch_rsp.api_status() == types::API_STATUS_OK);

    // Enable vlan mode with only vlan
    vlan_mode_req.set_channel(0);
    vlan_mode_req.set_enable(0);
    vlan_mode_req.set_mode(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vlan_mode_update(vlan_mode_req, &vlan_mode_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(ch_rsp.api_status() == types::API_STATUS_OK);

    // To test halctl in sim
    // sleep(10000);
}

TEST_F(ncsi_test, qos)
{
    uint32_t       uplinkif_id1 = UPLINK_IF_INDEX1,
                   uplinkif_id2 = UPLINK_IF_INDEX2, uplinkif_id3 = UPLINK_IF_INDEX3;
    uint32_t       up_ifid[2] = {0, 0}, /*ifid[2] = {0, 0},*/ ifid_count = 0;
    uint32_t       vrf_id_cl_up1 = 18, vrf_id_cl_up2 = 19, vrf_id_cl_up3 = 20/*,
                   vrf_id_hp1 = 65*/;
    uint32_t       l2seg_id_cl_up1 = 1, l2seg_id_cl_up2 = 2, l2seg_id_cl_up3 = 3 /*,l2seg_id_hp1 = 103*/;
    uint32_t       up_port1 = PORT_NUM_1, up_port2 = PORT_NUM_2, up_port3 = PORT_NUM_3;
    uint32_t       swm_lifid1 = 65, swm_lifid2 = 66, inb_lifid1 = 67, inb_lifid2 = 68, host_lifid1 = 69, host_lifid2 = 70, oob_lifid = 71;
    uint32_t       enic_swm1 = 300, enic_swm2 = 301, enic_inb1 = 257, enic_inb2 = 258, enic_oob = 259/*, enic_wl1 = 200*/;
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
    ASSERT_EQ(create_lif(swm_lifid1, uplinkif_id1, types::LIF_TYPE_SWM,
                         "swm_lif0"), HAL_RET_OK);
    ASSERT_EQ(create_lif(swm_lifid2, uplinkif_id2, types::LIF_TYPE_SWM,
                         "swm_lif1"), HAL_RET_OK);
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
    ASSERT_EQ(create_enic(enic_swm1, swm_lifid1, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_swm2, swm_lifid2, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up2), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_inb1, inb_lifid1, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up1), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_inb2, inb_lifid2, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up2), HAL_RET_OK);
    ASSERT_EQ(create_enic(enic_oob, oob_lifid, intf::IF_ENIC_TYPE_CLASSIC,
                          0, 0, l2seg_id_cl_up3), HAL_RET_OK);

    // Create Classic EPs
    ASSERT_EQ(create_ep(vrf_id_cl_up1, l2seg_id_cl_up1, enic_swm1, 0x000010002001), HAL_RET_OK);
    ASSERT_EQ(create_ep(vrf_id_cl_up2, l2seg_id_cl_up2, enic_swm1, 0x000010002001), HAL_RET_OK);
    ASSERT_EQ(create_ep(vrf_id_cl_up2, l2seg_id_cl_up2, enic_swm2, 0x000010003001), HAL_RET_OK);

    // Delete EP
    ASSERT_EQ(delete_ep(vrf_id_cl_up1, l2seg_id_cl_up1, 0x000010002001), HAL_RET_OK);
    ASSERT_EQ(delete_ep(vrf_id_cl_up2, l2seg_id_cl_up2, 0x000010003001), HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
