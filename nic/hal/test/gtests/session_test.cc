#include <interface.hpp>
#include <endpoint.hpp>
#include <session.hpp>
#include <nwsec.hpp>
#include <interface.pb.h>
#include <l2segment.pb.h>
#include <tenant.pb.h>
#include <nwsec.pb.h>
#include <endpoint.pb.h>
#include <session.pb.h>
#include <hal.hpp>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

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


class session_test : public ::testing::Test {
protected:
  session_test() {
  }

  virtual ~session_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
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
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(12);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    ret = hal::interface_create(up_spec, &up_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    ret = hal::interface_create(up_spec, &up_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(1);
    ep_spec.set_l2_segment_handle(l2seg_hdl2);
    ep_spec.set_interface_handle(up_hdl2);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(1);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(1);
    sess_spec.set_session_id(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IP_PROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1000);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2000);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IP_PROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2000);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1000);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    ret = hal::session_create(sess_spec, &sess_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

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
    ::google::protobuf::uint32  ip1 = 0x0a000001;
    ::google::protobuf::uint32  ip2 = 0x0a000002;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(100);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(200);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create a lif
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    lif_spec.add_queues();
    lif_spec.add_queues();
    lif_spec.mutable_queues(0)->set_qtype(intf::LIF_QUEUE_TYPE_RX);
    lif_spec.mutable_queues(0)->set_queue_id(2);
    lif_spec.mutable_queues(1)->set_qtype(intf::LIF_QUEUE_TYPE_TX);
    lif_spec.mutable_queues(1)->set_queue_id(1);
    ret = hal::lif_create(lif_spec, &lif_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif
    enicif_spec1.mutable_meta()->set_tenant_id(1);
    enicif_spec1.set_type(intf::IF_TYPE_ENIC);
    enicif_spec1.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec1.mutable_key_or_handle()->set_interface_id(1);
    enicif_spec1.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec1.mutable_if_enic_info()->set_l2segment_id(1);
    enicif_spec1.mutable_if_enic_info()->set_encap_vlan_id(10);
    enicif_spec1.mutable_if_enic_info()->set_mac_address(0x000000000001);
    ret = hal::interface_create(enicif_spec1, &enicif_rsp1);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl1 = enicif_rsp1.mutable_status()->if_handle();

    enicif_spec2.mutable_meta()->set_tenant_id(1);
    enicif_spec2.set_type(intf::IF_TYPE_ENIC);
    enicif_spec2.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    enicif_spec2.mutable_key_or_handle()->set_interface_id(2);
    enicif_spec2.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec2.mutable_if_enic_info()->set_l2segment_id(1);
    enicif_spec2.mutable_if_enic_info()->set_encap_vlan_id(20);
    enicif_spec2.mutable_if_enic_info()->set_mac_address(0x000000000002);
    ret = hal::interface_create(enicif_spec2, &enicif_rsp2);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 if_hdl2 = enicif_rsp2.mutable_status()->if_handle();


    // Create 2 Endpoints
    ep_spec.mutable_meta()->set_tenant_id(1);
    ep_spec.set_l2_segment_handle(l2seg_hdl2);
    ep_spec.set_interface_handle(if_hdl1);
    ep_spec.set_mac_address(0x00000000ABCD);
    ep_spec.add_ip_address();
    ep_spec.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    ep_spec1.mutable_meta()->set_tenant_id(1);
    ep_spec1.set_l2_segment_handle(l2seg_hdl2);
    ep_spec1.set_interface_handle(if_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Create Session
    sess_spec.mutable_meta()->set_tenant_id(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IP_PROTO_TCP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(1000);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(2000);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->
        mutable_flow_info()->set_queue_type(intf::LIF_QUEUE_TYPE_TX);

    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->set_ip_proto(types::IP_PROTO_TCP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_sport(2000);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->
        mutable_v4_key()->mutable_tcp_udp()->set_dport(1000);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->
        mutable_flow_info()->set_queue_type(intf::LIF_QUEUE_TYPE_TX);
    ret = hal::session_create(sess_spec, &sess_rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
