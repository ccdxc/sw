//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the declarations of global routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_BASE_HPP__
#define __TEST_API_UTILS_BASE_HPP__

#include <gtest/gtest.h>
#include "nic/apollo/test/base/base.hpp"
#include "nic/apollo/test/base/utils.hpp"

// globals
extern test_case_params_t g_tc_params;

// function prototypes
int api_test_program_run(int argc, char **argv);

#endif  // __TEST_API_UTILS_BASE_HPP__
