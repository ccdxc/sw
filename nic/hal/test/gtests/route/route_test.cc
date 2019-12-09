// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/nw/nh.hpp"
#include "nic/hal/plugins/cfg/nw/route.hpp"
#include "nic/hal/plugins/cfg/nw/route_acl.hpp"
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
#include "gen/hal/svc/session_svc_gen.hpp"

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
#include "gen/hal/svc/l2segment_svc_gen.hpp"
#include "gen/hal/svc/internal_svc_gen.hpp"
#include "gen/hal/svc/endpoint_svc_gen.hpp"
#include "gen/hal/svc/l4lb_svc_gen.hpp"
#include "gen/hal/svc/nwsec_svc_gen.hpp"
//#include "gen/hal/svc/dos_svc_gen.hpp"
#include "gen/hal/svc/qos_svc_gen.hpp"
#include "gen/hal/svc/acl_svc_gen.hpp"
#include "gen/hal/svc/cpucb_svc_gen.hpp"
#include "gen/hal/svc/multicast_svc_gen.hpp"

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
using nw::RouteSpec;
using nw::RouteResponse;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class route_test : public hal_base_test {
protected:
  route_test() {
  }

  virtual ~route_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// ----------------------------------------------------------------------------
// Creating a route
// ----------------------------------------------------------------------------
TEST_F(route_test, test1)
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
    EndpointDeleteRequest       ep_dreq;
    EndpointDeleteResponse      ep_dresp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    NexthopSpec                 nh_spec;
    NexthopResponse             nh_rsp;
    NexthopDeleteRequest        nh_dspec;
    NexthopDeleteResponse       nh_dresp;
    RouteSpec                   route_spec;
    RouteResponse               route_rsp;
    RouteDeleteRequest          rdel_spec;
    RouteDeleteResponse         rdel_rsp;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    NetworkKeyHandle            *nkh = NULL;


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
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
    ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create EP
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
    ::google::protobuf::uint64 ep_hdl = ep_rsp.mutable_endpoint_status()->key_or_handle().endpoint_handle();

    // Create a nexthop with if
    nh_spec.mutable_key_or_handle()->set_nexthop_id(1);
    nh_spec.mutable_if_key_or_handle()->set_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nexthop_create(nh_spec, &nh_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 nh_hdl1 = nh_rsp.mutable_status()->nexthop_handle();

    // Create a nexthop with EP
    nh_spec.mutable_key_or_handle()->set_nexthop_id(2);
    nh_spec.mutable_ep_key_or_handle()->set_endpoint_handle(ep_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nexthop_create(nh_spec, &nh_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 nh_hdl2 = nh_rsp.mutable_status()->nexthop_handle();

    // Create a route
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->set_prefix_len(24);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000001);
    route_spec.mutable_nh_key_or_handle()->set_nexthop_handle(nh_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::route_create(route_spec, &route_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 route_hdl1 = route_rsp.mutable_status()->route_handle();

    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->set_prefix_len(16);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    route_spec.mutable_key_or_handle()->mutable_route_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    route_spec.mutable_nh_key_or_handle()->set_nexthop_handle(nh_hdl2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::route_create(route_spec, &route_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 route_hdl2 = route_rsp.mutable_status()->route_handle();

    hal::route_key_t key = {0};
    hal_handle_t handle = 0;
    key.vrf_id = 1;
    key.pfx.addr.addr.v4_addr = 0xa0000001;
    key.pfx.len = 32;
    hal::route_acl_lookup(&key, &handle);
    EXPECT_EQ(handle, route_hdl1);

    // route delete
    rdel_spec.mutable_key_or_handle()->set_route_handle(route_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::route_delete(rdel_spec, &rdel_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Find route1 which is deleted
    key.vrf_id = 1;
    key.pfx.addr.addr.v4_addr = 0xa0000001;
    key.pfx.len = 32;
    ret = hal::route_acl_lookup(&key, &handle);
    HAL_TRACE_ERR("handle: {}", handle);
    EXPECT_EQ(handle, route_hdl2);

    // route delete
    rdel_spec.mutable_key_or_handle()->set_route_handle(route_hdl2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::route_delete(rdel_spec, &rdel_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Find route1 which is deleted
    key.vrf_id = 1;
    key.pfx.addr.addr.v4_addr = 0xa0000001;
    key.pfx.len = 32;
    ret = hal::route_acl_lookup(&key, &handle);
    EXPECT_EQ(ret, HAL_RET_ROUTE_NOT_FOUND);

    // Delete nexthop1
    nh_dspec.mutable_key_or_handle()->set_nexthop_handle(nh_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nexthop_delete(nh_dspec, &nh_dresp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete nexthop2
    nh_dspec.mutable_key_or_handle()->set_nexthop_handle(nh_hdl2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nexthop_delete(nh_dspec, &nh_dresp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete EP
    ep_dreq.mutable_key_or_handle()->set_endpoint_handle(ep_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_delete(ep_dreq, &ep_dresp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);



    // Uncomment these to have gtest work for CLI
#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
