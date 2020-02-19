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
    hal_base_test::SetUpTestCase("hal_gft.json");
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
    InterfaceDeleteResponse     del_rsp;
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
// - Create an uplink
// - Delete an uplink
// ----------------------------------------------------------------------------
TEST_F(uplinkif_test, test3)
{
    hal_ret_t                        ret;
    InterfaceSpec                   spec;
    InterfaceResponse               rsp;
    InterfaceDeleteRequest          del_req;
    InterfaceDeleteResponse         del_rsp;
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

    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
