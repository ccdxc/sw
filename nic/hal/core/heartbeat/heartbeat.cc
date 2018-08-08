//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/core/core.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"
#include "nic/hal/core/periodic/periodic.hpp"

#define HAL_HEARTBEAT_SCAN_INTVL        1000

namespace hal {
namespace hb {

static void *g_hb_timer;

//------------------------------------------------------------------------------
// callback invoked periodically to check for the health of the HAL threads
//------------------------------------------------------------------------------
static void
heartbeat_monitor_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    return;
}

//------------------------------------------------------------------------------
// heartbeat module initialization
//------------------------------------------------------------------------------
hal_ret_t
heartbeat_init (void)
{
    while (!hal::periodic::periodic_thread_is_running()) {
        pthread_yield();
    }
    g_hb_timer = hal::periodic::timer_schedule(
                     HAL_TIMER_ID_HEARTBEAT,
                     HAL_HEARTBEAT_SCAN_INTVL,
                     (void *)0,    // ctxt
                     heartbeat_monitor_cb, true);
    if (g_hb_timer == NULL) {
        HAL_TRACE_ERR("Failed to start heart beat timer\n");
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Started HAL heart beat monitoring timer with {}ms intvl",
                    HAL_HEARTBEAT_SCAN_INTVL);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// punch heartbeat for the current thread
//------------------------------------------------------------------------------
void
heartbeat_punch (void)
{
    return;
}

}    // namespace hb
}    // namespace hal

