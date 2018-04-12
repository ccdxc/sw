//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/hal/src/stats/stats.hpp"
#include "nic/hal/periodic/periodic.hpp"
#include "nic/include/hal_state.hpp"

namespace hal {

static thread_local void *t_stats_timer;

#define HAL_STATS_COLLECTION_INTVL            (1 * TIME_MSECS_PER_SEC)

//------------------------------------------------------------------------------
// callback invoked by the HAL periodic thread for stats collection
//------------------------------------------------------------------------------
static void
stats_timer_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    HAL_TRACE_DEBUG("Collecting stats ...");
}

//------------------------------------------------------------------------------
// stats module initialization callback
//------------------------------------------------------------------------------
hal_ret_t
hal_stats_init_cb (hal_cfg_t *hal_cfg)
{

    // no stats functionality in sim mode
    if (hal_cfg->platform_mode == hal::HAL_PLATFORM_MODE_SIM) {
        return HAL_RET_OK;
    }

    // wait until the periodic thread is ready
    while (!hal::periodic::periodic_thread_is_running()) {
        pthread_yield();
    }

    t_stats_timer = hal::periodic::timer_schedule(HAL_TIMER_ID_STATS,
                                                  HAL_STATS_COLLECTION_INTVL,
                                                  (void *)0,    // ctxt
                                                  stats_timer_cb, true);
    if (!t_stats_timer) {
        return HAL_RET_ERR;
    }
    HAL_TRACE_DEBUG("Started stats periodic timer with {} ms intvl",
                    HAL_STATS_COLLECTION_INTVL);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// stats module cleanup callback
//------------------------------------------------------------------------------
hal_ret_t
hal_stats_cleanup_cb (void)
{
    return HAL_RET_OK;
}

}    // namespace
