
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains combined session/conntrack aging test execution for athena
///
//----------------------------------------------------------------------------
#include "session_aging.hpp"
#include "conntrack_aging.hpp"

namespace test {
namespace athena_app {

static bool
combined_aging_expiry_count_check(void *user_ctx)
{
    bool    conntrack_res;
    bool    sesion_res;

    /*
     * Ensure both check functions are called, but only one of them would
     * need to set the poll_needed param to true.
     */
    conntrack_res = conntrack_4combined_expiry_count_check(true);
    sesion_res = session_4combined_expiry_count_check(false);
    return conntrack_res && sesion_res;
}

bool
combined_aging_test(test_vparam_ref_t vparam)
{
    test_timestamp_t    ts;
    bool    conntrack_res;
    bool    sesion_res;

    ts.time_expiry_set(APP_TIME_LIMIT_EXEC_SECS(
                                ct_tolerance_get().curr_max_tmo() +
                                APP_TIME_LIMIT_EXEC_GRACE));
    ts.time_limit_exec(combined_aging_expiry_count_check, nullptr, 0);

    /*
     * Ensure both check functions are called (for logging purposes).
     */
    conntrack_res = conntrack_4combined_result_check();
    sesion_res = session_4combined_result_check();
    return conntrack_res && sesion_res;
}

}    // namespace athena_app
}    // namespace test
