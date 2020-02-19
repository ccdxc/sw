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
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
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


class ep_vmotion_client_test : public hal_base_test {
protected:
  ep_vmotion_client_test() {
  }

  virtual ~ep_vmotion_client_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase(true, "", "50056");
  }

};

TEST_F(ep_vmotion_client_test, test1)
 {
     mac_addr_t mac = {0x0, 0x0, 0x0, 0x0, 0xAB, 0xCD};
    hal_ret_t                   ret;
    VrfSpec                     vrf_spec;
    VrfResponse                 vrf_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec, enicif_spec1;
    InterfaceResponse           up_rsp, enicif_rsp1;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    EndpointUpdateRequest       ep_req, ep_req1;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    ::google::protobuf::uint32  homing_host_ip = 0x7f000001; // 127.0.0.1
    ::google::protobuf::uint32  new_homing_host_ip = 0x7f000002; // 127.0.0.2
    NetworkKeyHandle            *nkh = NULL;

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(10);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(101);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(1001);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(102);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(1002);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(102);
    lif_spec.add_lif_qstate_map();
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(102);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(101);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec1.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(101);
    enicif_spec1.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(101);
    enicif_spec1.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x000000000001);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl1 = enicif_rsp1.mutable_status()->if_handle();


    // Create an uplink
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    up_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.3
    ep_spec.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_v4_addr(homing_host_ip);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec1.mutable_vrf_key_handle()->set_vrf_id(10);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec1.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x000000001234);
    ep_spec1.mutable_endpoint_attrs()->add_ip_address();
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.4
    ep_spec1.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_v4_addr(homing_host_ip);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    sleep(5);

    printf("Triggering vmotion client \n");
     // Start a client and connect to server
    hal::g_hal_state->get_vmotion()->process_rarp(mac);

    sleep(5);
    printf("Trigged EP update for MAC 0x00.00.00.00.AB.CD\n");
    ep_req.mutable_vrf_key_handle()->set_vrf_id(10);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    // setting the if_hdl to enic to trigger vmotion update
    ep_req.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(if_hdl1);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.3
    ep_req.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_v4_addr(new_homing_host_ip);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req, &ep_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    sleep(5);

    printf("Triggering vmotion client \n");
     // Start a client and connect to server
    hal::g_hal_state->get_vmotion()->process_rarp(mac);
#endif

    sleep(1000);
 }

TEST_F(ep_vmotion_client_test, DISABLED_test2)
 {
     mac_addr_t mac = {0x0, 0x0, 0x0, 0x0, 0xAB, 0xCD};
    hal_ret_t                   ret;
    VrfSpec                     vrf_spec;
    VrfResponse                 vrf_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    EndpointUpdateRequest       ep_req, ep_req1;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    InterfaceSpec               enicif_spec2;
    InterfaceResponse           enicif_rsp2; 
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    ::google::protobuf::uint32  homing_host_ip = 0x7f000001; // 127.0.0.1
    NetworkKeyHandle            *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(10);
    vrf_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(101);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(1001);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create an uplink
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(102);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create eNIC
    enicif_spec2.set_type(intf::IF_TYPE_ENIC);
    enicif_spec2.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(102);
    enicif_spec2.mutable_key_or_handle()->set_interface_id(102);
    enicif_spec2.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec2.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(101);
    enicif_spec2.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(101);
    enicif_spec2.mutable_if_enic_info()->mutable_enic_info()->set_mac_address(0x000000000002);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec2, &enicif_rsp2);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl2 = enicif_rsp2.mutable_status()->if_handle();

    // Create 2 Endpoints
    // EP1 - Remote Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    ep_spec.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_v4_addr(homing_host_ip);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // EP2 - Local Endpoint
    ep_spec1.mutable_vrf_key_handle()->set_vrf_id(10);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec1.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(if_hdl2);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x000000001234);
    ep_spec1.mutable_endpoint_attrs()->add_ip_address();
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.2
    ep_spec1.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_endpoint_attrs()->mutable_old_homing_host_ip()->set_v4_addr(homing_host_ip);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Session
    sess_spec.mutable_vrf_key_handle()->set_vrf_id(10);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1000);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2000);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2000);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1000);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    sleep(5);

     printf("Triggering vmotion client \n");
     // Start a client and connect to server
     hal::g_hal_state->get_vmotion()->process_rarp(mac);

    sleep(1000);
 }


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
