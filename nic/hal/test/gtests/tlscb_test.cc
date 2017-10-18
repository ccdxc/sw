#include "nic/hal/src/tlscb.hpp"
#include "nic/gen/proto/hal/tls_proxy_cb.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_base_test.hpp"

using tlscb::TlsCbSpec;
using tlscb::TlsCbResponse;
using tlscb::TlsCbKeyHandle;
using tlscb::TlsCbGetRequest;
using tlscb::TlsCbGetResponse;


class tlscb_test : public hal_base_test {
protected:
  tlscb_test() {
  }

  virtual ~tlscb_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// ----------------------------------------------------------------------------
// Creating a TLS CB
// ----------------------------------------------------------------------------
TEST_F(tlscb_test, test1) 
{
    hal_ret_t            ret;
    TlsCbSpec spec;
    TlsCbResponse rsp;
    TlsCbGetRequest getReq;
    TlsCbGetResponse getRsp;


    spec.mutable_key_or_handle()->set_tlscb_id(0);

    ret = hal::tlscb_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    
    spec.set_command(1);
    ret = hal::tlscb_update(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);

    getReq.mutable_key_or_handle()->set_tlscb_id(0);
    ret = hal::tlscb_get(getReq, &getRsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    printf("cb_id: %d\n", getRsp.spec().key_or_handle().tlscb_id());
    printf("Done with test1\n");
}

// ----------------------------------------------------------------------------
// Creating muliple TlsCbs
// ----------------------------------------------------------------------------
TEST_F(tlscb_test, test2) 
{
    hal_ret_t           ret;
    TlsCbSpec           spec;
    TlsCbResponse       rsp;

    for (int i = 0; i < 10; i++) {
        spec.mutable_key_or_handle()->set_tlscb_id(i);

        ret = hal::tlscb_create(spec, &rsp);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
