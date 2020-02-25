//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/core/core.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"

#define HAL_HEARTBEAT_SCAN_INTVL        10    // in seconds

namespace hal {
namespace hb {

static void *g_hb_timer;
static bool g_hal_health;

//------------------------------------------------------------------------------
// callback invoked periodically to check for the health of the HAL threads
//------------------------------------------------------------------------------
static void
heartbeat_monitor_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    timespec_t          curr_ts, ts_diff, hb_ts;
    sdk::lib::thread    *hal_thread;

    clock_gettime(CLOCK_MONOTONIC, &curr_ts);
    // skip main thread as it is gRPC wait call all the time
    for (uint32_t tid = HAL_THREAD_ID_CFG + 1;
         (tid != HAL_THREAD_ID_DELPHI_CLIENT) && (tid < HAL_THREAD_ID_MAX);
         tid++) {
        if ((hal_thread = hal_thread_get(tid)) != NULL) {
            hb_ts = hal_thread->heartbeat_ts();
            ts_diff = sdk::timestamp_diff(&curr_ts, &hb_ts);
            if (ts_diff.tv_sec >= HAL_HEARTBEAT_SCAN_INTVL) {
                HAL_TRACE_ERR("thread {} missed heartbeat for last {}s.{}ms",
                              hal_thread->name(),
                              ts_diff.tv_sec,
                              ts_diff.tv_nsec/TIME_NSECS_PER_MSEC);
                g_hal_health = false;
                return;
            }
        }
    }
    g_hal_health = true;
    return;
}

//------------------------------------------------------------------------------
// heartbeat module initialization
//------------------------------------------------------------------------------
hal_ret_t
heartbeat_init (void)
{
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }
    g_hb_timer = sdk::lib::timer_schedule(
                     HAL_TIMER_ID_HEARTBEAT,
                     HAL_HEARTBEAT_SCAN_INTVL * TIME_MSECS_PER_SEC,
                     (void *)0,    // ctxt
                     heartbeat_monitor_cb, true);
    if (g_hb_timer == NULL) {
        HAL_TRACE_ERR("Failed to start heart beat timer\n");
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Started HAL heart beat monitoring timer with {}s intvl",
                    HAL_HEARTBEAT_SCAN_INTVL);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Check if hal is healthy
//------------------------------------------------------------------------------
bool
is_hal_healthy (void)
{
    return g_hal_health;
}

}    // namespace hb
}    // namespace hal

