//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains combined session/conntrack aging test cases for athena
///
//----------------------------------------------------------------------------

#ifndef __TEST_ATHENA_COMBINED_AGING_HPP__
#define __TEST_ATHENA_COMBINED_AGING_HPP__

#include "app_test_utils.hpp"

namespace test {
namespace athena_app {

bool combined_aging_test(test_vparam_ref_t vparam);

#define COMBINED_AGING_NAME2FN_MAP                                      \
    APP_TEST_NAME2FN_MAP_ENTRY(combined_aging_test),                    \
    
}    // namespace athena_app
}    // namespace test

#endif   // __TEST_ATHENA_COMBINED_AGING_HPP__
