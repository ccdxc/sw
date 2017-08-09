#include <qos.hpp>
#include <qos.pb.h>
#include <hal.hpp>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include <defines.h>

using qos::QueueSpec;
using qos::QueueResponse;
using qos::QueueKeyHandle;
using qos::QueueInfo;
using qos::QueueSchedulerNode;

void
hal_initialize()
{
    char 			cfg_file[] = "hal.json";
	char 			*cfg_path;
    std::string     full_path;
    hal::hal_cfg_t  hal_cfg = { 0 };

    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + std::string(cfg_file);
        std::cerr << "full path " << full_path << std::endl;
    } else {
        full_path = std::string(cfg_file);
    }

    // make sure cfg file exists
    if (access(full_path.c_str(), R_OK) < 0) {
        fprintf(stderr, "config file %s has no read permissions\n",
                full_path.c_str());
        exit(1);
    }

    printf("Json file: %s\n", full_path.c_str());

    if (hal::hal_parse_cfg(full_path.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        ASSERT_TRUE(0);
    }
    printf("Parsed cfg json file \n");

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL Initialized \n");
}

class queue_test : public ::testing::Test {
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

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_initialize();
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
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
    for (int i = 0; i < cnt_l0_nodes; i++) {
        QueueInfo *qinfo = spec.add_queues();
        qinfo->mutable_key_or_handle()->set_queue_id(i);
        qinfo->mutable_queue_info()->set_priority(i);
        qinfo->mutable_queue_info()->mutable_dwrr()->set_weight(1+i*100);
    }

    for (int i = 0; i < cnt_l1_nodes; i++) {
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
    for (int i = 0; i < cnt_l0_nodes; i++) {
        QueueInfo *qinfo = spec.add_queues();
        qinfo->mutable_key_or_handle()->set_queue_id(i);
        qinfo->mutable_queue_info()->set_priority(i);
        qinfo->mutable_queue_info()->mutable_dwrr()->set_weight(1+i*100);
    }

    for (int i = 0; i < cnt_l1_nodes; i++) {
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
    for (int i = 0; i < cnt_l0_nodes; i++) {
        QueueInfo *qinfo = spec.add_queues();
        qinfo->mutable_key_or_handle()->set_queue_id(i);
        qinfo->mutable_queue_info()->set_priority(i);
        qinfo->mutable_queue_info()->mutable_dwrr()->set_weight(1+i*100);
    }

    for (int i = 0; i < cnt_l1_nodes; i++) {
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
