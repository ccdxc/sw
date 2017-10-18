#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/tenant.pb.h"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

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
using nw::NetworkSpec;
using nw::NetworkResponse;


class nwsec_test : public hal_base_test {
protected:
  nwsec_test() {
  }

  virtual ~nwsec_test() {
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
// Create nwsec profile
// Update
// Delete
// ----------------------------------------------------------------------------
TEST_F(nwsec_test, test1) 
{
    hal_ret_t                               ret;
    SecurityProfileSpec                     sp_spec;
    SecurityProfileResponse                 sp_rsp;
    SecurityProfileDeleteRequest            del_req;
    SecurityProfileDeleteResponseMsg        del_rsp;    
    slab_stats_t                            *pre = NULL, *post = NULL;
    bool                                    is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    // uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    // Update nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(false);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_update(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Delete nwsec
    del_req.mutable_key_or_handle()->set_profile_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::security_profile_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // There is a leak of HAL_SLAB_HANDLE_ID_LIST_ENTRY for adding 
    post = hal_test_utils_collect_slab_stats();
    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
