#include "nic/hal/src/interface.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/gen/proto/hal/qos.pb.h" 

using intf::LifSpec;
using intf::LifResponse;
using intf::LifKeyHandle;
using hal::lif_hal_info_t;

class scheduler_tx_test : public hal_base_test {
protected:
  scheduler_tx_test() {
  }

  virtual ~scheduler_tx_test() {
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
// Create a lif with multiple queues and cos values and check programming goes through
// ----------------------------------------------------------------------------
TEST_F(scheduler_tx_test, test1) 
{
    hal_ret_t            ret;
    LifSpec lif_spec;
    LifResponse lif_rsp;

    // Create a lif with two queue types and 3 coses
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(1);

    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(15);

    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(1)->set_type_num(1);
    lif_spec.mutable_lif_qstate_map(1)->set_size(1);
    lif_spec.mutable_lif_qstate_map(1)->set_entries(17);

    lif_spec.add_tcs()->set_cos(3);
    lif_spec.add_tcs()->set_cos(4);
    lif_spec.add_tcs()->set_cos(5);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}


// ----------------------------------------------------------------------------------
// Create multiple lif with multiple queues and cos values and check programming goes through
// --------------------------------------------------------------------------------------
TEST_F(scheduler_tx_test, test2)
{
    hal_ret_t            ret;
    LifSpec              lif_spec;
    LifResponse          lif_rsp;
   
    //First LIF 
    lif_spec.set_port_num(10);
    lif_spec.mutable_key_or_handle()->set_lif_id(2);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(15);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(1)->set_type_num(1);
    lif_spec.mutable_lif_qstate_map(1)->set_size(1);
    lif_spec.mutable_lif_qstate_map(1)->set_entries(13);
    lif_spec.add_tcs()->set_cos(4);
    lif_spec.add_tcs()->set_cos(5);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    //Second LIF
    lif_spec.set_port_num(20);
    lif_spec.mutable_key_or_handle()->set_lif_id(3);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(15);
    lif_spec.add_tcs()->set_cos(6);
    lif_spec.add_tcs()->set_cos(7);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Creating & deleting of a lif with cos values specified
// ----------------------------------------------------------------------------
TEST_F(scheduler_tx_test, test3)
{
    hal_ret_t                       ret;
    LifSpec                         spec;
    LifResponse                     rsp;
    LifDeleteRequest                del_req;
    LifDeleteResponseMsg            del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // Create lif
    spec.set_port_num(10);
    spec.set_vlan_strip_en(1);
    //spec.set_allmulti(1);
    spec.mutable_key_or_handle()->set_lif_id(400);
    spec.add_tcs()->set_cos(10);
    spec.add_tcs()->set_cos(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(spec, &rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Delete lif
    del_req.mutable_key_or_handle()->set_lif_id(400);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    post = hal_test_utils_collect_slab_stats();

    hal_test_utils_check_slab_leak(pre, post, &is_leak);
    ASSERT_TRUE(is_leak == false);
}

int main(int argc, char **argv) {                 
  ::testing::InitGoogleTest(&argc, argv);         
  return RUN_ALL_TESTS();                         
}
