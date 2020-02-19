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

#include "app_test.hpp"

namespace test {
namespace athena_app {

bool session_aging_init(void *arg);
bool session_aging_expiry_log_enable(void *arg);
bool session_aging_expiry_log_disable(void *arg);
bool session_aging_sim_mode(void *arg);
bool session_aging_hw_mode(void *arg);
bool session_aging_fini(void *arg);
bool session_aging_clear_full(void *arg);
bool session_aging_simple(void *arg);
bool session_aging_random_start_count(void *arg);
bool session_aging_full(void *arg);

#define SESSION_AGING_NAME2FN_MAP                                               \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_init),                  \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_sim_mode),              \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_hw_mode),               \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_fini),                  \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_clear_full),            \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_simple),                \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_random_start_count),    \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_expiry_log_enable),     \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_expiry_log_disable),    \
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(session_aging_full),                  \

}    // namespace athena_app
}    // namespace test

#endif   // __TEST_ATHENA_SESSION_AGING_HPP__
