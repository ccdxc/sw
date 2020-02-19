#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

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
using l2segment::L2SegmentDeleteRequest;
using vrf::VrfSpec;
using vrf::VrfResponse;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponseMsg;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

class l2seg_test : public hal_base_test {
protected:
  l2seg_test() {
  }

  virtual ~l2seg_test() {
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

// Update l2seg test with enicifs
TEST_F(l2seg_test, test1)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec, l2seg_spec1;
    L2SegmentResponse               l2seg_rsp, l2seg_rsp1;
    L2SegmentDeleteRequest          del_req;
    L2SegmentDeleteResponse      del_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(10);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(11);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    pre = hal_test_utils_collect_slab_stats();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete l2 segment failure
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_segment_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);

    // Delete L2 segment
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_segment_id(21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create L2 Segment
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(2);
    nkh = l2seg_spec1.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(21);
    l2seg_spec1.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete L2 segment
    del_req.mutable_vrf_key_handle()->set_vrf_id(2);
    del_req.mutable_key_or_handle()->set_segment_id(21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);


#if 0
    // Create enicif
    enicif_spec.mutable_vrf_key_handle()->set_vrf_id(2);
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec.mutable_key_or_handle()->set_interface_id(21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->set_l2segment_id(21);
    enicif_spec.mutable_if_enic_info()->set_encap_vlan_id(20);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

#if 0
    // Update vrf
    ten_spec1.mutable_key_or_handle()->set_vrf_id(2);
    ten_spec1.mutable_security_key_handle()->set_profile_handle(nwsec_hdl1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_update(ten_spec1, &ten_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif
}

// Create 100 l2segs and delete them
TEST_F(l2seg_test, test2)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec, l2seg_spec1;
    L2SegmentResponse               l2seg_rsp, l2seg_rsp1;
    L2SegmentDeleteRequest          del_req;
    L2SegmentDeleteResponse      del_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(21);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(22);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    pre = hal_test_utils_collect_slab_stats();

    // Create 100 l2segs
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    for (int i = 0; i < 100; i++) {
        l2seg_spec.mutable_key_or_handle()->set_segment_id(100+i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Delete 100 l2segs
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    for (int i = 0; i < 100; i++) {
        del_req.mutable_key_or_handle()->set_segment_id(100+i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        HAL_TRACE_DEBUG("ret: {}", ret);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

//-----------------------------------------------------------------------------
// l2seg update with no change
//-----------------------------------------------------------------------------
TEST_F(l2seg_test, test3)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec, ten_spec1;
    VrfResponse                  ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    // VrfDeleteRequest             del_req;
    // VrfDeleteResponseMsg         del_rsp;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec, l2seg_spec1;
    L2SegmentResponse               l2seg_rsp, l2seg_rsp1;
    L2SegmentDeleteRequest          del_req;
    L2SegmentDeleteResponse      del_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    // slab_stats_t                    *pre = NULL, *post = NULL;
    // bool                            is_leak = false;
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(31);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(32);
    sp_spec1.set_ipsg_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec1, &sp_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl1 = sp_rsp1.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(3);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

#if 0
    // Delete vrf
    del_req.mutable_vrf_key_handle()->set_vrf_id(1);
    del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
#endif

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(3);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // pre = hal_test_utils_collect_slab_stats();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(3);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create another l2segment with no change
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(3);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec1.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Now use the same key with a non-existent VRF.
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(1000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);

    // Set back the original VRF id, and add a non-existent network key.
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(3);
    nkh = l2seg_spec1.add_network_key_handle();
    nkh->set_nw_handle(10000);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);

    // Clear the network, and add in a GIPO IPv4 address which is different.
    l2seg_spec1.clear_network_key_handle();
    l2seg_spec1.mutable_gipo()->set_ip_af(types::IP_AF_INET);
    l2seg_spec1.mutable_gipo()->set_v4_addr(0xa0000000);
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);

    // Clear the GIPO field
    l2seg_spec1.clear_gipo();
    l2seg_spec1.mutable_gipo()->set_ip_af(types::IP_AF_INET6);
    l2seg_spec1.mutable_gipo()->set_v6_addr("00010001000100010001000100010001");
    ret = hal::l2segment_create(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);
    l2seg_spec1.clear_gipo();

    // Update to l2segment with no change
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(3);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// l2segments with multiple networks
TEST_F(l2seg_test, test4)
{
    hal_ret_t                       ret;
    VrfSpec                      ten_spec;
    VrfResponse                  ten_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    VrfDeleteRequest             ten_del_req;
    VrfDeleteResponseMsg         ten_del_rsp;
    L2SegmentSpec                   l2seg_spec, l2seg_spec1;
    L2SegmentResponse               l2seg_rsp, l2seg_rsp1;
    L2SegmentDeleteRequest          l2seg_del_req;
    L2SegmentDeleteResponse      l2seg_del_rsp;
    NetworkSpec                     nw_spec, nw_spec_v6;
    NetworkResponse                 nw_rsp, nw_rsp_v6;
    NetworkDeleteRequest            nw_del_req;
    NetworkDeleteResponse           nw_del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    hal_handle_t                    nw_v4handles[100], nw_v6handles[100];
    std::string                     ipv6_ip1 = "00010001000100010001000100010001";
    std::string                     ipv6_ip2 = "00010001000100010001000100010001";
    std::string                     v6_pattern = "010101";
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(4);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(4);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    pre = hal_test_utils_collect_slab_stats();

    // Create network
    for (int i = 0; i < 10; i++) {
        // Create network
        nw_spec.set_rmac(0x0000DEADBEEF);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000 + i);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(4);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_create(nw_spec, &nw_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        nw_v4handles[i] = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

        // Create v6 network
        nw_spec_v6.set_rmac(0x0000DEADBEEF);
        nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
        nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET6);
        nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v6_addr(ipv6_ip1);
        nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(4);
        ipv6_ip1.replace(i, 5, v6_pattern);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_create(nw_spec_v6, &nw_rsp_v6);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        nw_v6handles[i] = nw_rsp_v6.mutable_status()->mutable_key_or_handle()->nw_handle();
    }

    // Create 10 l2segs
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(4);
    for (int i = 0; i < 10; i++) {
        // Add nw handles
        nkh = l2seg_spec.add_network_key_handle();
        nkh->set_nw_handle(nw_v4handles[i]);
    }
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    for (int i = 0; i < 10; i++) {
        l2seg_spec.mutable_key_or_handle()->set_segment_id(400+i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Update l2segs with more network
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(4);
    for (int i = 0; i < 10; i++) {
        // Add nw handles
        nkh = l2seg_spec1.add_network_key_handle();
        nkh->set_nw_handle(nw_v6handles[i]);
    }
    nkh = l2seg_spec1.add_network_key_handle();
    nkh->set_nw_handle(nw_v4handles[0]);
    nkh = l2seg_spec1.add_network_key_handle();
    nkh->set_nw_handle(nw_v4handles[1]);
    l2seg_spec1.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec1.mutable_wire_encap()->set_encap_value(10);
    for (int i = 0; i < 10; i++) {
        l2seg_spec1.mutable_key_or_handle()->set_segment_id(400+i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    for (int i = 0; i < 10; i++) {
#if 0
        // Remove network
        nw_del_req.mutable_vrf_key_handle()->set_vrf_id(4);
        nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_v4handles[i]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_delete(nw_del_req, &nw_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);
#endif

        // Remove v6 network
        nw_del_req.mutable_vrf_key_handle()->set_vrf_id(4);
        nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_v6handles[i]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_delete(nw_del_req, &nw_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);
    }

    // Delete 10 l2segs
    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(4);
    for (int i = 0; i < 10; i++) {
        l2seg_del_req.mutable_key_or_handle()->set_segment_id(400+i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
        hal::hal_cfg_db_close();
        HAL_TRACE_DEBUG("ret: {}", ret);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    for (int i = 0; i < 10; i++) {
        // Remove network
        nw_del_req.mutable_vrf_key_handle()->set_vrf_id(4);
        nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_v4handles[i]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_delete(nw_del_req, &nw_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);

        // Remove v6 network
        nw_del_req.mutable_vrf_key_handle()->set_vrf_id(4);
        nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_v6handles[i]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_delete(nw_del_req, &nw_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Memleak check
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// ----------------------------------------------------------------------------
// L2Segment -ve test cases
// ----------------------------------------------------------------------------
TEST_F(l2seg_test, test5)
{
    hal_ret_t                        ret = HAL_RET_OK;
    L2SegmentSpec                    l2seg_spec, l2seg_spec1, l2seg_spec2, infra_spec;
    L2SegmentResponse                l2seg_rsp, l2seg_rsp1, l2seg_rsp2;
    SecurityProfileSpec              sp_spec;
    SecurityProfileResponse          sp_rsp;
    SecurityProfileDeleteRequest     sp_del_req;
    SecurityProfileDeleteResponse    sp_del_rsp;
    VrfSpec                          ten_spec;
    VrfResponse                      ten_rsp;
    VrfDeleteRequest                 ten_del_req;
    VrfDeleteResponse                ten_del_rsp;
    NetworkSpec                      nw_spec;
    NetworkResponse                  nw_rsp;
    NetworkDeleteRequest             nw_del_req;
    NetworkDeleteResponse            nw_del_rsp;
    L2SegmentDeleteRequest           l2seg_del_req;
    L2SegmentDeleteResponse          l2seg_del_rsp;
    L2SegmentGetRequest              l2seg_get_req;
    L2SegmentGetResponseMsg          l2seg_get_rsp;
    slab_stats_t                     *pre = NULL, *post = NULL;
    bool                             is_leak = false;
    int                              num_l2segs = 2048;
    NetworkKeyHandle                 *nkh = NULL;

    pre = hal_test_utils_collect_slab_stats();

    // Create l2seg with vrf id doesnt exist
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    // l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);

    // Create l2seg with no meta
    // l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    // l2seg_spec.add_network_handle(nw_hdl);
    l2seg_spec2.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec2.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec2.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec2, &l2seg_rsp2);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(5);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(5);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(51);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();


    // Create l2seg with network that doesnt exist
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl + 1);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create l2seg with no access or fabric encap set
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // There are no infra l2segs anymore
#if 0
    // Create Infra l2seg with non-infra vrf
    infra_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    infra_spec.clear_network_key_handle();
    nkh = infra_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    infra_spec.mutable_key_or_handle()->set_segment_id(85);
    infra_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    infra_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(infra_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);
#endif

    // Create Infra vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(85);
    ten_spec.set_vrf_type(types::VRF_TYPE_INFRA);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Infra l2seg with Infra vrf
    infra_spec.mutable_vrf_key_handle()->set_vrf_id(85);
    infra_spec.clear_network_key_handle();
    nkh = infra_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    infra_spec.mutable_key_or_handle()->set_segment_id(85);
    infra_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    infra_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(infra_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a second Infra L2Seg
    infra_spec.mutable_vrf_key_handle()->set_vrf_id(85);
    infra_spec.clear_network_key_handle();
    nkh = infra_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    infra_spec.mutable_key_or_handle()->set_segment_id(86);
    infra_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    infra_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(infra_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Create l2seg
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec.clear_network_key_handle();
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->key_or_handle().l2segment_handle();

    // Create l2seg which already exists, this should be idempotent and return the old
    // handle
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(l2seg_hdl == l2seg_rsp.mutable_l2segment_status()->key_or_handle().l2segment_handle());

    // Create the max number of HAL_MAX_HW_L2SEGMENTS
    // Create l2seg resulting in no resource
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec.clear_network_key_handle();
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    for (int i = 0; i < num_l2segs; i++) {
        l2seg_spec.mutable_key_or_handle()->set_segment_id(500 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(10);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        HAL_TRACE_DEBUG("ret: {}", ret);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK || ret == HAL_RET_NO_RESOURCE);
    }

    // Update l2seg with no key or handle
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Update l2seg with handle that doesn't exist
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec1.mutable_key_or_handle()->set_l2segment_handle(0xFFFFFFF);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);

    // Update l2seg with invalid vrf id
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(HAL_VRF_ID_INVALID);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Update l2seg with different vrf not allowed
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(6);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_VRF_NOT_FOUND);

    // Update l2seg with different vrf not allowed
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(51);
    l2seg_spec1.mutable_key_or_handle()->set_segment_id(51);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Update l2seg with the same networks as when its created

    // Update l2seg with a handle
    l2seg_spec1.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_spec1.mutable_key_or_handle()->set_l2segment_handle(l2seg_hdl);
    l2seg_spec1.set_mcast_fwd_policy(l2segment::MULTICAST_FWD_POLICY_DROP);
    l2seg_spec1.set_bcast_fwd_policy(l2segment::BROADCAST_FWD_POLICY_DROP);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec1, &l2seg_rsp1);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete l2seg with no key or handle
    // l2seg_del_req.mutable_key_or_handle()->set_segment_id(400+i);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);

    // Get of l2seg with no key or handle and no vrf id
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_get(l2seg_get_req, &l2seg_get_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(l2seg_get_rsp.response_size() > 0);

    // Get of l2seg without key or handle but with vrf id set.
    l2seg_get_rsp.clear_response();
    l2seg_get_req.mutable_vrf_key_handle()->set_vrf_id(5);
    // l2seg_get_req.mutable_key_or_handle()->set_segment_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_get(l2seg_get_req, &l2seg_get_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(l2seg_get_rsp.response_size() > 0);

    // Get of l2seg id for which l2seg is not present
    l2seg_get_rsp.clear_response();
    l2seg_get_req.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_get_req.mutable_key_or_handle()->set_segment_id(50);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_get(l2seg_get_req, &l2seg_get_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);

    // Get of l2seg handle for which l2seg is not present
    l2seg_get_rsp.clear_response();
    l2seg_get_req.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_get_req.mutable_key_or_handle()->set_l2segment_handle(0xFFFFFF);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_get(l2seg_get_req, &l2seg_get_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);

    // Delete all l2segs
    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(5);
    l2seg_del_req.mutable_key_or_handle()->set_segment_id(51);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete infra l2seg
    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(85);
    l2seg_del_req.mutable_key_or_handle()->set_segment_id(85);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    for (int i = 0; i < num_l2segs; i++) {
        l2seg_del_req.mutable_key_or_handle()->set_segment_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
        hal::hal_cfg_db_close();
        HAL_TRACE_DEBUG("ret: {}", ret);
        // ASSERT_TRUE(ret == HAL_RET_OK);
        ASSERT_TRUE(ret == HAL_RET_OK || ret == HAL_RET_L2SEG_NOT_FOUND);
    }

    // Delete network
    nw_del_req.mutable_vrf_key_handle()->set_vrf_id(5);
    nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_delete(nw_del_req, &nw_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete vrfs
    ten_del_req.mutable_key_or_handle()->set_vrf_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(ten_del_req, &ten_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ten_del_req.mutable_key_or_handle()->set_vrf_id(51);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(ten_del_req, &ten_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    ten_del_req.mutable_key_or_handle()->set_vrf_id(85);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(ten_del_req, &ten_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete nwsec
    sp_del_req.mutable_key_or_handle()->set_profile_id(5);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sp_del_req, &sp_del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// Testing pinned uplink selection
TEST_F(l2seg_test, test6)
{
    hal_ret_t           ret = HAL_RET_OK;
    VrfSpec             vrf_spec;
    VrfResponse         vrf_rsp;
    InterfaceSpec       if_spec;
    InterfaceResponse   if_rsp;
    L2SegmentSpec       l2seg_spec;
    L2SegmentResponse   l2seg_resp;

    // Create vrf
    vrf_spec.mutable_key_or_handle()->set_vrf_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(vrf_spec, &vrf_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create uplinks
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 61);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.Clear();
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 62);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2seg with uplinks
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(6);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(61);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    l2seg_spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 61);
    l2seg_spec.add_if_key_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 62);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_resp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Simulating port event
    ret = hal::if_port_oper_state_process_event(5, port_event_t::PORT_EVENT_LINK_DOWN);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = hal::if_port_oper_state_process_event(1, port_event_t::PORT_EVENT_LINK_DOWN);
    ASSERT_TRUE(ret == HAL_RET_OK);

    ret = hal::if_port_oper_state_process_event(5, port_event_t::PORT_EVENT_LINK_UP);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
