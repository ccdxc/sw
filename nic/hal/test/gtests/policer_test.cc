#include "nic/hal/src/qos.hpp"
#include "nic/gen/proto/hal/qos.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/src/qos.hpp"

using qos::PolicerSpec;
using qos::PolicerResponse;
using qos::PolicerKeyHandle;

class policer_test : public hal_base_test {
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
