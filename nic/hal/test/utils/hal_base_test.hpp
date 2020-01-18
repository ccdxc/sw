//----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------

#pragma once

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

#define IF_ID_OFFSET 255
#define UPLINK_IF_ID_OFFSET 127

class hal_base_test : public ::testing::Test {
protected:
  hal_base_test() {
  }

  virtual ~hal_base_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase();

#if 0
  static void SetUpTestCase(const char c_file[]);
#endif

  static void SetUpTestCase(bool disable_fte, std::string c_file="hal.json", std::string vmotion_port = "0");

  static void SetUpTestCaseGrpc();

  // Will be called at the end of all test cases in this class
  static void TearDownTestCase(void);
};
