#include "nic/include/fte.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/src/l4lb.hpp"
#include "nic/proto/hal/interface.pb.h"
#include "nic/proto/hal/l2segment.pb.h"
#include "nic/proto/hal/tenant.pb.h"
#include "nic/proto/hal/nwsec.pb.h"
#include "nic/proto/hal/endpoint.pb.h"
#include "nic/proto/hal/session.pb.h"
#include "nic/proto/hal/l4lb.pb.h"
#include "nic/proto/hal/nw.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/include/hal_state.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using intf::InterfaceKeyHandle;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using tenant::TenantSpec;
using tenant::TenantResponse;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using intf::LifKeyHandle;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using endpoint::EndpointSpec;
using endpoint::EndpointResponse;
using session::SessionSpec;
using session::SessionResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;
using l4lb::L4LbServiceSpec;
using l4lb::L4LbServiceResponse;

void
hal_initialize()
{
    char 			cfg_file[] = "hal.json";
	char 			*cfg_path;
    std::string     full_path;
    hal::hal_cfg_t  hal_cfg = { 0 };

    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + std::string(cfg_file);
        std::cerr << "full path " << full_path << std::endl;
    } else {
        full_path = std::string(cfg_file);
    }

    // make sure cfg file exists
    if (access(full_path.c_str(), R_OK) < 0) {
        fprintf(stderr, "config file %s has no read permissions\n",
                full_path.c_str());
        exit(1);
    }

    printf("Json file: %s\n", full_path.c_str());

    if (hal::hal_parse_cfg(full_path.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        ASSERT_TRUE(0);
    }
    printf("Parsed cfg json file \n");

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL Initialized \n");
}


void
hal_uninitialize(void)
{
    // uninitialize HAL
    if (hal::hal_destroy() != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL UnInitialized \n");
}


class session_test : public ::testing::Test {
protected:
  session_test() {
  }

  virtual ~session_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
    // hal_initialize();
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_initialize();
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
    hal_uninitialize();
  }
};

// ----------------------------------------------------------------------------
// Creating a session
// ----------------------------------------------------------------------------
TEST_F(session_test, test1) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(1);
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(1);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(1);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(1);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(1);
    sess_spec.set_session_id(1);
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
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t sess_hdl = sess_rsp.mutable_status()->session_handle();
    HAL_TRACE_DEBUG("Session Handle: {}", sess_hdl);
}


// ----------------------------------------------------------------------------
// Creating a session with enicifs
// ----------------------------------------------------------------------------
TEST_F(session_test, test2) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1, ep_spec2, ep_spec3;
    EndpointResponse            ep_rsp, ep_rsp1, ep_rsp2, ep_rsp3;
    SessionSpec                 sess_spec, sess_spec2;
    SessionResponse             sess_rsp, sess_rsp2;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    InterfaceSpec               enicif_spec1, enicif_spec2;
    InterfaceResponse           enicif_rsp1, enicif_rsp2;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000001;
    ::google::protobuf::uint32  ip2 = 0x0a000002;
    ::google::protobuf::uint32  ip3 = 0x40020001;
    ::google::protobuf::uint32  ip4 = 0x40020002;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(2);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(2);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xb0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(2);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(100);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(2);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(22);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(200);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create a lif
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec1.mutable_meta()->set_tenant_id(2);
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(21);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec1.mutable_if_enic_info()->set_l2segment_id(21);
    enicif_spec1.mutable_if_enic_info()->set_encap_vlan_id(10);
    enicif_spec1.mutable_if_enic_info()->set_mac_address(0x000000000001);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl1 = enicif_rsp1.mutable_status()->if_handle();

    enicif_spec2.mutable_meta()->set_tenant_id(2);
    enicif_spec2.set_type(intf::IF_TYPE_ENIC);
    enicif_spec2.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec2.mutable_key_or_handle()->set_interface_id(22);
    enicif_spec2.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec2.mutable_if_enic_info()->set_l2segment_id(21);
    enicif_spec2.mutable_if_enic_info()->set_encap_vlan_id(20);
    enicif_spec2.mutable_if_enic_info()->set_mac_address(0x000000000002);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec2, &enicif_rsp2);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl2 = enicif_rsp2.mutable_status()->if_handle();


    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(2);
    ep_spec.set_l2_segment_handle(l2seg_hdl2);
    ep_spec.set_interface_handle(if_hdl1);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(2);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(if_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec2.mutable_meta()->set_tenant_id(2);
    ep_spec2.set_l2_segment_handle(l2seg_hdl2);
    ep_spec2.set_interface_handle(if_hdl1);
    ep_spec2.set_mac_address(0x00000001ABCD);
    ep_spec2.add_ip_address();
    ep_spec2.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec2.mutable_ip_address(0)->set_v4_addr(ip3);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec2, &ep_rsp2);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec3.mutable_meta()->set_tenant_id(2);
    ep_spec3.set_l2_segment_handle(l2seg_hdl2);
    ep_spec3.set_interface_handle(if_hdl2);
    ep_spec3.set_mac_address(0x000000011234);
    ep_spec3.add_ip_address();
    ep_spec3.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec3.mutable_ip_address(0)->set_v4_addr(ip4);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec3, &ep_rsp3);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(2);
    sess_spec.set_session_id(2);
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
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Session for coll
    sess_spec2.mutable_meta()->set_tenant_id(2);
    sess_spec2.set_session_id(3);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip3);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip4);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(4);
    sess_spec2.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec2.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip3);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip4);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(28750);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2);
    sess_spec2.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec2, &sess_rsp2);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Creating a session with enicifs
