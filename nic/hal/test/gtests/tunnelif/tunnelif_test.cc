#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/endpoint.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "gen/proto/nw.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
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
using nw::NetworkSpec;
using nw::NetworkResponse;

class tunnelif_test : public hal_base_test {
protected:
  tunnelif_test() {
  }

  virtual ~tunnelif_test() {
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
    hal_test_utils_slab_disable_delete();
  }

};

// ----------------------------------------------------------------------------
// Creating a useg tunnelif
// ----------------------------------------------------------------------------
TEST_F(tunnelif_test, test1)
{
    hal_ret_t                   ret;
    VrfSpec                  ten_spec;
    VrfResponse              ten_rsp;
    L2SegmentSpec               l2seg_spec;
    L2SegmentResponse           l2seg_rsp;
    InterfaceSpec               up_spec, tnnl_spec, tnnl_spec1;
    InterfaceResponse           up_rsp, tnnl_rsp, tnnl_rsp1;
    EndpointSpec                ep_spec, ep_spec1;
    EndpointResponse            ep_rsp, ep_rsp1;
    NetworkSpec                 nw_spec, nw_spec1;
    NetworkResponse             nw_rsp, nw_rsp1;
    ::google::protobuf::uint32  ip1 = 0x0a000003;
    ::google::protobuf::uint32  ip2 = 0x0a000004;
    NetworkKeyHandle            *nkh = NULL;

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

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

    // Create an uplink
    up_spec.set_type(intf::IF_TYPE_UPLINK);
    up_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    up_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(up_spec, &up_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

    // Create 2 Endpoints
    ep_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD);
    ep_spec.mutable_endpoint_attrs()->add_ip_address();
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip1);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec, &ep_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ep_spec1.mutable_vrf_key_handle()->set_vrf_id(1);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
    ep_spec1.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl);
    ep_spec1.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x000000001234);
    ep_spec1.mutable_endpoint_attrs()->add_ip_address();
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_ip_af(types::IP_AF_INET);
    ep_spec1.mutable_endpoint_attrs()->mutable_ip_address(0)->set_v4_addr(ip2);  // 10.0.0.1
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::endpoint_create(ep_spec1, &ep_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // tunnel create
    tnnl_spec.mutable_key_or_handle()->set_interface_id(2);
    tnnl_spec.set_type(intf::IF_TYPE_TUNNEL);
    tnnl_spec.mutable_if_tunnel_info()->mutable_vrf_key_handle()->set_vrf_id(1);
    tnnl_spec.mutable_if_tunnel_info()->set_encap_type(intf::IF_TUNNEL_ENCAP_TYPE_VXLAN);
    tnnl_spec.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_local_tep()->set_ip_af(types::IP_AF_INET);
    tnnl_spec.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_local_tep()->set_v4_addr(0x0a000000);
    tnnl_spec.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_remote_tep()->set_ip_af(types::IP_AF_INET);
    tnnl_spec.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_remote_tep()->set_v4_addr(0x0a000003);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(tnnl_spec, &tnnl_rsp);
    HAL_TRACE_DEBUG("ret:{}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // tunnel create
    tnnl_spec1.mutable_key_or_handle()->set_interface_id(12);
    tnnl_spec1.set_type(intf::IF_TYPE_TUNNEL);
    tnnl_spec1.mutable_if_tunnel_info()->mutable_vrf_key_handle()->set_vrf_id(1);
    tnnl_spec1.mutable_if_tunnel_info()->set_encap_type(intf::IF_TUNNEL_ENCAP_TYPE_VXLAN);
    tnnl_spec1.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_local_tep()->set_ip_af(types::IP_AF_INET);
    tnnl_spec1.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_local_tep()->set_v4_addr(0x0a000000);
    tnnl_spec1.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_remote_tep()->set_ip_af(types::IP_AF_INET);
    tnnl_spec1.mutable_if_tunnel_info()->mutable_vxlan_info()->mutable_remote_tep()->set_v4_addr(0x0a000004);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(tnnl_spec1, &tnnl_rsp1);
    HAL_TRACE_DEBUG("ret:{}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
#endif
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
