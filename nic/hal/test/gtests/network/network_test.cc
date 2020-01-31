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

class nw_test : public hal_base_test {
protected:
  nw_test() {
  }

  virtual ~nw_test() {
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

// Update networktest with enicifs
TEST_F(nw_test, test1)
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


    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(4);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Create nwsec
    sp_spec1.mutable_key_or_handle()->set_profile_id(5);
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

    pre = hal_test_utils_collect_slab_stats();

    for (int i = 0; i < 10; i++) {
        // Create network
        nw_spec.set_rmac(0x0000DEADBEEF);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000 + i);
        nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
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
        nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
        ipv6_ip1.replace(i, 5, v6_pattern);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_create(nw_spec_v6, &nw_rsp_v6);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        nw_v6handles[i] = nw_rsp_v6.mutable_status()->mutable_key_or_handle()->nw_handle();

    }

    // Duplicate create
    // Create network
    nw_spec.set_rmac(0x0000DEADBEEF);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0xa0000000);
    nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);

    // Create v6 network
    nw_spec_v6.set_rmac(0x0000DEADBEEF);
    nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(32);
    nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET6);
    nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v6_addr(ipv6_ip2);
    nw_spec_v6.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec_v6, &nw_rsp_v6);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_ENTRY_EXISTS);

    for (int i = 0; i < 10; i++) {
        // Remove network
        nw_del_req.mutable_vrf_key_handle()->set_vrf_id(1);
        nw_del_req.mutable_key_or_handle()->set_nw_handle(nw_v4handles[i]);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::network_delete(nw_del_req, &nw_del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);

        // Remove v6 network
        nw_del_req.mutable_vrf_key_handle()->set_vrf_id(1);
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
