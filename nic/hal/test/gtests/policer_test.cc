#include "nic/hal/src/qos.hpp"
#include "nic/proto/hal/qos.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"

using qos::PolicerSpec;
using qos::PolicerResponse;
using qos::PolicerKeyHandle;

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

class policer_test : public ::testing::Test {
protected:
  policer_test() {
  }

  virtual ~policer_test() {
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

// Creating policers
TEST_F(policer_test, test1)
{
    hal_ret_t       ret;
    PolicerSpec     spec;
    PolicerResponse rsp;

    spec.Clear();
    spec.mutable_key_or_handle()->set_policer_id(1);

    spec.set_direction(qos::INGRESS_POLICER);
    spec.set_bandwidth(100000);
    spec.set_burst_size(1000);

    ret = hal::policer_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
}

// Create policers with marking action in a batch
TEST_F(policer_test, test2)
{
    hal_ret_t       ret;
    PolicerSpec     spec;
    PolicerResponse rsp;

    for (int i = 0; i < 10; i++) {
        spec.Clear();
        spec.mutable_key_or_handle()->set_policer_id(1);

        if (i%2) {
            spec.set_direction(qos::INGRESS_POLICER);
        } else {
            spec.set_direction(qos::EGRESS_POLICER);
        }
        spec.set_bandwidth(1 + i*100000);
        spec.set_burst_size(1 + i*1000);

        if (!(i%3)) {
            spec.mutable_marking_spec()->set_pcp_rewrite_en(true);
            spec.mutable_marking_spec()->set_pcp(i%8);
            spec.mutable_marking_spec()->set_dscp_rewrite_en(true);
            spec.mutable_marking_spec()->set_dscp(i);
        }

        ret = hal::policer_create(spec, &rsp);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
