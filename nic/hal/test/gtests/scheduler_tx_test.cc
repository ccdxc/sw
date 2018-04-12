#include "nic/hal/src/nw/interface.hpp"
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
using kh::LifKeyHandle;
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
    qos_init();
  }

  static void qos_init(void); 
  static const kh::QosGroup cos_a_1 = kh::USER_DEFINED_2;
  static const kh::QosGroup cos_a_2 = kh::USER_DEFINED_1;
};

void scheduler_tx_test::qos_init()
{
    // add 2 user-qos-classes and an admin-qos-class
    hal_ret_t        ret;
    QosClassSpec     spec;
    QosClassResponse rsp;

    spec.Clear();

    spec.mutable_key_or_handle()->set_qos_group(cos_a_1);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(50);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(2);
    spec.mutable_uplink_class_map()->add_ip_dscp(10);
    spec.mutable_uplink_class_map()->add_ip_dscp(15);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);


    spec.Clear();
    spec.mutable_key_or_handle()->set_qos_group(cos_a_2);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_strict()->set_bps(10000);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(3);
    spec.mutable_uplink_class_map()->add_ip_dscp(3);
    spec.mutable_uplink_class_map()->add_ip_dscp(5);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.mutable_marking()->set_dot1q_pcp(3);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qosclass_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);
}


// ----------------------------------------------------------------------------
// Create a lif with multiple queues and cos values and check programming goes through
// ----------------------------------------------------------------------------
TEST_F(scheduler_tx_test, test1) 
{
    hal_ret_t            ret;
    LifSpec lif_spec;
    LifResponse lif_rsp;

    // Create a lif with two queue types and specify qos-class and tx-policer
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    lif_spec.mutable_tx_qos_class()->set_qos_group(cos_a_1);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(15);
    lif_spec.mutable_tx_policer()->mutable_bps_policer()->set_bytes_per_sec(10000);
    lif_spec.mutable_tx_policer()->mutable_bps_policer()->set_burst_bytes(200);

    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(1)->set_type_num(1);
    lif_spec.mutable_lif_qstate_map(1)->set_size(1);
    lif_spec.mutable_lif_qstate_map(1)->set_entries(17);

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
    lif_spec.mutable_key_or_handle()->set_lif_id(2);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_tx_qos_class()->set_qos_group(cos_a_1);
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(15);

    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(1)->set_type_num(1);
    lif_spec.mutable_lif_qstate_map(1)->set_size(1);
    lif_spec.mutable_lif_qstate_map(1)->set_entries(13);


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    //Second LIF
    lif_spec.mutable_key_or_handle()->set_lif_id(3);
    lif_spec.mutable_tx_qos_class()->set_qos_group(cos_a_2);
    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(15);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Create a lif with no queue-map-info and update with multiple queues and cos values
// and check programming goes through
// ----------------------------------------------------------------------------
TEST_F(scheduler_tx_test, test3)
{
    hal_ret_t            ret;
    LifSpec lif_spec;
    LifResponse lif_rsp;

    // Create a lif with two queue types and cos-value.
    lif_spec.mutable_key_or_handle()->set_lif_id(4);
    lif_spec.mutable_tx_qos_class()->set_qos_group(cos_a_1);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(0)->set_type_num(0);
    lif_spec.mutable_lif_qstate_map(0)->set_size(1);
    lif_spec.mutable_lif_qstate_map(0)->set_entries(5);

    lif_spec.add_lif_qstate_map();
    lif_spec.mutable_lif_qstate_map(1)->set_type_num(1);
    lif_spec.mutable_lif_qstate_map(1)->set_size(1);
    lif_spec.mutable_lif_qstate_map(1)->set_entries(4);

    lif_spec.mutable_tx_policer()->mutable_bps_policer()->set_bytes_per_sec(20000);
    lif_spec.mutable_tx_policer()->mutable_bps_policer()->set_burst_bytes(400);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::lif_update(lif_spec, &lif_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// ----------------------------------------------------------------------------
// Creating & deleting of a lif with cos values specified
// ----------------------------------------------------------------------------
TEST_F(scheduler_tx_test, test4)
{
    hal_ret_t                       ret;
    LifSpec                         spec;
    LifResponse                     rsp;
    LifDeleteRequest                del_req;
    LifDeleteResponse            del_rsp;
    slab_stats_t                    *pre = NULL, *post = NULL;
    bool                            is_leak = false;

    pre = hal_test_utils_collect_slab_stats();

    // Create lif
    spec.set_vlan_strip_en(1);
    spec.mutable_key_or_handle()->set_lif_id(400);
    spec.mutable_tx_qos_class()->set_qos_group(cos_a_1);

    spec.add_lif_qstate_map();
    spec.mutable_lif_qstate_map(0)->set_type_num(1);
    spec.mutable_lif_qstate_map(0)->set_size(1);
    spec.mutable_lif_qstate_map(0)->set_entries(13);
    spec.mutable_tx_policer()->mutable_bps_policer()->set_bytes_per_sec(10000);
    spec.mutable_tx_policer()->mutable_bps_policer()->set_burst_bytes(200);

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
