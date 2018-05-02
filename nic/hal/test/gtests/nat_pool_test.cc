// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/nw/nh.hpp"
#include "nic/hal/src/nw/route.hpp"
#include "nic/hal/src/nw/route_acl.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/src/l4lb/l4lb.hpp"
#include "nic/hal/src/nat/nat.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/gen/proto/hal/session.pb.h"
#include "nic/gen/proto/hal/l4lb.pb.h"
#include "nic/gen/proto/hal/nw.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/include/hal_state.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/rdma_svc.hpp"
#include "nic/hal/svc/session_svc.hpp"
#include "nic/hal/svc/wring_svc.hpp"
#include "nic/hal/svc/rawrcb_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/barco_rings_svc.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"

#include "nic/gen/hal/svc/telemetry_svc_gen.hpp"
#include "nic/gen/hal/svc/nw_svc_gen.hpp"
#include "nic/gen/hal/svc/tls_proxy_cb_svc_gen.hpp"
#include "nic/gen/hal/svc/tcp_proxy_cb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyccb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyrcb_svc_gen.hpp"
#include "nic/gen/hal/svc/vrf_svc_gen.hpp"
#include "nic/gen/hal/svc/l2segment_svc_gen.hpp"
#include "nic/gen/hal/svc/internal_svc_gen.hpp"
#include "nic/gen/hal/svc/endpoint_svc_gen.hpp"
#include "nic/gen/hal/svc/l4lb_svc_gen.hpp"
#include "nic/gen/hal/svc/nwsec_svc_gen.hpp"
#include "nic/gen/hal/svc/dos_svc_gen.hpp"
#include "nic/gen/hal/svc/qos_svc_gen.hpp"
#include "nic/gen/hal/svc/descriptor_aol_svc_gen.hpp"
#include "nic/gen/hal/svc/acl_svc_gen.hpp"
#include "nic/gen/hal/svc/ipseccb_svc_gen.hpp"
#include "nic/gen/hal/svc/cpucb_svc_gen.hpp"
#include "nic/gen/hal/svc/crypto_keys_svc_gen.hpp"
#include "nic/gen/hal/svc/rawccb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyrcb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyccb_svc_gen.hpp"
#include "nic/gen/hal/svc/crypto_apis_svc_gen.hpp"
#include "nic/gen/hal/svc/multicast_svc_gen.hpp"
#include "nic/gen/hal/svc/gft_svc_gen.hpp"
#include "nic/gen/hal/svc/nat_svc_gen.hpp"

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

void
svc_reg (const std::string& server_addr,
         hal::hal_feature_set_t feature_set)
{
    VrfServiceImpl           vrf_svc;
    NetworkServiceImpl       nw_svc;
    InterfaceServiceImpl     if_svc;
    InternalServiceImpl      internal_svc;
    RdmaServiceImpl          rdma_svc;
    L2SegmentServiceImpl     l2seg_svc;
    DebugServiceImpl         debug_svc;
    TableServiceImpl         table_svc;
    SessionServiceImpl       session_svc;
    EndpointServiceImpl      endpoint_svc;
    L4LbServiceImpl          l4lb_svc;
    NwSecurityServiceImpl    nwsec_svc;
    DosServiceImpl           dos_svc;
    QOSServiceImpl           qos_svc;
    AclServiceImpl           acl_svc;
    TelemetryServiceImpl     telemetry_svc;
    ServerBuilder            server_builder;
    TlsCbServiceImpl         tlscb_svc;
    TcpCbServiceImpl         tcpcb_svc;
    DescrAolServiceImpl      descraol_svc;
    WRingServiceImpl         wring_svc;
    ProxyServiceImpl         proxy_svc;
    IpsecCbServiceImpl       ipseccb_svc;
    CpuCbServiceImpl         cpucb_svc;
    CryptoKeyServiceImpl     crypto_key_svc;
    RawrCbServiceImpl        rawrcb_svc;
    RawcCbServiceImpl        rawccb_svc;
    ProxyrCbServiceImpl      proxyrcb_svc;
    ProxycCbServiceImpl      proxyccb_svc;
    CryptoApisServiceImpl    crypto_apis_svc;
    EventServiceImpl         event_svc;
    QuiesceServiceImpl       quiesce_svc;
    BarcoRingsServiceImpl    barco_rings_svc;
    MulticastServiceImpl     multicast_svc;
    GftServiceImpl           gft_svc;
    SystemServiceImpl        system_svc;
    SoftwarePhvServiceImpl   swphv_svc;
    NatServiceImpl           nat_svc;

    grpc_init();
    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder.RegisterService(&vrf_svc);
        server_builder.RegisterService(&nw_svc);
        server_builder.RegisterService(&if_svc);
        server_builder.RegisterService(&internal_svc);
        server_builder.RegisterService(&rdma_svc);
        server_builder.RegisterService(&l2seg_svc);
        server_builder.RegisterService(&debug_svc);
        server_builder.RegisterService(&table_svc);
        server_builder.RegisterService(&session_svc);
        server_builder.RegisterService(&endpoint_svc);
        server_builder.RegisterService(&l4lb_svc);
        server_builder.RegisterService(&nwsec_svc);
        server_builder.RegisterService(&dos_svc);
        server_builder.RegisterService(&tlscb_svc);
        server_builder.RegisterService(&tcpcb_svc);
        server_builder.RegisterService(&qos_svc);
        server_builder.RegisterService(&descraol_svc);
        server_builder.RegisterService(&wring_svc);
        server_builder.RegisterService(&proxy_svc);
        server_builder.RegisterService(&acl_svc);
        server_builder.RegisterService(&telemetry_svc);
        server_builder.RegisterService(&ipseccb_svc);
        server_builder.RegisterService(&cpucb_svc);
        server_builder.RegisterService(&crypto_key_svc);
        server_builder.RegisterService(&rawrcb_svc);
        server_builder.RegisterService(&rawccb_svc);
        server_builder.RegisterService(&proxyrcb_svc);
        server_builder.RegisterService(&proxyccb_svc);
        server_builder.RegisterService(&crypto_apis_svc);
        server_builder.RegisterService(&event_svc);
        server_builder.RegisterService(&quiesce_svc);
        server_builder.RegisterService(&barco_rings_svc);
        server_builder.RegisterService(&multicast_svc);
        server_builder.RegisterService(&system_svc);
        server_builder.RegisterService(&swphv_svc);
        server_builder.RegisterService(&nat_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder.RegisterService(&vrf_svc);
        server_builder.RegisterService(&if_svc);
        server_builder.RegisterService(&rdma_svc);
        server_builder.RegisterService(&l2seg_svc);
        server_builder.RegisterService(&gft_svc);
        server_builder.RegisterService(&system_svc);
        // Revisit. DOL was not able to create Lif without qos class
        server_builder.RegisterService(&qos_svc);
        // Revisit. DOL was not able to create Tenant with security profile.
        server_builder.RegisterService(&nwsec_svc);
        server_builder.RegisterService(&dos_svc);
        server_builder.RegisterService(&endpoint_svc);
    }

    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());
    hal::utils::hal_logger()->flush();
    HAL_SYSLOG_INFO("HAL-STATUS:UP");

    // assemble the server
    std::unique_ptr<Server> server(server_builder.BuildAndStart());

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}

