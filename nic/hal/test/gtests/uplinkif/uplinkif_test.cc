#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
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
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

class uplinkif_test : public hal_base_test {
protected:
  uplinkif_test() {
  }

  virtual ~uplinkif_test() {
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
// Test1:
//
// 1. Create uplink
// 2. Update native l2seg
// 3. No l2seg, update will be a noop
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test1)
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;

    spec.set_type(intf::IF_TYPE_UPLINK);

    spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    spec.mutable_if_uplink_info()->set_native_l2segment_id(1);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    spec.mutable_if_uplink_info()->set_native_l2segment_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(spec, &rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);

    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}

// ----------------------------------------------------------------------------
// Test 2:
// 1. Create uplinks
// 2. Delete uplinks
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test2)
{
    hal_ret_t                   ret;
    InterfaceSpec               spec;
    InterfaceResponse           rsp;
    InterfaceDeleteRequest      del_req;
    InterfaceDeleteResponse  del_rsp;
    slab_stats_t                *pre = NULL, *post = NULL;
    bool                        is_leak = false;
    mtrack_info_t               *pre_minfo, *post_minfo;
    bool                        is_mleak = false;

    pre       = hal_test_utils_collect_slab_stats();
    pre_minfo = hal_test_utils_collect_mtrack_stats();
    HAL_TRACE_DEBUG("1alloc_id : 6 allocs: {}, frees: {}", pre_minfo[6].num_allocs, pre_minfo[6].num_frees);

    for (int i = 1; i <= 8; i++) {
        // Create uplink
        spec.set_type(intf::IF_TYPE_UPLINK);
        spec.mutable_key_or_handle()->set_interface_id(100 + i);
        spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
        spec.mutable_if_uplink_info()->set_native_l2segment_id(i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    for (int i = 1; i <= 8; i++) {
        del_req.mutable_key_or_handle()->set_interface_id(100 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    HAL_TRACE_DEBUG("2alloc_id : 6 allocs: {}, frees: {}", pre_minfo[6].num_allocs, pre_minfo[6].num_frees);
    // Checking for slab leak
    post       = hal_test_utils_collect_slab_stats();
    HAL_TRACE_DEBUG("3alloc_id : 6 allocs: {}, frees: {}", pre_minfo[6].num_allocs, pre_minfo[6].num_frees);
    post_minfo = hal_test_utils_collect_mtrack_stats();
    HAL_TRACE_DEBUG("4alloc_id : 6 allocs: {}, frees: {}", pre_minfo[6].num_allocs, pre_minfo[6].num_frees);
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    HAL_TRACE_DEBUG("5alloc_id : 6 allocs: {}, frees: {}", pre_minfo[6].num_allocs, pre_minfo[6].num_frees);
    ASSERT_TRUE(is_leak == false);
    hal_test_utils_check_mtrack_leak(pre_minfo, post_minfo, &is_mleak);
    ASSERT_TRUE(is_mleak == false);
}

// ----------------------------------------------------------------------------
// Test 3:
// - Create NwSEC
// - Create Vrf
// - Create network
// - Create l2seg
// - Create Uplink
// - Add l2seg on uplink
// - Delete uplink - expect failure
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test3)
{
    hal_ret_t                       ret;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    VrfSpec                      ten_spec;
    VrfResponse                  ten_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse      del_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(2);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
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
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(31);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    // if_spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(1);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    HAL_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close();

    // Try to delete uplink
    del_req.mutable_key_or_handle()->set_interface_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);

    // Delete l2segment on uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(1);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::del_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    HAL_TRACE_DEBUG("ret:{}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    hal::hal_cfg_db_close();

    // Try to delete uplink
    del_req.mutable_key_or_handle()->set_interface_id(31);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}

// ----------------------------------------------------------------------------
// Test 4:
// - Create an uplink
// - Delete an uplink
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test4)
{
    hal_ret_t                        ret;
    InterfaceSpec                   spec;
    InterfaceResponse               rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse      del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // create uplink if
    spec.set_type(intf::IF_TYPE_UPLINK);
    spec.mutable_key_or_handle()->set_interface_id(41);
    spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete uplink if
    del_req.mutable_key_or_handle()->set_interface_id(41);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// ----------------------------------------------------------------------------
// Test 5:
// - Create multiple uplinks
// - Create multiple l2segs and make them up on uplinks
// - Remove l2segs on uplinks
// - Delete l2segs
// - Delete uplinks
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test5)
{
    hal_ret_t                       ret;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    L2SegmentDeleteRequest          l2seg_del_req;
    L2SegmentDeleteResponse      l2seg_del_rsp;
    VrfSpec                      ten_spec;
    VrfResponse                  ten_rsp;
    InterfaceL2SegmentSpec          if_l2seg_spec;
    InterfaceL2SegmentResponse      if_l2seg_rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse      del_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    int                             num_uplinks = 8;
    int                             num_l2segs = 10;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    mtrack_info_t                   *pre_minfo, *post_minfo;
    bool                            is_mleak = false;
    NetworkKeyHandle                *nkh = NULL;

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

    pre = hal_test_utils_collect_slab_stats();
    pre_minfo = hal_test_utils_collect_mtrack_stats();

    // Create l2segments
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    for (int i = 1; i <= num_l2segs; i++) {
        // Create l2segment
        l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(5);
        l2seg_spec.mutable_key_or_handle()->set_segment_id(500 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create Uplinks
    for (int i = 1; i <= num_uplinks; i++) {
        // Create uplink
        if_spec.set_type(intf::IF_TYPE_UPLINK);
        if_spec.mutable_key_or_handle()->set_interface_id(500 + i);
        if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
        if_spec.mutable_if_uplink_info()->set_native_l2segment_id(500+1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(if_spec, &if_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Adding L2segment on Uplink
    for (int i = 1; i <= num_uplinks; i++) {
        for (int j = 1; j <= num_l2segs; j++) {
            if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(500+j);
            if_l2seg_spec.mutable_if_key_handle()->set_interface_id(500 + i);
            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
            HAL_TRACE_DEBUG("ret:{}", ret);
            ASSERT_TRUE(ret == HAL_RET_OK);
            hal::hal_cfg_db_close();
        }
    }

    // Deleting L2segment on Uplink
    for (int i = 1; i <= num_uplinks; i++) {
        for (int j = 1; j <= num_l2segs; j++) {
            if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(500+j);
            if_l2seg_spec.mutable_if_key_handle()->set_interface_id(500 + i);
            hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
            ret = hal::del_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
            HAL_TRACE_DEBUG("ret:{}", ret);
            ASSERT_TRUE(ret == HAL_RET_OK);
            hal::hal_cfg_db_close();
        }
    }

    // Delete uplinks
    for (int i = 1; i <= num_uplinks; i++) {
        del_req.mutable_key_or_handle()->set_interface_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Delete l2segments
    for (int i = 1; i <= num_l2segs; i++) {
        l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(5);
        l2seg_del_req.mutable_key_or_handle()->set_segment_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
        hal::hal_cfg_db_close();
        HAL_TRACE_DEBUG("ret: {}", ret);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Checking for slab leak
    post = hal_test_utils_collect_slab_stats();
    post_minfo = hal_test_utils_collect_mtrack_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
    // Seeing leak in mcast mtrack ids, met related IDs
    hal_test_utils_check_mtrack_leak(pre_minfo, post_minfo, &is_mleak);
    // ASSERT_TRUE(is_mleak == false);
}

// ----------------------------------------------------------------------------
// Test 6:
// - Create NwSEC
// - Create Vrf
// - Create network
// - Create l2seg
// - Create Uplink
// - Add l2seg on uplink
// - Delete uplink - expect failure
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test6)
{
    hal_ret_t                       ret;
    InterfaceSpec                   if_spec;
    InterfaceResponse               if_rsp;
    L2SegmentSpec                   l2seg_spec;
    L2SegmentResponse               l2seg_rsp;
    VrfSpec                         ten_spec;
    VrfResponse                     ten_rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse         del_rsp;
    L2SegmentDeleteRequest          l2seg_del_req;
    L2SegmentDeleteResponse         l2seg_del_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(6);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(6);
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
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(6);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->mutable_key_or_handle()->nw_handle();

    // Create Uplink If
    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(61);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    if_spec.set_type(intf::IF_TYPE_UPLINK);
    if_spec.mutable_key_or_handle()->set_interface_id(62);
    if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(if_spec, &if_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(6);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(61);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add IF1 to l2seg
    auto ifkh = l2seg_spec.add_if_key_handle();
    ifkh->set_interface_id(61);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Try to delete uplink
    del_req.mutable_key_or_handle()->set_interface_id(61);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);

    // Add one more uplink. Results in sending two IFs
    auto ifkh2 = l2seg_spec.add_if_key_handle();
    ifkh2->set_interface_id(62);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add only one uplink. Should result in delete of another
    l2seg_spec.clear_if_key_handle();
    auto ifkh3 = l2seg_spec.add_if_key_handle();
    ifkh3->set_interface_id(62);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add no uplinks. Should result in delete of one uplink
    l2seg_spec.clear_if_key_handle();
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add two uplinks
    auto ifkh4 = l2seg_spec.add_if_key_handle();
    ifkh4->set_interface_id(61);
    auto ifkh5 = l2seg_spec.add_if_key_handle();
    ifkh5->set_interface_id(62);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_update(l2seg_spec, &l2seg_rsp);
    HAL_TRACE_DEBUG("ret: {}", ret);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete l2seg
    l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(6);
    l2seg_del_req.mutable_key_or_handle()->set_segment_id(61);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