// ----------------------------------------------------------------------------
TEST_F(session_test, test3) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    InterfaceSpec               enicif_spec1, enicif_spec2;
    InterfaceResponse           enicif_rsp1, enicif_rsp2;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::std::string ipv6_ip1 = "00010001000100010001000100010001"; 
    ::std::string ipv6_ip2 = "00010001000100010001000100010002"; 

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(3);
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET6);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v6_addr("00010001000000000000000000000000");
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(3);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET6);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v6_addr("00010002000000000000000000000000");
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(3);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(301);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(3);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(32);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(302);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create a lif
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec1.mutable_meta()->set_tenant_id(3);
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(2);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(5);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec1.mutable_if_enic_info()->set_l2segment_id(31);
    enicif_spec1.mutable_if_enic_info()->set_encap_vlan_id(13);
    enicif_spec1.mutable_if_enic_info()->set_mac_address(0x000000000001);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl1 = enicif_rsp1.mutable_status()->if_handle();

    enicif_spec2.mutable_meta()->set_tenant_id(3);
    enicif_spec2.set_type(intf::IF_TYPE_ENIC);
    enicif_spec2.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(2);
    enicif_spec2.mutable_key_or_handle()->set_interface_id(6);
    enicif_spec2.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec2.mutable_if_enic_info()->set_l2segment_id(32);
    enicif_spec2.mutable_if_enic_info()->set_encap_vlan_id(14);
    enicif_spec2.mutable_if_enic_info()->set_mac_address(0x000000000002);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec2, &enicif_rsp2);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl2 = enicif_rsp2.mutable_status()->if_handle();


    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(3);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(if_hdl1);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET6);
    ep_spec.mutable_ip_address(0)->set_v6_addr(ipv6_ip1);  
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(3);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(if_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET6);
    ep_spec1.mutable_ip_address(0)->set_v6_addr(ipv6_ip2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(3);
    sess_spec.set_session_id(3);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_v6_addr(ipv6_ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_v6_addr(ipv6_ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v6_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v6_key()->mutable_tcp_udp()->set_sport(1000);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v6_key()->mutable_tcp_udp()->set_dport(2000);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_v6_addr(ipv6_ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_v6_addr(ipv6_ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v6_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v6_key()->mutable_tcp_udp()->set_sport(2000);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v6_key()->mutable_tcp_udp()->set_dport(1000);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Creating a session with uplinks for routing
// ----------------------------------------------------------------------------
TEST_F(session_test, test4) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec, l2seg_spec1;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec, sess_spec2;
    SessionResponse             sess_rsp, sess_rsp2;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x40020001;
    ::google::protobuf::uint32  ip2 = 0x40020002;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(4);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(4);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x40020000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(4);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x40020000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(4);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(41);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec1.mutable_meta()->set_tenant_id(4);
    l2seg_spec1.add_network_handle(nw_hdl1);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(42);
    l2seg_spec1.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(4);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(4);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(4);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(4);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(4);
    sess_spec.set_session_id(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(4);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2001);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1001);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    sess_spec2.mutable_meta()->set_tenant_id(4);
    sess_spec2.set_session_id(2);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(28750);
    sess_spec2.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2);
    sess_spec2.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec2.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1001);
    sess_spec2.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1002);
    sess_spec2.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec2, &sess_rsp2);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}
