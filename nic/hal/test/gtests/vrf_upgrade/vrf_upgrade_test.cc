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
#include "nic/hal/pd/pd_api.hpp"
#include <google/protobuf/util/json_util.h>

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
    hal_ret_t                           ret;
    VrfSpec                             ten_spec;
    VrfResponse                         ten_rsp;
    SecurityProfileSpec                 sp_spec;
    SecurityProfileResponse             sp_rsp;
    SecurityProfileGetRequest           sec_pre_get_req, sec_del_get_req, sec_post_get_req;
    SecurityProfileGetResponseMsg       sec_pre_get_rsp_msg, sec_del_get_rsp_msg, sec_post_get_rsp_msg;
    SecurityProfileDeleteRequest        sec_del_req;
    SecurityProfileDeleteResponse       sec_del_rsp;
    VrfDeleteRequest                    vrf_del_req;
    VrfDeleteResponse                   vrf_del_rsp;
    VrfGetRequest                       vrf_pre_get_req, vrf_del_get_req, vrf_post_get_req;
    VrfGetResponseMsg                   vrf_pre_get_rsp_msg, vrf_del_get_rsp_msg, vrf_post_get_rsp_msg;
    uint32_t                            size1 = 0, size2 = 0;
    uint8_t                                 *mem1 = NULL, *mem2 = NULL;

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
    HAL_TRACE_DEBUG("ret: {}", ret);
    ASSERT_TRUE(ret == HAL_RET_OK);


    // Get Vrf and Nwsec
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_get(sec_pre_get_req, &sec_pre_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_get(vrf_pre_get_req, &vrf_pre_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // Preserve hal state
    hal::test::hal_test_preserve_state();

    // Delete vrf and nwsec
    vrf_del_req.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_delete(vrf_del_req, &vrf_del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    sec_del_req.mutable_key_or_handle()->set_profile_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sec_del_req, &sec_del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // Delete default profile
    sec_del_req.mutable_key_or_handle()->set_profile_id(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sec_del_req, &sec_del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    sec_del_req.mutable_key_or_handle()->set_profile_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sec_del_req, &sec_del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    sec_del_req.mutable_key_or_handle()->set_profile_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(sec_del_req, &sec_del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // Get nwsec and vrf. Should be empty
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_get(sec_del_get_req, &sec_del_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_get(vrf_del_get_req, &vrf_del_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(sec_del_get_rsp_msg.response_size(), 0);
    EXPECT_EQ(vrf_del_get_rsp_msg.response_size(), 0);

    // Restore hal state
    hal::test::hal_test_restore_state();

    // Get nwsec and vrf.
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_get(sec_post_get_req, &sec_post_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_get(vrf_post_get_req, &vrf_post_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // compare responses
    EXPECT_EQ(vrf_pre_get_rsp_msg.ByteSizeLong(), vrf_post_get_rsp_msg.ByteSizeLong());
    EXPECT_EQ(sec_pre_get_rsp_msg.ByteSizeLong(), sec_post_get_rsp_msg.ByteSizeLong());
    size1 = vrf_pre_get_rsp_msg.ByteSizeLong();
    size2 = sec_pre_get_rsp_msg.ByteSizeLong();
    mem1 = (uint8_t *)malloc(size1);
    mem2 = (uint8_t *)malloc(size1);
    vrf_pre_get_rsp_msg.SerializeToArray(mem1, size1);
    vrf_post_get_rsp_msg.SerializeToArray(mem2, size1);
    EXPECT_EQ(memcmp(mem1, mem2, size1), 0);
    free(mem1);
    free(mem2);
    mem1 = (uint8_t *)malloc(size2);
    mem2 = (uint8_t *)malloc(size2);
    sec_pre_get_rsp_msg.SerializeToArray(mem1, size2);
    sec_post_get_rsp_msg.SerializeToArray(mem2, size2);
    EXPECT_EQ(memcmp(mem1, mem2, size2), 0);
    free(mem1);
    free(mem2);


    std::string pre, post;
    google::protobuf::util::MessageToJsonString(sec_pre_get_rsp_msg, &pre);
    google::protobuf::util::MessageToJsonString(sec_post_get_rsp_msg, &post);
    HAL_TRACE_DEBUG("Sec Pre: {}", pre);
    HAL_TRACE_DEBUG("Sec Post: {}", post);
    google::protobuf::util::MessageToJsonString(vrf_pre_get_rsp_msg, &pre);
    google::protobuf::util::MessageToJsonString(vrf_post_get_rsp_msg, &post);
    HAL_TRACE_DEBUG("Vrf Pre: {}", pre);
    HAL_TRACE_DEBUG("Vrf Post: {}", post);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
