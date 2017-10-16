#include "nic/hal/src/qos.hpp"
#include "nic/proto/hal/qos.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/test/utils/hal_base_test.hpp"

using qos::QueueSpec;
using qos::QueueResponse;
using qos::QueueKeyHandle;
using qos::QueueInfo;
using qos::QueueSchedulerNode;

class queue_test : public hal_base_test {
protected:
  queue_test() {
  }

  virtual ~queue_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// Creating queues
TEST_F(queue_test, test1)
{
    hal_ret_t       ret;
    QueueSpec     spec;
    QueueResponse rsp;
    uint32_t cnt_l0_nodes, cnt_l1_nodes;

    spec.Clear();
    spec.set_port_num(TM_PORT_UPLINK_0);

    spec.set_scheduler_policy(qos::TM_QUEUE_SCHEDULER_2_4);

    cnt_l0_nodes = 32;
    cnt_l1_nodes = 16;
    for (uint i = 0; i < cnt_l0_nodes; i++) {
        QueueInfo *qinfo = spec.add_queues();
        qinfo->mutable_key_or_handle()->set_queue_id(i);
        qinfo->mutable_queue_info()->set_priority(i);
        qinfo->mutable_queue_info()->mutable_dwrr()->set_weight(1+i*100);
    }

    for (uint i = 0; i < cnt_l1_nodes; i++) {
        QueueSchedulerNode *queue_info = spec.add_l1_nodes();

        queue_info->set_priority(i);
        queue_info->mutable_strict()->set_rate(1+i*10000);
    }

    ret = hal::queue_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// Negative test with fewer L1 nodes 
TEST_F(queue_test, test2)
{
    hal_ret_t       ret;
    QueueSpec     spec;
    QueueResponse rsp;
    uint32_t cnt_l0_nodes, cnt_l1_nodes;

    spec.Clear();
    spec.set_port_num(TM_PORT_DMA);

    spec.set_scheduler_policy(qos::TM_QUEUE_SCHEDULER_4_2);

    cnt_l0_nodes = 26;
    cnt_l1_nodes = 6;
    for (uint i = 0; i < cnt_l0_nodes; i++) {
        QueueInfo *qinfo = spec.add_queues();
        qinfo->mutable_key_or_handle()->set_queue_id(i);
        qinfo->mutable_queue_info()->set_priority(i);
        qinfo->mutable_queue_info()->mutable_dwrr()->set_weight(1+i*100);
    }

    for (uint i = 0; i < cnt_l1_nodes; i++) {
        QueueSchedulerNode *queue_info = spec.add_l1_nodes();

        queue_info->set_priority(i);
        queue_info->mutable_strict()->set_rate(1+i*10000);
    }

    ret = hal::queue_create(spec, &rsp);
    EXPECT_TRUE(rsp.api_status() == types::API_STATUS_QUEUE_COUNT_INVALID);
    ASSERT_TRUE(ret != HAL_RET_OK);
}

// Negative test with more than hw supported nodes 
TEST_F(queue_test, test3)
{
    hal_ret_t       ret;
    QueueSpec     spec;
    QueueResponse rsp;
    uint32_t cnt_l0_nodes, cnt_l1_nodes;

    spec.Clear();
    spec.set_port_num(TM_PORT_DMA);

    spec.set_scheduler_policy(qos::TM_QUEUE_SCHEDULER_2_4);

    cnt_l0_nodes = 36;
    cnt_l1_nodes = 18;
    for (uint i = 0; i < cnt_l0_nodes; i++) {
        QueueInfo *qinfo = spec.add_queues();
        qinfo->mutable_key_or_handle()->set_queue_id(i);
        qinfo->mutable_queue_info()->set_priority(i);
        qinfo->mutable_queue_info()->mutable_dwrr()->set_weight(1+i*100);
    }

    for (uint i = 0; i < cnt_l1_nodes; i++) {
        QueueSchedulerNode *queue_info = spec.add_l1_nodes();

        queue_info->set_priority(i);
        queue_info->mutable_strict()->set_rate(1+i*10000);
    }

    ret = hal::queue_create(spec, &rsp);
    ASSERT_TRUE(ret != HAL_RET_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