// ----------------------------------------------------------------------------
// Creating a session with uplinks for routing
// ----------------------------------------------------------------------------
TEST_F(session_test, test5) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec, l2seg_spec1;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(5);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(5);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xb0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(5);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec1.mutable_meta()->set_tenant_id(5);
    l2seg_spec1.add_network_handle(nw_hdl1);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(52);
    l2seg_spec1.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(5);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(5);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(5);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(5);
    sess_spec.set_session_id(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1001);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2001);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2001);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1001);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Creating a session with uplinks for routing and NAT
// ----------------------------------------------------------------------------
TEST_F(session_test, test6) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec, l2seg_spec1;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    ::google::protobuf::uint32  ip3 = 0x0a000005;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(6);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(6);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xb0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(6);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(61);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec1.mutable_meta()->set_tenant_id(6);
    l2seg_spec1.add_network_handle(nw_hdl1);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(62);
    l2seg_spec1.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(6);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(61);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(6);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(62);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    // CIP
    ep_spec.mutable_meta()->set_tenant_id(6);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
#if 0
    // VIP
    ep_spec1.mutable_meta()->set_tenant_id(6);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // PIP
    ep_spec1.mutable_meta()->set_tenant_id(6);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001235);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip3); 
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Session
    // CIP -> VIP ==> CIP -> PIP :: DNAT
    sess_spec.mutable_meta()->set_tenant_id(6);
    sess_spec.set_session_id(61);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1001);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2001);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_nat_type(session::NAT_TYPE_DNAT);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_dip()->set_ip_af(types::IP_AF_INET);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_dip()->set_v4_addr(ip3);

    // PIP -> CIP ==> VIP -> CIP :: SNAT
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip3);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2001);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1001);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_nat_type(session::NAT_TYPE_SNAT);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_sip()->set_ip_af(types::IP_AF_INET);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_sip()->set_v4_addr(ip2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Creating a session with drop
// ----------------------------------------------------------------------------
TEST_F(session_test, test7) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(7);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(7);
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(7);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(7);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(71);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(7);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(72);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(7);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(71);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(7);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(72);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(7);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(7);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(7);
    sess_spec.set_session_id(71);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1000);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2000);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_DROP);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2000);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1000);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // uint64_t sess_hdl = sess_rsp.mutable_status()->session_handle();
    // HAL_TRACE_DEBUG("Session Handle: {}", sess_hdl);
}

// ----------------------------------------------------------------------------
// Creating a session with uplinks for bridging and NAT
// ----------------------------------------------------------------------------
TEST_F(session_test, test8) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec, l2seg_spec1;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    L4LbServiceSpec             l4lb_spec;
    L4LbServiceResponse         l4lb_rsp;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    ::google::protobuf::uint32  ip3 = 0x0a000005;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(8);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(8);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(8);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xb0000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp1.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(8);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(81);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec1.mutable_meta()->set_tenant_id(8);
    l2seg_spec1.add_network_handle(nw_hdl1);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(82);
    l2seg_spec1.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(8);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(81);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(8);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(82);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    // CIP
    ep_spec.mutable_meta()->set_tenant_id(8);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
