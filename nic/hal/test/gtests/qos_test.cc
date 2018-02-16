#include "nic/hal/src/qos.hpp"
#include "nic/gen/proto/hal/qos.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/test/utils/hal_base_test.hpp"

using qos::QosClassSpec;
using qos::QosClassResponse;
using kh::QosClassKeyHandle;
using qos::QosClassDeleteRequest;
using qos::QosClassDeleteResponse;

class qos_class_test : public hal_base_test {
protected:
  qos_class_test() {
  }

  virtual ~qos_class_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// Creating user-defined qos_classs
TEST_F(qos_class_test, test1)
{
    hal_ret_t        ret;
    QosClassSpec     spec;
    QosClassResponse rsp;

    spec.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_strict()->set_bps(10000);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(3);
    spec.mutable_uplink_class_map()->add_ip_dscp(3);
    spec.mutable_uplink_class_map()->add_ip_dscp(5);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.mutable_marking()->set_dot1q_pcp(3);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // Negative test with using same dscp value as above
    spec.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_strict()->set_bps(10000);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(5);
    spec.mutable_uplink_class_map()->add_ip_dscp(3);
    spec.mutable_uplink_class_map()->add_ip_dscp(2);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);

    // Update
    spec.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_strict()->set_bps(10000);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(4);
    spec.mutable_uplink_class_map()->add_ip_dscp(4);
    spec.mutable_uplink_class_map()->add_ip_dscp(5);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.mutable_marking()->set_dot1q_pcp(3);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // Now this should go through because contention on ip_dscp(3) is gone with
    // the update
    spec.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_strict()->set_bps(10000);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(5);
    spec.mutable_uplink_class_map()->add_ip_dscp(3);
    spec.mutable_uplink_class_map()->add_ip_dscp(2);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    // Update fail because reuse same dot1_pcp as user-def-2
    spec.Clear();
    spec.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_strict()->set_bps(10000);
    spec.mutable_uplink_class_map()->set_dot1q_pcp(5);
    spec.mutable_uplink_class_map()->add_ip_dscp(4);
    spec.mutable_uplink_class_map()->add_ip_dscp(5);
    spec.mutable_marking()->set_dot1q_pcp_rewrite_en(true);
    spec.mutable_marking()->set_dot1q_pcp(3);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_update(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);

    // Delete
    QosClassDeleteRequest del_req;
    QosClassDeleteResponse del_rsp;

    del_req.Clear();
    del_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

    del_req.Clear();
    del_req.mutable_key_or_handle()->set_qos_group(kh::USER_DEFINED_2);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_delete(del_req, &del_rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);

}

// Creating internal qos_classs
TEST_F(qos_class_test, test2)
{
    hal_ret_t        ret;
    QosClassSpec     spec;
    QosClassResponse rsp;

    spec.Clear();

    spec.mutable_key_or_handle()->set_qos_group(kh::INTERNAL_RX_PROXY_NO_DROP);
    spec.set_mtu(2000);
    spec.mutable_sched()->mutable_dwrr()->set_bw_percentage(100);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::qos_class_create(spec, &rsp);
    hal::hal_cfg_db_close();
    // Internal classes are created during init. So this should fail
    ASSERT_NE(ret, HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
