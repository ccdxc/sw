#include "nic/hal/src/qos.hpp"
#include "nic/gen/proto/hal/qos.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/test/utils/hal_base_test.hpp"

using qos::CoppSpec;
using qos::CoppResponse;
using qos::CoppKeyHandle;

class copp_test : public hal_base_test {
protected:
  copp_test() {
  }

  virtual ~copp_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// Creating user-defined copps
TEST_F(copp_test, test1)
{
    hal_ret_t    ret;
    CoppSpec     spec;
    CoppResponse rsp;

    spec.Clear();

    spec.mutable_key_or_handle()->set_copp_type(qos::COPP_TYPE_FLOW_MISS);
    spec.mutable_policer()->set_bps_rate(10000);
    spec.mutable_policer()->set_burst_size(1000);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::copp_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);


    spec.Clear();
    spec.mutable_key_or_handle()->set_copp_type(qos::COPP_TYPE_ARP);
    spec.mutable_policer()->set_bps_rate(10000);
    spec.mutable_policer()->set_burst_size(1000);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::copp_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);
}

// Negative tests
TEST_F(copp_test, test2)
{
    hal_ret_t    ret;
    CoppSpec     spec;
    CoppResponse rsp;

    spec.Clear();

    spec.mutable_key_or_handle()->set_copp_type(qos::COPP_TYPE_DHCP);
    // rate is 0
    spec.mutable_policer()->set_burst_size(1000);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::copp_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_NE(ret, HAL_RET_OK);

    spec.Clear();
    // copp type not set
    spec.mutable_policer()->set_bps_rate(10000);
    spec.mutable_policer()->set_burst_size(1000);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::copp_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_INVALID_ARG);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
