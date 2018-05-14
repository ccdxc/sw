#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/pd/pd_api.hpp"

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
using vrf::VrfSpec;
using vrf::VrfResponse;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponse;
using vrf::VrfDeleteResponse;
using nwsec::SecurityProfileSpec;
using nwsec::SecurityProfileResponse;
using nw::NetworkSpec;
using nw::NetworkResponse;

class vrf_upgrade_test : public hal_base_test {
protected:
  vrf_upgrade_test() {
  }

  virtual ~vrf_upgrade_test() {
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
// Vrf delete test
// ----------------------------------------------------------------------------
TEST_F(vrf_upgrade_test, test1)
{
    hal_ret_t               ret;
    VrfSpec                 ten_spec;
    VrfResponse             ten_rsp;
    SecurityProfileSpec     sp_spec;
    SecurityProfileResponse sp_rsp;
    VrfDeleteRequest        del_req;
    VrfDeleteResponse       del_rsp;
    slab_stats_t            *pre      = NULL   , *post = NULL;
    bool                    is_leak   = false;
    hal::hal_obj_id_t       obj_id;
    void                    *obj;
    hal::vrf_t              *vrf = NULL;
    VrfGetResponseMsg       get_rsp_msg;
    VrfGetRequest           get_req;
    hal::pd::pd_get_object_from_flow_lkupid_args_t args;

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(1);
    sp_spec.set_ipsg_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nwsec_hdl = sp_rsp.mutable_profile_status()->profile_handle();

    pre = hal_test_utils_collect_slab_stats();

    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    ten_spec.mutable_security_key_handle()->set_profile_handle(nwsec_hdl);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);

    args.flow_lkupid = 4096;
    args.obj_id = &obj_id;
    args.pi_obj = &obj;
    // ret = hal::pd::pd_get_object_from_flow_lkupid(4096, &obj_id, &obj);
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_OBJ_FROM_FLOW_LKPID, (void *)&args);
    ASSERT_TRUE(ret == HAL_RET_OK);
    vrf = (hal::vrf_t *)obj;
    ASSERT_TRUE(vrf->vrf_id == 1);

    // Get vrf
    get_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_get(get_req, &get_rsp_msg);
    hal::hal_cfg_db_close();
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_l2_segments() == 0);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_security_groups() == 0);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_l4lb_services() == 0);
    ASSERT_TRUE(get_rsp_msg.response(0).stats().num_endpoints() == 0);


    // Create nwsec
    // Create vrf
    // Get nwsec and vrf, store them as state1.
    // Delete nwsec and vrf
    // Restore nwsec and vrf.
    // Get nwsec and vrf, store them as state2.
    // state1 and state2 have to match.
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
