#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "gen/proto/interface.pb.h"
#include "gen/proto/l2segment.pb.h"
#include "gen/proto/vrf.pb.h"
#include "gen/proto/nwsec.pb.h"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/hal.hpp"
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

class uplinkpc_test : public hal_base_test {
protected:
  uplinkpc_test() {
  }

  virtual ~uplinkpc_test() {
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
// Creating a uplinkpc
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test1)
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;
    InterfaceSpec       pc_spec;
    InterfaceResponse   pc_rsp;
    InterfaceKeyHandle  *ifkh = NULL;

    // Uplink create
    spec.set_type(intf::IF_TYPE_UPLINK);
    spec.mutable_key_or_handle()->set_interface_id(UPLINK_IF_ID_OFFSET + 1);
    spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
    // spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();

    // Uplink PC Create
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(2);
    ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
    ifkh->set_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Uplink PC Update
    pc_spec.mutable_if_uplink_pc_info()->set_native_l2segment_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_L2SEG_NOT_FOUND);

    // Verify that changing type in update is not allowed
    pc_spec.mutable_if_uplink_pc_info()->clear_native_l2segment_id();
    pc_spec.set_type(intf::IF_TYPE_UPLINK);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_INVALID_ARG);
    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}

// ----------------------------------------------------------------------------
// Creating muliple uplinkpcs
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test2)
{
    hal_ret_t            ret;
    InterfaceSpec       spec;
    InterfaceResponse   rsp;
    InterfaceSpec       pc_spec;
    InterfaceResponse   pc_rsp;
    InterfaceKeyHandle  *ifkh = NULL;

    for (int i = 1; i <= 8; i++) {
        spec.set_type(intf::IF_TYPE_UPLINK);

        spec.mutable_key_or_handle()->set_interface_id(100 + i);
        spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
        // spec.mutable_if_uplink_info()->set_native_l2segment_id(i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();

        pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
        pc_spec.mutable_key_or_handle()->set_interface_id(i+150);
        ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
        ifkh->set_if_handle(up_hdl);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

}

// ----------------------------------------------------------------------------
// Creating a uplinkpc and segements
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test3)
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
    InterfaceSpec                   pc_spec;
    InterfaceResponse               pc_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;
    InterfaceKeyHandle              *ifkh = NULL;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(10);
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
    ::google::protobuf::uint64 up_hdl = if_rsp.mutable_status()->if_handle();

    // Create l2segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(31);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    printf("ret: %d\n", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create Uplink PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(32);
    ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
    ifkh->set_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Adding L2segment on Uplink
    if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(31);
    if_l2seg_spec.mutable_if_key_handle()->set_interface_id(32);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
    hal::hal_cfg_db_close();
    printf("ret: %d api_status: %d\n", ret, if_l2seg_rsp.api_status());
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Release if_uplink_info
    // free spec.release_if_uplink_info();
}

