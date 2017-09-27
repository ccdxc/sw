#include "nic/hal/src/internal.hpp"
#include "nic/proto/hal/internal.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

using internal::ProgramAddressReq;
using internal::ProgramAddressResp;
using internal::HbmAddressReq;
using internal::HbmAddressResp;

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


class internal_test : public ::testing::Test {
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

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_initialize();
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
  }
};

// ----------------------------------------------------------------------------
// Resolving a program name to address
// ----------------------------------------------------------------------------
TEST_F(internal_test, test1) 
{
    ProgramAddressReq req;
    ProgramAddressResp rsp;

    req.set_handle("p4plus");
    req.set_prog_name("txdma_stage0.bin");
    req.set_resolve_label(false);

    hal::GetProgramAddress(req, &rsp);
    ASSERT_TRUE(rsp.addr() != 0xFFFFFFFFFFFFFFFFULL);
}

// ----------------------------------------------------------------------------
// Resolving a program name, label to PC offset
// ----------------------------------------------------------------------------
TEST_F(internal_test, test2) 
{
    ProgramAddressReq req;
    ProgramAddressResp rsp;

    req.set_handle("p4plus");
    req.set_prog_name("txdma_stage0.bin");
    req.set_resolve_label(true);
    req.set_label("storage_tx_stage0");

    hal::GetProgramAddress(req, &rsp);
    ASSERT_TRUE(rsp.addr() != 0xFFFFFFFFFFFFFFFFULL);
}

// ----------------------------------------------------------------------------
// Resolving handle into hbm address 
// ----------------------------------------------------------------------------
TEST_F(internal_test, test3) 
{
    HbmAddressReq req;
    HbmAddressResp rsp;

    req.set_handle("storage");

    hal::AllocHbmAddress(req, &rsp);
    ASSERT_TRUE(rsp.addr() != 0xFFFFFFFFFFFFFFFFULL);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
