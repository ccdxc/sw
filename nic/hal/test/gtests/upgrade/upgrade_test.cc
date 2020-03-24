#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "gen/proto/interface.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/iris/upgrade/upgrade.hpp"
#include "nic/hal/iris/upgrade/nicmgr_upgrade.hpp"

using intf::LifSpec;
using intf::LifResponse;
using intf::LifGetRequest;
using intf::LifGetResponseMsg;
using kh::LifKeyHandle;
using hal::lif_hal_info_t;
using hal::upgrade::upgrade_handler;
using hal::upgrade::nicmgr_upgrade_handler;
using upgrade::UpgHandlerPtr;

class upgrade_test : public hal_base_test {
protected:
  upgrade_test() {
  }

  virtual ~upgrade_test() {
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
// HAL Upgrade Handler test
// ----------------------------------------------------------------------------

// nicmgr Upgrade Handler test
TEST_F(upgrade_test, test1)
{
    hal_ret_t           ret;
    LifSpec 		    spec;
    LifResponse 	    rsp;
    LifGetResponseMsg   get_rsp_msg;
    LifGetRequest       get_req;

    for (int i = 0; i < 10; i++) {
        spec.mutable_key_or_handle()->set_lif_id(200 + i);
        spec.set_hw_lif_id(200 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::lif_create(spec, &rsp, NULL);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // LinkDown handler
    UpgCtx upgCtx;
    UpgHandlerPtr nicmgr_upg_handler = std::make_shared<nicmgr_upgrade_handler>();

    HdlrResp resp = nicmgr_upg_handler->CompatCheckHandler(upgCtx);
    ASSERT_TRUE(resp.resp == ::upgrade::SUCCESS);

    resp = nicmgr_upg_handler->LinkDownHandler(upgCtx);
    ASSERT_TRUE(resp.resp != ::upgrade::FAIL);
    if (resp.resp == ::upgrade::INPROGRESS) {
        sleep(2); // wait for any pending status
    }
    resp = nicmgr_upg_handler->SaveStateHandler(upgCtx);
    ASSERT_TRUE(resp.resp == ::upgrade::SUCCESS);

}

// hal Upgrade Handler test
TEST_F(upgrade_test, test2)
{
    hal_ret_t           ret;
    LifSpec 		    spec;
    LifResponse 	    rsp;
    LifGetResponseMsg   get_rsp_msg;
    LifGetRequest       get_req;

    for (int i = 0; i < 10; i++) {
        spec.mutable_key_or_handle()->set_lif_id(100 + i);
        spec.set_hw_lif_id(100 + i);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::lif_create(spec, &rsp, NULL);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

    // LinkDown handler
    UpgCtx upgCtx;
    UpgHandlerPtr upg_handler = std::make_shared<upgrade_handler>();

    HdlrResp resp = upg_handler->CompatCheckHandler(upgCtx);
    ASSERT_TRUE(resp.resp == ::upgrade::SUCCESS);

    resp = upg_handler->LinkDownHandler(upgCtx);
    ASSERT_TRUE(resp.resp != ::upgrade::FAIL);
    if (resp.resp == ::upgrade::INPROGRESS) {
        sleep(2); // wait for any pending status
    }
    resp = upg_handler->SaveStateHandler(upgCtx);
    ASSERT_TRUE(resp.resp == ::upgrade::SUCCESS);

    // psp is timing out
#if 0
    // PostHostDown handler
    HdlrResp resp1 = upg_handler->PostHostDownHandler(upgCtx);
    ASSERT_TRUE(resp1.resp == ::upgrade::SUCCESS);
#endif
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
