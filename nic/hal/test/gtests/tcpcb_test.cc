#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/gen/proto/hal/tcp_proxy_cb.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_base_test.hpp"

using tcpcb::TcpCbSpec;
using tcpcb::TcpCbResponse;
using tcpcb::TcpCbKeyHandle;
using tcpcb::TcpCbGetRequest;
using tcpcb::TcpCbGetResponse;


class tcpcb_test : public hal_base_test {
protected:
  tcpcb_test() {
  }

  virtual ~tcpcb_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// ----------------------------------------------------------------------------
// Creating a TCP CB
// ----------------------------------------------------------------------------
TEST_F(tcpcb_test, test1) 
{
    hal_ret_t            ret;
    TcpCbSpec spec;
    TcpCbResponse rsp;
    TcpCbGetRequest getReq;
    TcpCbGetResponse getRsp;
    TcpCbGetResponseMsg rsp_msg;

    spec.mutable_key_or_handle()->set_tcpcb_id(0);
    spec.set_rcv_nxt(100);

    ret = hal::tcpcb_create(spec, &rsp);
    ASSERT_TRUE(ret == HAL_RET_OK);
    printf("TCP CB create done\n");

    getReq.mutable_key_or_handle()->set_tcpcb_id(0);
    ret = hal::tcpcb_get(getReq, &rsp_msg);
    ASSERT_TRUE(ret == HAL_RET_OK);
    getRsp = rsp_msg.response(0);
    printf("cb_id: %d\n", getRsp.spec().key_or_handle().tcpcb_id());
    printf("rcv_nxt: %d\n", getRsp.spec().rcv_nxt());
    ASSERT_TRUE(100 == getRsp.spec().rcv_nxt());

    printf("Done with test1\n");
}

// ----------------------------------------------------------------------------
// Creating muliple TCPCBs
// ----------------------------------------------------------------------------
/*
TEST_F(tcpcb_test, test2) 
{
    hal_ret_t           ret;
    TcpCbSpec           spec;
    TcpCbResponse       rsp;

    for (int i = 0; i < 10; i++) {
        spec.mutable_key_or_handle()->set_tcpcb_id(i);

        ret = hal::tcpcb_create(spec, &rsp);
        ASSERT_TRUE(ret == HAL_RET_OK);
    }

}
*/
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
