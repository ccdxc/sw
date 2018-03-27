#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/hal/src/mcast/multicast.hpp"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/multicast.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/security/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

using intf::InterfaceSpec;
using intf::InterfaceResponse;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponse;
using intf::LifSpec;
using intf::LifResponse;
using multicast::MulticastEntrySpec;
using multicast::MulticastEntryResponse;
using multicast::MulticastEntryDeleteRequest;
using multicast::MulticastEntryDeleteResponse;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentDeleteRequest;

class multicast_test : public hal_base_test {
protected:
    multicast_test() {
  }

  virtual ~multicast_test() {
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

// Create multicast entries and delete them
TEST_F(multicast_test, test2)
{
    hal_ret_t                       ret;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;
    VrfSpec                         ten_spec, ten_spec1;
    VrfResponse                     ten_rsp, ten_rsp1;
    SecurityProfileSpec             sp_spec, sp_spec1;
    SecurityProfileResponse         sp_rsp, sp_rsp1;
    LifSpec                         lif_spec;
    LifResponse                     lif_rsp;
    L2SegmentSpec                   l2seg_spec, l2seg_spec1;
    L2SegmentResponse               l2seg_rsp, l2seg_rsp1;
    L2SegmentDeleteRequest          del_req;
    L2SegmentDeleteResponse         del_rsp;
    InterfaceSpec                   enicif_spec;
    InterfaceResponse               enicif_rsp;
    NetworkSpec                     nw_spec;
    NetworkResponse                 nw_rsp;
    NetworkKeyHandle                *nkh = NULL;
    MulticastEntrySpec              mc_spec, mc_upd;
    MulticastEntryResponse          mc_rsp;
    MulticastEntryDeleteRequest     mc_del_req;
    MulticastEntryDeleteResponse    mc_del_rsp;

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

    // Create network
    nw_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    // Create an l2seg
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh = l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(10);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(100);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create a lif
    lif_spec.mutable_key_or_handle()->set_lif_id(21);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif21
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_key_or_handle()->set_interface_id(21);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(100);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif22
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_key_or_handle()->set_interface_id(22);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(100);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create enicif23
    enicif_spec.set_type(intf::IF_TYPE_ENIC);
    enicif_spec.mutable_key_or_handle()->set_interface_id(23);
    enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(21);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(100);
    enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::interface_create(enicif_spec, &enicif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    pre = hal_test_utils_collect_slab_stats();

    mc_spec.mutable_meta()->set_vrf_id(1);
    mc_spec.mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(100);
    mc_spec.add_oif_key_handles()->set_interface_id(21);
    mc_spec.add_oif_key_handles()->set_interface_id(22);
    for (int i = 0; i < 100; i++) {
        mc_spec.mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(0x01005E010101LL + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::multicastentry_create(mc_spec, &mc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    mc_upd.mutable_meta()->set_vrf_id(1);
    mc_upd.mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(100);
    mc_upd.add_oif_key_handles()->set_interface_id(22);
    mc_upd.add_oif_key_handles()->set_interface_id(23);
    for (int i = 0; i < 100; i++) {
        mc_upd.mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(0x01005E010101LL + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::multicastentry_update(mc_upd, &mc_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    mc_del_req.mutable_meta()->set_vrf_id(1);
    mc_del_req.mutable_key_or_handle()->mutable_key()->mutable_l2segment_key_handle()->set_segment_id(100);
    for (int i = 0; i < 100; i++) {
        mc_del_req.mutable_key_or_handle()->mutable_key()->mutable_mac()->set_group(0x01005E010101LL + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::multicastentry_delete(mc_del_req, &mc_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
