// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <sdk/timerfd.hpp>
#include <sdk/linkmgr.hpp>
#include <sdk/twheel.hpp>
#include <sdk/thread.hpp>

namespace sdk {
namespace linkmgr {

static sdk::lib::twheel *g_twheel = NULL;

sdk_ret_t
linkmgr_timer_init(void)
{
    // create a timer wheel
    g_twheel = sdk::lib::twheel::factory(TWHEEL_DEFAULT_SLICE_DURATION,
                                         TWHEEL_DEFAULT_DURATION, true);
    if (g_twheel == NULL) {
        SDK_TRACE_ERR("failed to create timer wheel");
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

void *
linkmgr_periodic_thread_start (void *ctxt)
{
    uint64_t            missed;
    timerfd_info_t      timerfd_info;

    SDK_THREAD_INIT(ctxt);

    // prepare the timer fd(s)
    sdk::lib::timerfd_init(&timerfd_info);
    timerfd_info.usecs = TWHEEL_DEFAULT_SLICE_DURATION * TIME_USECS_PER_MSEC;
    if (sdk::lib::timerfd_prepare(&timerfd_info) < 0) {
        SDK_TRACE_ERR("Periodic thread failed to intiialize timerfd");
        return NULL;
    }

    // start the forever loop
    while (TRUE) {
        // wait for timer to fire
        if (sdk::lib::timerfd_wait(&timerfd_info, &missed) < 0) {
            SDK_TRACE_ERR("Periodic thread failed to wait on timer");
            break;
        }

        // drive the timer wheel if enough time elapsed
        g_twheel->tick(missed * TWHEEL_DEFAULT_SLICE_DURATION);
    }

    return NULL;
}

//------------------------------------------------------------------------------
// API invoked by other threads to trigger cb after timeout
// Returns the timer entry used to update/delete the timer
//------------------------------------------------------------------------------
void *
linkmgr_timer_schedule (uint32_t timer_id, uint64_t timeout, void *ctxt,
                        sdk::lib::twheel_cb_t cb, bool periodic)
{
    if (g_twheel) {
        return g_twheel->add_timer(timer_id, timeout, ctxt, cb, periodic);
    }
    return NULL;
}

//------------------------------------------------------------------------------
// API invoked by other threads to delete the scheduled timer
//------------------------------------------------------------------------------
void *
linkmgr_timer_delete (void *timer)
{
    if (g_twheel) {
        return g_twheel->del_timer(timer);
    }
    return NULL;
}

//------------------------------------------------------------------------------
// API invoked by other threads to update the scheduled timer
//------------------------------------------------------------------------------
void *
linkmgr_timer_update (void *timer, uint64_t timeout, bool periodic, void *ctxt)
{
    if (g_twheel) {
        return g_twheel->upd_timer(timer, timeout, periodic, ctxt);
    }
    return NULL;
}

}    // namespace linkmgr
}    // namespace sdk
