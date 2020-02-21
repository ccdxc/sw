//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all session aging test cases for athena
///
//----------------------------------------------------------------------------

#ifndef __TEST_ATHENA_SESSION_AGING_HPP__
#define __TEST_ATHENA_SESSION_AGING_HPP__

#include "app_test_utils.hpp"

namespace test {
namespace athena_app {

bool session_aging_init(test_vparam_ref_t vparam);
bool session_aging_expiry_log_set(test_vparam_ref_t vparam);
bool session_aging_sim_mode(test_vparam_ref_t vparam);
bool session_aging_fini(test_vparam_ref_t vparam);
bool session_aging_clear_full(test_vparam_ref_t vparam);
bool session_aging_simple(test_vparam_ref_t vparam);
bool session_aging_random_start_count(test_vparam_ref_t vparam);
bool session_aging_full(test_vparam_ref_t vparam);
bool session_aging_metrics_show(test_vparam_ref_t vparam);

#define SESSION_AGING_NAME2FN_MAP                                   \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_init),                 \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_sim_mode),             \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_fini),                 \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_clear_full),           \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_simple),               \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_random_start_count),   \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_expiry_log_set),       \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_full),                 \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_metrics_show),         \

}    // namespace athena_app
}    // namespace test

#endif   // __TEST_ATHENA_SESSION_AGING_HPP__
