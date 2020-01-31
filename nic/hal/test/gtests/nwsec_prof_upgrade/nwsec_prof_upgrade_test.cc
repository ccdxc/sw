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
#include <boost/interprocess/managed_shared_memory.hpp>

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
using nwsec::SecurityProfileGetRequest;
using nwsec::SecurityProfileGetResponseMsg;
using nw::NetworkSpec;
using nw::NetworkResponse;

using namespace hal::test;
 using namespace boost::interprocess;

class nwsec_prof_upgrade_test : public hal_base_test {
protected:
  nwsec_prof_upgrade_test() {
  }

  virtual ~nwsec_prof_upgrade_test() {
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
// NWsec profile upgrade test
// ----------------------------------------------------------------------------
TEST_F(nwsec_prof_upgrade_test, test1)
{
    hal_ret_t                               ret;
    SecurityProfileSpec                     sp_spec, sp_spec1;
    SecurityProfileResponse                 sp_rsp, sp_rsp1;
    SecurityProfileDeleteRequest            del_req;
    SecurityProfileDeleteResponse           del_rsp;
    SecurityProfileGetRequest               pre_get_req, del_get_req, post_get_req;
    SecurityProfileGetResponseMsg           pre_get_rsp_msg, pre_get_rsp_msg1, del_get_rsp_msg, post_get_rsp_msg;
    uint8_t                                 *mem1 = NULL, *mem2 = NULL;
    uint32_t                                size = 0;

    // shared_memory_object::remove("h3s");

    // Create nwsec
    sp_spec.mutable_key_or_handle()->set_profile_id(10);
    sp_spec.set_ipsg_en(true);
    sp_spec.set_ip_normalization_en(true);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_create(sp_spec, &sp_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // Get nwsec and store
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_get(pre_get_req, &pre_get_rsp_msg);
    hal::hal_cfg_db_close();

    // Preserve hal state
    hal::test::hal_test_preserve_state();

    // Delete default nwsec 
    del_req.mutable_key_or_handle()->set_profile_id(0);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    del_req.mutable_key_or_handle()->set_profile_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    del_req.mutable_key_or_handle()->set_profile_id(2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // Delete nwsec
    del_req.mutable_key_or_handle()->set_profile_id(10);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // Get nwsec. Should be empty
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_get(pre_get_req, &pre_get_rsp_msg1);
    hal::hal_cfg_db_close();
    EXPECT_EQ(pre_get_rsp_msg1.response_size(), 0);

    // Restore hal state
    hal::test::hal_test_restore_state();

    // Query nwsec and compare with nwsecs before preserve
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::securityprofile_get(post_get_req, &post_get_rsp_msg);
    hal::hal_cfg_db_close();
    EXPECT_EQ(ret, HAL_RET_OK);

    // compare responses
    EXPECT_EQ(pre_get_rsp_msg.ByteSizeLong(), post_get_rsp_msg.ByteSizeLong());
    size = pre_get_rsp_msg.ByteSizeLong();
    mem1 = (uint8_t *)malloc(size);
    mem2 = (uint8_t *)malloc(size);
    pre_get_rsp_msg.SerializeToArray(mem1, size);
    post_get_rsp_msg.SerializeToArray(mem2, size);
    EXPECT_EQ(memcmp(mem1, mem2, size), 0);

    std::string pre, post;
    google::protobuf::util::MessageToJsonString(pre_get_rsp_msg, &pre);
    google::protobuf::util::MessageToJsonString(post_get_rsp_msg, &post);
    HAL_TRACE_DEBUG("Pre: {}", pre);
    HAL_TRACE_DEBUG("Post: {}", post);

    free(mem1);
    free(mem2);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