#if 0
    // VIP
    ep_spec1.mutable_meta()->set_tenant_id(6);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // PIP
    ep_spec1.mutable_meta()->set_tenant_id(8);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001235);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip3); 
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l4lb
    l4lb_spec.mutable_meta()->set_tenant_id(8);
    l4lb_spec.mutable_key_or_handle()->mutable_service_key()->mutable_service_ip_address()->
        set_ip_af(types::IP_AF_INET);
    l4lb_spec.mutable_key_or_handle()->mutable_service_key()->mutable_service_ip_address()->
        set_v4_addr(ip2);
    l4lb_spec.mutable_key_or_handle()->mutable_service_key()->set_ip_protocol((l4lb::L4LBProtocol)types::IPPROTO_TCP);
    l4lb_spec.mutable_key_or_handle()->mutable_service_key()->set_service_port(1000);
    l4lb_spec.set_service_mac(0x111111001235);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l4lbservice_create(l4lb_spec, &l4lb_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    // CIP -> VIP ==> CIP -> PIP :: DNAT
    sess_spec.mutable_meta()->set_tenant_id(8);
    sess_spec.set_session_id(81);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1001);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2001);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_nat_type(session::NAT_TYPE_DNAT);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_dip()->set_ip_af(types::IP_AF_INET);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_dip()->set_v4_addr(ip3);

    // PIP -> CIP ==> VIP -> CIP :: SNAT
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip3);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2001);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1001);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_nat_type(session::NAT_TYPE_SNAT);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_sip()->set_ip_af(types::IP_AF_INET);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->mutable_nat_sip()->set_v4_addr(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_nat_sport(1000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

}

// ----------------------------------------------------------------------------
// Creating a session
// ----------------------------------------------------------------------------
TEST_F(session_test, test9) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(9);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(9);
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(9);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(9);
    l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(91);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(9);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(92);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(9);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(9);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(9);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(9);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(9);
    sess_spec.set_session_id(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_ESP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_esp()->set_spi(0xAAAABBBB);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IPPROTO_ESP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_esp()->set_spi(0xBBBBAAAA);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    uint64_t sess_hdl = sess_rsp.mutable_status()->session_handle();
    HAL_TRACE_DEBUG("Session Handle: {}", sess_hdl);
}

TEST_F(session_test, test10) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec;
    InterfaceResponse           up_rsp;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    SessionSpec                 sess_spec;
    SessionResponse             sess_rsp;
    SecurityProfileSpec         sp_spec;
    SecurityProfileResponse     sp_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;
    InterfaceSpec               enicif_spec1, enicif_spec2;
    InterfaceResponse           enicif_rsp1, enicif_rsp2;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::std::string ipv6_ip1 = "00010001000100010001000100010001"; 
    ::std::string ipv6_ip2 = "00010001000100010001000100010002"; 

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(10);
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET6);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v6_addr("00010001000000000000000000000000");
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(10);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET6);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v6_addr("00010002000000000000000000000000");
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(10);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(101);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(301);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(10);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(102);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(302);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create a lif
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(102);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec1.mutable_meta()->set_tenant_id(10);
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(102);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(101);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec1.mutable_if_enic_info()->set_l2segment_id(101);
    enicif_spec1.mutable_if_enic_info()->set_encap_vlan_id(13);
    enicif_spec1.mutable_if_enic_info()->set_mac_address(0x000000000001);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl1 = enicif_rsp1.mutable_status()->if_handle();

    enicif_spec2.mutable_meta()->set_tenant_id(10);
    enicif_spec2.set_type(intf::IF_TYPE_ENIC);
    enicif_spec2.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(102);
    enicif_spec2.mutable_key_or_handle()->set_interface_id(102);
    enicif_spec2.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec2.mutable_if_enic_info()->set_l2segment_id(102);
    enicif_spec2.mutable_if_enic_info()->set_encap_vlan_id(14);
    enicif_spec2.mutable_if_enic_info()->set_mac_address(0x000000000002);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec2, &enicif_rsp2);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl2 = enicif_rsp2.mutable_status()->if_handle();


    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(10);
    ep_spec.set_l2_segment_handle(l2seg_hdl);
    ep_spec.set_interface_handle(if_hdl1);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET6);
    ep_spec.mutable_ip_address(0)->set_v6_addr(ipv6_ip1);  
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(10);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(if_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET6);
    ep_spec1.mutable_ip_address(0)->set_v6_addr(ipv6_ip2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(10);
    sess_spec.set_session_id(10);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_v6_addr(ipv6_ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_v6_addr(ipv6_ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v6_key()->set_ip_proto(types::IPPROTO_ESP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v6_key()->mutable_esp()->set_spi(0xAAAABBBB);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_sip()->set_v6_addr(ipv6_ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_ip_af(types::IP_AF_INET6);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v6_key()->mutable_dip()->set_v6_addr(ipv6_ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v6_key()->set_ip_proto(types::IPPROTO_ESP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v6_key()->mutable_esp()->set_spi(0xBBBBAAAA);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = fte::session_create(sess_spec, &sess_rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);

}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
