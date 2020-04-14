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
#include "nic/apollo/api/include/athena/pds_base.h"

namespace test {
namespace athena_app {

bool session_aging_init(test_vparam_ref_t vparam);
bool session_aging_expiry_log_set(test_vparam_ref_t vparam);
bool session_aging_force_expired_ts(test_vparam_ref_t vparam);
bool session_aging_fini(test_vparam_ref_t vparam);
bool session_aging_tolerance_secs_set(test_vparam_ref_t vparam);
bool session_table_clear_full(test_vparam_ref_t vparam);
bool session_populate_simple(test_vparam_ref_t vparam);
bool session_populate_random(test_vparam_ref_t vparam);
bool session_populate_full(test_vparam_ref_t vparam);
bool session_aging_test(test_vparam_ref_t vparam);
bool session_aging_normal_tmo_set(test_vparam_ref_t vparam);
bool session_aging_accel_tmo_set(test_vparam_ref_t vparam);
bool session_aging_accel_control(test_vparam_ref_t vparam);
bool session_aging_metrics_show(test_vparam_ref_t vparam);

#define SESSION_AGING_NAME2FN_MAP                                   \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_init),                 \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_force_expired_ts),     \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_fini),                 \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_tolerance_secs_set),   \
    APP_TEST_NAME2FN_MAP_ENTRY(session_table_clear_full),           \
    APP_TEST_NAME2FN_MAP_ENTRY(session_populate_simple),            \
    APP_TEST_NAME2FN_MAP_ENTRY(session_populate_random),            \
    APP_TEST_NAME2FN_MAP_ENTRY(session_populate_full),              \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_test),                 \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_expiry_log_set),       \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_normal_tmo_set),       \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_accel_tmo_set),        \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_accel_control),        \
    APP_TEST_NAME2FN_MAP_ENTRY(session_aging_metrics_show),         \


pds_ret_t session_aging_expiry_fn(uint32_t expiry_id,
                                  pds_flow_age_expiry_type_t expiry_type,
                                  void *user_ctx);
bool session_4combined_expiry_count_check(bool poll_needed = false);
bool session_4combined_result_check(void);

const aging_tolerance_t& session_tolerance_get(void);

}    // namespace athena_app
}    // namespace test

#endif   // __TEST_ATHENA_SESSION_AGING_HPP__