// ----------------------------------------------------------------------------
// Creating and deleting a uplinkpc
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test4)
{
    hal_ret_t            ret;
    // InterfaceSpec       spec;
    // InterfaceResponse   rsp;
    InterfaceSpec       pc_spec;
    InterfaceResponse   pc_rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse         del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;

    pre = hal_test_utils_collect_slab_stats();
#if 0
    // create uplink if
    spec.set_type(intf::IF_TYPE_UPLINK);
    spec.mutable_key_or_handle()->set_interface_id(1);
    spec.mutable_if_uplink_info()->set_port_num(1);
    // spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ::google::protobuf::uint64 up_hdl = rsp.mutable_status()->if_handle();
#endif

    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(42);
    // pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    pc_spec.mutable_if_uplink_pc_info()->set_native_l2segment_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // delete uplink if
    del_req.mutable_key_or_handle()->set_interface_id(42);
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
//      - Create PC
//      - Create 8 Uplink Ifs
//      - Add 4 uplinkifs to PC
//      - Remove 4 ifs
//      - Add 4
//      - Remove 2 and add 4
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test5)
{
    hal_ret_t                        ret;
    InterfaceSpec                   spec;
    InterfaceResponse               rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse         del_rsp;
    InterfaceSpec                   pc_spec;
    InterfaceResponse               pc_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    int                             num_uplinkifs = 8;
    hal_handle_t                    upif_handles[8];
    InterfaceKeyHandle              *ifkh = NULL;

    pre = hal_test_utils_collect_slab_stats();

    for (int i = 1; i <= num_uplinkifs; i++) {
        // create uplink if
        spec.set_type(intf::IF_TYPE_UPLINK);
        spec.mutable_key_or_handle()->set_interface_id(500 + i);
        spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
        spec.mutable_if_uplink_info()->set_native_l2segment_id(1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        upif_handles[i-1] = rsp.mutable_status()->if_handle();
    }

    // Uplink PC Create
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    // pc_spec.mutable_if_uplink_pc_info()->add_member_if_handle(up_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add 8 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 1; i <= num_uplinkifs; i++) {
        ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
        ifkh->set_if_handle(upif_handles[i-1]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_update(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Remove 8 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 1; i <= num_uplinkifs; i++) {
        pc_spec.mutable_if_uplink_pc_info()->clear_member_if_key_handle();
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_update(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Add 4 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 1; i <= 4; i++) {
        ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
        ifkh->set_if_handle(upif_handles[i-1]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add other 4 uplinks to PC
    pc_spec.mutable_if_uplink_pc_info()->clear_member_if_key_handle();
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 5; i <= 8; i++) {
        ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
        ifkh->set_if_handle(upif_handles[i-1]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete all mbrs
    pc_spec.mutable_if_uplink_pc_info()->clear_member_if_key_handle();
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add other 4 uplinks to PC
    pc_spec.mutable_if_uplink_pc_info()->clear_member_if_key_handle();
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    for (int i = 5; i <= 8; i++) {
        ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
        ifkh->set_if_handle(upif_handles[i-1]);
    }
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_update(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete all mbr ifs
    for (int i = 5; i <= num_uplinkifs; i++) {
        // create uplink if
        del_req.mutable_key_or_handle()->set_interface_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OBJECT_IN_USE);
    }

    // Delete PC
    del_req.mutable_key_or_handle()->set_interface_id(500 + num_uplinkifs + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete all mbr ifs
    for (int i = 1; i <= num_uplinkifs; i++) {
        // create uplink if
        del_req.mutable_key_or_handle()->set_interface_id(500 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

// ----------------------------------------------------------------------------
// Test 6:
// - Create multiple uplinks
// - Create multiple l2segs and make them up on uplinks
// - Remove l2segs on uplinks
// - Delete l2segs
// - Delete uplinks
// ----------------------------------------------------------------------------
TEST_F(uplinkpc_test, test6)
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
    InterfaceDeleteResponse         del_rsp;
    InterfaceSpec                   pc_spec;
    InterfaceResponse               pc_rsp;
    SecurityProfileSpec             sp_spec;
    SecurityProfileResponse         sp_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    int                             num_uplinks = 8;
    int                             num_l2segs = 100;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    hal_handle_t                    upif_handles[8];
    NetworkKeyHandle                *nkh = NULL;
    InterfaceKeyHandle              *ifkh = NULL;
    InterfaceGetRequest             get_req;
    InterfaceGetResponseMsg         get_rsp_msg;

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

    pre = hal_test_utils_collect_slab_stats();

    // Create l2segments
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    for (int i = 1; i <= num_l2segs; i++) {
        // Create l2segment
        l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(6);
        l2seg_spec.mutable_key_or_handle()->set_segment_id(600 + i);
        l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        l2seg_spec.mutable_wire_encap()->set_encap_value(600 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Create Uplinks
    for (int i = 1; i <= num_uplinks; i++) {
        // Create uplink
        if_spec.set_type(intf::IF_TYPE_UPLINK);
        if_spec.mutable_key_or_handle()->set_interface_id(600 + i);
        if_spec.mutable_if_uplink_info()->set_port_num(PORT_NUM_1);
        if_spec.mutable_if_uplink_info()->set_native_l2segment_id(600+1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(if_spec, &if_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        upif_handles[i-1] = if_rsp.mutable_status()->if_handle();
    }

    // Uplink PC Create with 1 mbr
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(600 + num_uplinks + 1);
    ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
    ifkh->set_if_handle(upif_handles[0]);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(pc_spec, &pc_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Add 7 uplinks to PC
    pc_spec.set_type(intf::IF_TYPE_UPLINK_PC);
    pc_spec.mutable_key_or_handle()->set_interface_id(600 + num_uplinks + 1);
    for (int i = 2; i <= num_uplinks; i++) {
        ifkh = pc_spec.mutable_if_uplink_pc_info()->add_member_if_key_handle();
        ifkh->set_if_handle(upif_handles[i-1]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_update(pc_spec, &pc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Adding L2segment on UplinkPC
    for (int j = 1; j <= num_l2segs; j++) {
        if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(600+j);
        if_l2seg_spec.mutable_if_key_handle()->set_interface_id(600 + num_uplinks + 1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::add_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
        HAL_TRACE_DEBUG("ret:{}", ret);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hal::hal_cfg_db_close();
    }

    // Get interface by setting key.
    get_req.mutable_key_or_handle()->set_interface_id(600 + num_uplinks + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_get(get_req, &get_rsp_msg);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(get_rsp_msg.response_size() == 1);

    // Get interface without setting the key
    get_rsp_msg.clear_response();
    get_req.clear_key_or_handle();
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_get(get_req, &get_rsp_msg);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(get_rsp_msg.response_size() > 1);

    // Deleting L2segment on UplinkPC
    for (int j = 1; j <= num_l2segs; j++) {
        if_l2seg_spec.mutable_l2segment_key_or_handle()->set_segment_id(600+j);
        if_l2seg_spec.mutable_if_key_handle()->set_interface_id(600 + num_uplinks + 1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::del_l2seg_on_uplink(if_l2seg_spec, &if_l2seg_rsp);
        HAL_TRACE_DEBUG("ret:{}", ret);
        ASSERT_TRUE(ret == HAL_RET_OK);
        hal::hal_cfg_db_close();
    }

    // Delete PC
    del_req.mutable_key_or_handle()->set_interface_id(600 + num_uplinks + 1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete uplinks
    for (int i = 1; i <= num_uplinks; i++) {
        del_req.mutable_key_or_handle()->set_interface_id(600 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Delete l2segments
    for (int i = 1; i <= num_l2segs; i++) {
        l2seg_del_req.mutable_vrf_key_handle()->set_vrf_id(6);
        l2seg_del_req.mutable_key_or_handle()->set_segment_id(600 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::l2segment_delete(l2seg_del_req, &l2seg_del_rsp);
        hal::hal_cfg_db_close();
        HAL_TRACE_DEBUG("ret: {}", ret);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // Checking for slab leak
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
