#include "nic/hal/src/internal/internal.hpp"
#include "nic/gen/proto/hal/internal.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_base_test.hpp"

using internal::ProgramAddressReq;
using internal::ProgramAddressResp;
using internal::ProgramAddressResponseMsg;
using internal::HbmAddressReq;
using internal::HbmAddressResp;


class internal_test : public hal_base_test {
protected:
  internal_test() {
  }

  virtual ~internal_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// ----------------------------------------------------------------------------
// Resolving a program name to address
// ----------------------------------------------------------------------------
TEST_F(internal_test, test1) 
{
    ProgramAddressReq req;
    ProgramAddressResp rsp;
    ProgramAddressResponseMsg resp;

    req.set_handle("p4plus");
    req.set_prog_name("txdma_stage0.bin");
    req.set_resolve_label(false);

    hal::getprogram_address(req, &resp);
    rsp = resp.response(0);
    ASSERT_TRUE((uint64_t)rsp.addr() != 0xFFFFFFFFFFFFFFFFULL);
}

// ----------------------------------------------------------------------------
// Resolving a program name, label to PC offset
// ----------------------------------------------------------------------------
TEST_F(internal_test, test2) 
{
    ProgramAddressReq req;
    ProgramAddressResp rsp;
    ProgramAddressResponseMsg resp;

    req.set_handle("p4plus");
    req.set_prog_name("txdma_stage0.bin");
    req.set_resolve_label(true);
    req.set_label("storage_tx_stage0");

    hal::getprogram_address(req, &resp);
    rsp = resp.response(0);
    ASSERT_TRUE((uint64_t)rsp.addr() != 0xFFFFFFFFFFFFFFFFULL);
}

// ----------------------------------------------------------------------------
// Resolving handle into hbm address 
// ----------------------------------------------------------------------------
TEST_F(internal_test, test3) 
{
    HbmAddressReq req;
    HbmAddressResp rsp;

    req.set_handle("storage");

    hal::allochbm_address(req, &rsp);
    ASSERT_TRUE((uint64_t)rsp.addr() != 0xFFFFFFFFFFFFFFFFULL);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
