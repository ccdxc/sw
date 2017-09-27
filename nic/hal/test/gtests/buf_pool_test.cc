#include "nic/hal/src/qos.hpp"
#include "nic/proto/hal/qos.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"

using qos::BufPoolSpec;
using qos::BufPoolResponse;
using qos::BufPoolKeyHandle;

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

class buf_pool_test : public ::testing::Test {
protected:
  buf_pool_test() {
  }

  virtual ~buf_pool_test() {
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

// Creating a buf pool
TEST_F(buf_pool_test, test1)
{
    hal_ret_t       ret;
    BufPoolSpec     spec;
    BufPoolResponse rsp;

    spec.Clear();
    spec.set_port_num(TM_PORT_UPLINK_0);

    spec.mutable_key_or_handle()->set_buf_pool_id(1);

    // Some random values for now. 
    spec.set_reserved_bytes(500000);
    spec.set_headroom_bytes(20000);
    spec.set_sharing_factor(10);
    spec.set_xon_threshold(20000);
    spec.set_mtu(9216);


    spec.add_tcs()->set_cos(0);
    spec.add_tcs()->set_cos(1);
    spec.add_tcs()->set_cos(2);
    spec.add_tcs()->set_cos(3);
    
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::buf_pool_create(spec, &rsp);
    hal::hal_cfg_db_close(false);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// Creating multiple buffer pools
TEST_F(buf_pool_test, test2)
{
    hal_ret_t       ret;
    BufPoolSpec     spec;
    BufPoolResponse rsp;

    for (int i = 1; i < 4; i++) {
        spec.Clear();
        spec.set_port_num(i);

        spec.mutable_key_or_handle()->set_buf_pool_id(i);

        // Some random values for now. 
        spec.set_reserved_bytes(i*100000);
        spec.set_headroom_bytes(i*2000);
        spec.set_sharing_factor(i*5);
        spec.set_xon_threshold(i*3000);
        spec.set_mtu(i*1500);

        spec.add_tcs()->set_cos(0);
        spec.add_tcs()->set_cos(1);
        spec.add_tcs()->set_cos(2);
        spec.add_tcs()->set_cos(3);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::buf_pool_create(spec, &rsp);
        hal::hal_cfg_db_close(false);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }
}

// Negative test when multiple coses are mapped to same buf pool
TEST_F(buf_pool_test, test3)
{
    hal_ret_t       ret;
    BufPoolSpec     spec;
    BufPoolResponse rsp;

    for (int i = 0; i < 4; i++) {
        spec.Clear();
        spec.set_port_num(TM_PORT_EGRESS);

        spec.mutable_key_or_handle()->set_buf_pool_id(i);

        // Some random values for now. 
        spec.set_reserved_bytes(i*100000);
        spec.set_headroom_bytes(i*2000);
        spec.set_sharing_factor(i*5);
        spec.set_xon_threshold(i*3000);
        spec.set_mtu(i*1500);


        spec.add_tcs()->set_cos(i);
        spec.add_tcs()->set_cos(i + 1);
        spec.add_tcs()->set_cos(i + 2);
        spec.add_tcs()->set_cos(i + 3);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::buf_pool_create(spec, &rsp);
        hal::hal_cfg_db_close(false);
        if (i == 0) {
            ASSERT_TRUE(ret == HAL_RET_OK);
        } else {
            EXPECT_TRUE(rsp.api_status() == types::API_STATUS_BUF_POOL_COS_MAP_EXISTS);
            EXPECT_TRUE(ret != HAL_RET_OK);
        }
    }
}

// Negative tests when programming more buf-pools than supported by hardware
TEST_F(buf_pool_test, test4)
{
    hal_ret_t       ret;
    BufPoolSpec     spec;
    BufPoolResponse rsp;

    for (int i = 0; i < 9; i++) {
        spec.Clear();
        spec.set_port_num(TM_PORT_NCSI);

        spec.mutable_key_or_handle()->set_buf_pool_id(i);

        // Some random values for now. 
        spec.set_reserved_bytes(i*100000);
        spec.set_headroom_bytes(i*2000);
        spec.set_sharing_factor(i*5);
        spec.set_xon_threshold(i*3000);
        spec.set_mtu(i*1500);

        spec.add_tcs()->set_cos(i);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::buf_pool_create(spec, &rsp);
        hal::hal_cfg_db_close(false);
        if (i == 8) {
            EXPECT_TRUE(ret != HAL_RET_OK);
        } else {
            ASSERT_TRUE(ret == HAL_RET_OK);
        }
    }

    for (int i = 0; i < 17; i++) {
        spec.Clear();
        spec.set_port_num(TM_PORT_DMA);

        spec.mutable_key_or_handle()->set_buf_pool_id(i);

        // Some random values for now. 
        spec.set_reserved_bytes(i*100000);
        spec.set_headroom_bytes(i*2000);
        spec.set_sharing_factor(i*5);
        spec.set_xon_threshold(i*3000);
        spec.set_mtu(i*1500);

        spec.add_tcs()->set_cos(i);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::buf_pool_create(spec, &rsp);
        hal::hal_cfg_db_close(false);
        if (i == 16) {
            EXPECT_TRUE(ret != HAL_RET_OK);
        } else {
            ASSERT_TRUE(ret == HAL_RET_OK);
        }
    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
