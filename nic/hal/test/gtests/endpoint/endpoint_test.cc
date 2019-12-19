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


class endpoint_test : public hal_base_test {
protected:
  endpoint_test() {
  }

  virtual ~endpoint_test() {
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
// Creating a endpoint
// ----------------------------------------------------------------------------
TEST_F(endpoint_test, test1)
{
    hal_ret_t                   ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp;
    EndpointUpdateRequest       ep_req, ep_req1;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    ::google::protobuf::uint32  ip3 = 0x0a000005;
    ::google::protobuf::uint32  ip4 = 0x0a000006;
    NetworkKeyHandle            *nkh = NULL;


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    ten_spec.set_vrf_type(types::VRF_TYPE_CUSTOMER);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);
#if 0
    ep_spec1.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec1.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x000000001234);
    ep_spec1.mutable_endpoint_attrs()->add_ip_address();
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Try deletint if
    InterfaceDeleteRequest   if_del_req;
    InterfaceDeleteResponse  if_del_rsp;
    if_del_req.mutable_key_or_handle()->set_interface_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(if_del_req, &if_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);


    // Update with IP adds
    ep_req.mutable_vrf_key_handle()->set_vrf_id(1);
    // ep_req.mutable_key_or_handle()->set_endpoint_handle(ep_rsp.endpoint_status().mutable_key_or_handle().endpoint_handle());
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_req.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(1)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(1)->set_v4_addr(ip2);  // 10.0.0.1
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(2)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(2)->set_v4_addr(ip3);  // 10.0.0.1
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(3)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(3)->set_v4_addr(ip4);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req, &ep_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update with IP deletes
    ep_req1.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_req1.mutable_key_or_handle()->set_endpoint_handle(ep_rsp.endpoint_status().key_or_handle().endpoint_handle());
    ep_req1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_req1.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_req1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_req1.mutable_endpoint_attrs()->add_ip_address();
    // ep_req1.mutable_endpoint_attrs()->add_ip_address();
    // ep_req1.mutable_endpoint_attrs()->add_ip_address();
    // ep_req1.mutable_endpoint_attrs()->add_ip_address();
    ep_req1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    // ep_req1.mutable_endpoint_attrs()->mutable_ip_address(1)->set_ip_af(types::IP_AF_INET);
    // ep_req1.mutable_endpoint_attrs()->mutable_ip_address(1)->set_v4_addr(ip2);  // 10.0.0.1
    // ep_req1.mutable_endpoint_attrs()->mutable_ip_address(2)->set_ip_af(types::IP_AF_INET);
    // ep_req1.mutable_endpoint_attrs()->mutable_ip_address(2)->set_v4_addr(ip3);  // 10.0.0.1
    // ep_req1.mutable_endpoint_attrs()->mutable_ip_address(3)->set_ip_af(types::IP_AF_INET);
    // ep_req1.mutable_endpoint_attrs()->mutable_ip_address(3)->set_v4_addr(ip4);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req1, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Creating a endpoint with mcast mac
// ----------------------------------------------------------------------------
TEST_F(endpoint_test, test2)
{
    hal_ret_t                   ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec;
    EndpointResponse            ep_rsp;
    EndpointUpdateRequest       ep_req;
    EndpointDeleteRequest       ep_del_req;
    EndpointDeleteResponse      ep_del_rsp;
    NetworkSpec                 nw_spec;
    NetworkResponse             nw_rsp;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    NetworkKeyHandle            *nkh = NULL;


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec.set_vrf_type(types::VRF_TYPE_CUSTOMER);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0002DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();

    // Create an uplink
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(21);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x01000000ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000002ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_req.mutable_vrf_key_handle()->set_vrf_id(2);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_req.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000003ABCD);
    ep_req.mutable_endpoint_attrs()->add_ip_address();
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_EP_NOT_FOUND);

    ep_del_req.mutable_vrf_key_handle()->set_vrf_id(2);
    ep_del_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_del_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000003ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_delete(ep_del_req, &ep_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_EP_NOT_FOUND);

    ep_del_req.mutable_vrf_key_handle()->set_vrf_id(2);
    ep_del_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(100);
    ep_del_req.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000003ABCD);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_delete(ep_del_req, &ep_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
}

// ----------------------------------------------------------------------------
// Creating a endpoint and toggling ipsg en on nwsec profile
// ----------------------------------------------------------------------------
TEST_F(endpoint_test, test3)
{
    hal_ret_t                   ret;
    VrfSpec                     ten_spec;
    VrfResponse                 ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp;
    EndpointUpdateRequest       ep_req, ep_req1;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    ::google::protobuf::uint32  ip3 = 0x0a000005;
    ::google::protobuf::uint32  ip4 = 0x0a000006;
    NetworkKeyHandle            *nkh = NULL;
    SecurityProfileSpec         sp_spec, sp_spec1;
    SecurityProfileResponse     sp_rsp, sp_rsp1;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(3);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(3);
    ten_spec.set_vrf_type(types::VRF_TYPE_CUSTOMER);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(3);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(3);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->mutable_key_or_handle()->l2segment_handle();

    // Create an uplink
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(3);
    up_spec.mutable_if_uplink_info()->set_port_num(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    // Create Endpoint
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(3);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.3
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(1)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(1)->set_v4_addr(ip2);  // 10.0.0.4
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(2)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(2)->set_v4_addr(ip3);  // 10.0.0.5
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(3)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(3)->set_v4_addr(ip4);  // 10.0.0.6
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    printf("EP Create ret: %d", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Disable ipsg on nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(3);
    sp_spec.set_ipsg_en(false);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Enable ipsg on nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(3);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
