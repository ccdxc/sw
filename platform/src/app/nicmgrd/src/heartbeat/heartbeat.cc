//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/core/core.hpp"
#include "heartbeat.hpp"
#include "nic/sdk/include/sdk/periodic.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"

#define NICMGR_HEARTBEAT_SCAN_INTVL        2    // in seconds

namespace nicmgr {
namespace hb {

static void *g_hb_timer;
static bool g_nicmgr_health;

//------------------------------------------------------------------------------
// callback invoked periodically to check for the health of the NICMgr threads
//------------------------------------------------------------------------------
static void
heartbeat_monitor_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    timespec_t          curr_ts, ts_diff, hb_ts;
    sdk::lib::thread    *nicmgr_hb_thread;

    clock_gettime(CLOCK_MONOTONIC, &curr_ts);
    // skip main thread as it is gRPC wait call all the time
    for (uint32_t tid = NICMGR_THREAD_ID_MIN + 1; tid < NICMGR_THREAD_ID_MAX; tid++) {
        if ((nicmgr_hb_thread = hal_thread_get(tid)) != NULL) {
            hb_ts = nicmgr_hb_thread->heartbeat_ts();
            ts_diff = sdk::timestamp_diff(&curr_ts, &hb_ts);
            if (ts_diff.tv_sec >= NICMGR_HEARTBEAT_SCAN_INTVL) {
                g_nicmgr_health = false;
                return;
            }
        }
    }
    g_nicmgr_health = true;
    return;
}

//------------------------------------------------------------------------------
// heartbeat module initialization
//------------------------------------------------------------------------------
uint32_t
heartbeat_init (void)
{
    while (!sdk::lib::periodic_thread_is_running()) {
        pthread_yield();
    }
    g_hb_timer = sdk::lib::timer_schedule(
                     NICMGR_TIMER_ID_HEARTBEAT,
                     NICMGR_HEARTBEAT_SCAN_INTVL * TIME_MSECS_PER_SEC,
                     (void *)0,    // ctxt
                     heartbeat_monitor_cb, true);
    if (g_hb_timer == NULL) {
        return (1);
    }
    return (0);
}

//------------------------------------------------------------------------------
// Check if hal is healthy
//------------------------------------------------------------------------------
bool
is_nicmgr_healthy (void)
{
    return g_nicmgr_health;
}

}    // namespace hb
}    // namespace nicmgr 