class nat_pool_test : public hal_base_test {
protected:
  nat_pool_test() {
  }

  virtual ~nat_pool_test() {
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
TEST_F(nat_pool_test, test1)
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
    NexthopSpec                 nh_spec;
    NexthopResponse             nh_rsp;
    RouteSpec                   route_spec;
    RouteResponse               route_rsp;
    NatPoolSpec                 nat_spec;
    NatPoolResponse             nat_rsp;
    NatMappingSpec              nat_mapping_spec;
    NatMappingResponse          nat_mapping_rsp;
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
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

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
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

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
    up_spec.mutable_key_or_handle()->set_interface_id(1);
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
    // ::google::protobuf::uint64 ep_hdl = ep_rsp.mutable_endpoint_status()->endpoint_handle();

    // Create NAT Pool
    nat_spec.mutable_key_or_handle()->mutable_pool_key()->mutable_vrf_kh()->set_vrf_id(1);
    nat_spec.mutable_key_or_handle()->mutable_pool_key()->set_pool_id(1);
    nat_spec.add_address();
    nat_spec.mutable_address(0)->mutable_range()->mutable_ipv4_range()->mutable_low_ipaddr()->set_ip_af(types::IP_AF_INET);
    nat_spec.mutable_address(0)->mutable_range()->mutable_ipv4_range()->mutable_low_ipaddr()->set_v4_addr(0x0a000001);
    nat_spec.mutable_address(0)->mutable_range()->mutable_ipv4_range()->mutable_high_ipaddr()->set_ip_af(types::IP_AF_INET);
    nat_spec.mutable_address(0)->mutable_range()->mutable_ipv4_range()->mutable_high_ipaddr()->set_v4_addr(0x0a000003);
#if 0
    nat_spec.mutable_address(0)->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nat_spec.mutable_address(0)->mutable_prefix()->mutable_ipv4_subnet()->mutable_address()->set_v4_addr(ip1);
    nat_spec.mutable_address(0)->mutable_prefix()->mutable_ipv4_subnet()->set_prefix_len(24);
#endif
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_pool_create(nat_spec, &nat_rsp);
    hal::hal_cfg_db_close();

    // Create NAT Mapping
    nat_mapping_spec.mutable_key_or_handle()->mutable_svc()->mutable_vrf_kh()->set_vrf_id(1);
    nat_mapping_spec.mutable_key_or_handle()->mutable_svc()->mutable_ip_addr()->set_ip_af(types::IP_AF_INET);
    nat_mapping_spec.mutable_key_or_handle()->mutable_svc()->mutable_ip_addr()->set_v4_addr(0x14000001);
    nat_mapping_spec.mutable_nat_pool()->mutable_pool_key()->set_pool_id(1);
    nat_mapping_spec.set_bidir(1);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::nat_mapping_create(nat_mapping_spec, &nat_mapping_rsp);
    hal::hal_cfg_db_close();
    
    EXPECT_EQ(ret, HAL_RET_OK);




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
