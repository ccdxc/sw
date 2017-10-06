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
using endpoint::EndpointUpdateRequest;
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


class endpoint_test : public ::testing::Test {
protected:
  endpoint_test() {
  }

  virtual ~endpoint_test() {
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
// Creating a endpoint
// ----------------------------------------------------------------------------
TEST_F(endpoint_test, test1) 
{
    hal_ret_t                   ret;
    TenantSpec                  ten_spec;
    TenantResponse              ten_rsp;
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


    // Create tenant
    ten_spec.mutable_key_or_handle()->set_tenant_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::tenant_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_meta()->set_tenant_id(1);
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_meta()->set_tenant_id(1);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
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
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_meta()->set_tenant_id(1);
    l2seg_spec.add_network_handle(nw_hdl1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
    l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_fabric_encap()->set_encap_value(12);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    // Create an uplink
    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(1);
    up_spec.mutable_if_uplink_info()->set_port_num(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    up_spec.mutable_meta()->set_tenant_id(1);
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(2);
    up_spec.mutable_if_uplink_info()->set_port_num(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
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
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    
#if 0
    ep_spec1.mutable_meta()->set_tenant_id(1);
    ep_spec1.set_l2_segment_handle(l2seg_hdl);
    ep_spec1.set_interface_handle(up_hdl2);
    ep_spec1.set_mac_address(0x000000001234);
    ep_spec1.add_ip_address();
    ep_spec1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Update with IP adds
    ep_req.mutable_meta()->set_tenant_id(1);
    ep_req.mutable_key_or_handle()->set_endpoint_handle(ep_rsp.endpoint_status().endpoint_handle());
    ep_req.set_l2_segment_handle(l2seg_hdl);
    ep_req.set_interface_handle(up_hdl2);
    ep_req.set_mac_address(0x00000000ABCD);
    ep_req.add_ip_address();
    ep_req.add_ip_address();
    ep_req.add_ip_address();
    ep_req.add_ip_address();
    ep_req.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    ep_req.mutable_ip_address(1)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_ip_address(1)->set_v4_addr(ip2);  // 10.0.0.1
    ep_req.mutable_ip_address(2)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_ip_address(2)->set_v4_addr(ip3);  // 10.0.0.1
    ep_req.mutable_ip_address(3)->set_ip_af(types::IP_AF_INET);
    ep_req.mutable_ip_address(3)->set_v4_addr(ip4);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req, &ep_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Update with IP deletes
    ep_req1.mutable_meta()->set_tenant_id(1);
    ep_req1.mutable_key_or_handle()->set_endpoint_handle(ep_rsp.endpoint_status().endpoint_handle());
    ep_req1.set_l2_segment_handle(l2seg_hdl);
    ep_req1.set_interface_handle(up_hdl2);
    ep_req1.set_mac_address(0x00000000ABCD);
    ep_req1.add_ip_address();
    // ep_req1.add_ip_address();
    // ep_req1.add_ip_address();
    // ep_req1.add_ip_address();
    ep_req1.mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_req1.mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    // ep_req1.mutable_ip_address(1)->set_ip_af(types::IP_AF_INET);
    // ep_req1.mutable_ip_address(1)->set_v4_addr(ip2);  // 10.0.0.1
    // ep_req1.mutable_ip_address(2)->set_ip_af(types::IP_AF_INET);
    // ep_req1.mutable_ip_address(2)->set_v4_addr(ip3);  // 10.0.0.1
    // ep_req1.mutable_ip_address(3)->set_ip_af(types::IP_AF_INET);
    // ep_req1.mutable_ip_address(3)->set_v4_addr(ip4);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_update(ep_req1, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
