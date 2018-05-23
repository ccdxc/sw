// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/nw/nh.hpp"
#include "nic/hal/src/nw/route.hpp"
#include "nic/hal/src/ipsec/ipsec.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/gen/proto/hal/session.pb.h"
#include "nic/gen/proto/hal/nw.pb.h"
#include "nic/gen/proto/hal/ipsec.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/include/hal_state.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/session_svc.hpp"
#include "nic/hal/svc/wring_svc.hpp"
#include "nic/hal/svc/rawrcb_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/barco_rings_svc.hpp"
#include "nic/hal/svc/interface_svc.hpp"

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
#include "nic/gen/hal/svc/ipsec_svc_gen.hpp"
#include <google/protobuf/util/json_util.h>

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
using endpoint::EndpointSpec;
using endpoint::EndpointResponse;
using endpoint::EndpointUpdateRequest;
using session::SessionSpec;
using session::SessionResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;
using nw::RouteSpec;
using nw::RouteResponse;
using ipsec::IpsecSAEncrypt;
using ipsec::IpsecSADecrypt;
using ipsec::IpsecSAEncryptDeleteRequest;
using ipsec::IpsecSAEncryptDeleteResponseMsg;
using ipsec::IpsecSADecryptDeleteRequest;
using ipsec::IpsecSADecryptDeleteResponseMsg;
using ipsec::IpsecSAEncryptGetRequest;
using ipsec::IpsecSADecryptGetRequest;
using ipsec::IpsecSAEncryptGetResponse;
using ipsec::IpsecSADecryptGetResponse;
using ipsec::IPSecSAEncryptGetSpec;
using ipsec::IPSecSADecryptGetSpec;

using ipsec::IpsecRuleSpec;
using ipsec::IpsecRuleMatchSpec;
using ipsec::IpsecSAAction;


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
    ServerBuilder            server_builder;
    TlsCbServiceImpl         tlscb_svc;
    TcpCbServiceImpl         tcpcb_svc;
    DescrAolServiceImpl      descraol_svc;
    WRingServiceImpl         wring_svc;
    IpsecServiceImpl         ipsec_svc;
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
        server_builder.RegisterService(&acl_svc);
        server_builder.RegisterService(&ipsec_svc);
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

class ipsec_encrypt_test : public hal_base_test {
protected:
  ipsec_encrypt_test() {
  }

  virtual ~ipsec_encrypt_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

static inline void
ipsec_sa_encrypt_test_spec_dump (IpsecSAEncryptGetResponseMsg& rsp)
{
    std::string    ipsec_sa_encrypt_cfg_str;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return;
    }

    google::protobuf::util::MessageToJsonString(rsp, &ipsec_sa_encrypt_cfg_str);
    HAL_TRACE_DEBUG("IPSec SA Encrypt Config:");
    HAL_TRACE_DEBUG("{}", ipsec_sa_encrypt_cfg_str.c_str());
    return;
}

static inline void
ipsec_sa_decrypt_test_spec_dump (IpsecSADecryptGetResponseMsg& rsp)
{
    std::string    ipsec_sa_decrypt_cfg_str;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return;
    }

    google::protobuf::util::MessageToJsonString(rsp, &ipsec_sa_decrypt_cfg_str);
    HAL_TRACE_DEBUG("IPSec SA Decrypt Config:");
    HAL_TRACE_DEBUG("{}", ipsec_sa_decrypt_cfg_str.c_str());
    return;
}

// ----------------------------------------------------------------------------
// Creating a route
// ----------------------------------------------------------------------------
TEST_F(ipsec_encrypt_test, test1)
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
    IpsecSAEncrypt              encrypt_spec;
    IpsecSAEncryptResponse      encrypt_resp;
    IpsecSADecrypt              decrypt_spec;
    IpsecSADecryptResponse      decrypt_resp;

    IpsecSAEncryptGetRequest       enc_get_req;
    IpsecSAEncryptGetResponse      enc_get_rsp;
    IpsecSAEncryptGetResponseMsg   enc_get_rsp_msg;
    IpsecSADecryptGetRequest       dec_get_req;
    IpsecSADecryptGetResponse      dec_get_rsp;
    IpsecSADecryptGetResponseMsg   dec_get_rsp_msg;

    IpsecSAEncryptDeleteRequest del_enc_req;
    IpsecSADecryptDeleteRequest del_dec_req;
    IpsecSAEncryptDeleteResponseMsg del_enc_rsp;
    IpsecSADecryptDeleteResponseMsg del_dec_rsp;

    IpsecRuleSpec rule_req;
    IpsecRuleResponse rule_rsp;

 
    ::google::protobuf::uint32  ip1 = 0x0a010001;
    ::google::protobuf::uint32  ip2 = 0x0a010002;
    NetworkKeyHandle            *nkh = NULL;


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //uint64_t vrf1_handle = ten_rsp.mutable_vrf_status()->vrf_handle();

    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t vrf2_handle = ten_rsp.mutable_vrf_status()->vrf_handle();

    // Create network
    nw_spec.set_rmac(0x0000EEFF0002);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a010000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.set_rmac(0x0000EEFF0003);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a020000);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

    nw_spec.set_rmac(0x0000EEFF0008);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x40010000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.set_rmac(0x0000EEFF0009);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x40020000);
    nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();
    
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(3);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(4);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

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
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0000EEFF0002);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.1.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //::google::protobuf::uint64 ep_hdl = ep_rsp.mutable_endpoint_status()->endpoint_handle();

    ep_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x0000EEFF0003);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.1.0.2
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //ep_hdl = ep_rsp.mutable_endpoint_status()->endpoint_handle();

    //Encrypt
    encrypt_spec.mutable_key_or_handle()->set_cb_id(1);
    encrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    encrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    encrypt_spec.set_encryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    encrypt_spec.mutable_authentication_key()->set_key("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    encrypt_spec.mutable_encryption_key()->set_key("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    encrypt_spec.mutable_remote_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_remote_gateway_ip()->set_v4_addr(ip2);
    encrypt_spec.mutable_local_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_local_gateway_ip()->set_v4_addr(ip1);
    encrypt_spec.set_salt(0xbbbbbbbb);
    encrypt_spec.set_iv(0xaaaaaaaaaaaaaaaa);
    encrypt_spec.set_spi(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_create(encrypt_spec, &encrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //::google::protobuf::uint64 encrypt_hdl = encrypt_resp.mutable_ipsec_sa_status()->ipsec_sa_handle();

    encrypt_spec.mutable_key_or_handle()->set_cb_id(3);
    encrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    encrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    encrypt_spec.set_encryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    encrypt_spec.mutable_authentication_key()->set_key("3333333333333333333333333333333333333333333333333333333333333333");
    encrypt_spec.mutable_encryption_key()->set_key("3333333333333333333333333333333333333333333333333333333333333333");
    encrypt_spec.mutable_remote_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_remote_gateway_ip()->set_v4_addr(ip2);
    encrypt_spec.mutable_local_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_local_gateway_ip()->set_v4_addr(ip1);
    encrypt_spec.set_salt(0x22222222);
    encrypt_spec.set_iv(0x2222222222222222);
    encrypt_spec.set_spi(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_create(encrypt_spec, &encrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //::google::protobuf::uint64 encrypt_hdl3 = encrypt_resp.mutable_ipsec_sa_status()->ipsec_sa_handle();

    encrypt_spec.mutable_key_or_handle()->set_cb_id(4);
    encrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    encrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    encrypt_spec.set_encryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    encrypt_spec.mutable_authentication_key()->set_key("5555555555555555555555555555555555555555555555555555555555555555");
    encrypt_spec.mutable_encryption_key()->set_key("5555555555555555555555555555555555555555555555555555555555555555");
    encrypt_spec.mutable_remote_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_remote_gateway_ip()->set_v4_addr(ip1);
    encrypt_spec.mutable_local_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_local_gateway_ip()->set_v4_addr(ip1);
    encrypt_spec.set_salt(0x11111111);
    encrypt_spec.set_iv(0x1111111111111111);
    encrypt_spec.set_spi(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_create(encrypt_spec, &encrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //::google::protobuf::uint64 encrypt_hdl4 = encrypt_resp.mutable_ipsec_sa_status()->ipsec_sa_handle();



    encrypt_spec.mutable_key_or_handle()->set_cb_id(1);
    encrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    encrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    encrypt_spec.set_encryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    encrypt_spec.mutable_authentication_key()->set_key("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    encrypt_spec.mutable_encryption_key()->set_key("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    encrypt_spec.mutable_remote_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_remote_gateway_ip()->set_v4_addr(ip2);
    encrypt_spec.mutable_local_gateway_ip()->set_ip_af(types::IP_AF_INET);
    encrypt_spec.mutable_local_gateway_ip()->set_v4_addr(ip1);
    encrypt_spec.set_salt(0xdddddddd);
    encrypt_spec.set_iv(0xbbbbbbbbbbbbbbbb);
    encrypt_spec.set_spi(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_update(encrypt_spec, &encrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    enc_get_req.mutable_key_or_handle()->set_cb_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_get(enc_get_req, &enc_get_rsp_msg);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ipsec_sa_encrypt_test_spec_dump(enc_get_rsp_msg);

    rule_req.mutable_key_or_handle()->set_rule_handle(0);
    rule_req.mutable_vrf_key_handle()->set_vrf_handle(vrf2_handle);
    rule_req.add_rules();
    rule_req.mutable_key_or_handle()->mutable_rule_key()->mutable_vrf_key_or_handle()->set_vrf_handle(vrf2_handle);
    rule_req.mutable_rules(0)->set_rule_id(1);
    types::RuleMatch *match_spec = rule_req.mutable_rules(0)->mutable_match();
    //match_spec->mutable_src_address(0)->mutable_address()->set_type(types::IPAddressType::IP_ADDRESS_IPV4_ANY);
    match_spec->add_src_address();
    match_spec->mutable_src_address(0)->set_negate(false);
    auto v4_range = match_spec->mutable_src_address(0)->mutable_address()->mutable_range()->mutable_ipv4_range();
    v4_range->mutable_low_ipaddr()->set_v4_addr(0x40010000);
    v4_range->mutable_high_ipaddr()->set_v4_addr(0x400100ff);

    rule_req.mutable_rules(0)->mutable_sa_action()->set_sa_action_type(ipsec::IpsecSAActionType::IPSEC_SA_ACTION_TYPE_ENCRYPT);
    rule_req.mutable_rules(0)->mutable_sa_action()->mutable_enc_handle()->set_cb_id(1);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_rule_create(rule_req, &rule_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    del_enc_req.mutable_key_or_handle()->set_cb_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_delete(del_enc_req, &del_enc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    del_enc_req.mutable_key_or_handle()->set_cb_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_delete(del_enc_req, &del_enc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    del_enc_req.mutable_key_or_handle()->set_cb_id(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_saencrypt_delete(del_enc_req, &del_enc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    //Decrypt
    decrypt_spec.mutable_key_or_handle()->set_cb_id(2);
    decrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    decrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    decrypt_spec.set_decryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    decrypt_spec.set_rekey_dec_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    decrypt_spec.mutable_authentication_key()->set_key("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    decrypt_spec.mutable_decryption_key()->set_key("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
    decrypt_spec.set_salt(0xbbbbbbbb);
    decrypt_spec.set_spi(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_sadecrypt_create(decrypt_spec, &decrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    //::google::protobuf::uint64 decrypt_hdl = decrypt_resp.mutable_ipsec_sa_status()->ipsec_sa_handle();

    // Uncomment these to have gtest work for CLI
    decrypt_spec.mutable_key_or_handle()->set_cb_id(2);
    decrypt_spec.set_protocol(ipsec::IpsecProtocol::IPSEC_PROTOCOL_ESP);
    decrypt_spec.set_authentication_algorithm(ipsec::AuthenticationAlgorithm::AUTHENTICATION_AES_GCM);
    decrypt_spec.set_decryption_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    decrypt_spec.set_rekey_dec_algorithm(ipsec::EncryptionAlgorithm::ENCRYPTION_ALGORITHM_AES_GCM_256);
    decrypt_spec.mutable_authentication_key()->set_key("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    decrypt_spec.mutable_decryption_key()->set_key("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    decrypt_spec.set_salt(0xbbbbbbbb);
    decrypt_spec.set_spi(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_sadecrypt_update(decrypt_spec, &decrypt_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    dec_get_req.mutable_key_or_handle()->set_cb_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_sadecrypt_get(dec_get_req, &dec_get_rsp_msg);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ipsec_sa_decrypt_test_spec_dump(dec_get_rsp_msg);


    del_dec_req.mutable_key_or_handle()->set_cb_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::ipsec_sadecrypt_delete(del_dec_req, &del_dec_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    svc_reg(std::string("0.0.0.0:") + std::string("50054"), hal::HAL_FEATURE_SET_IRIS);
    hal::hal_wait();
#endif
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
