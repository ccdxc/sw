#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "sdk/thread.hpp"
#include "sdk/timerfd.hpp"
#include "nic/hal/periodic/periodic.hpp"

//------------------------------------------------------------------------------
//          ALTERNATE DESIGN TO CONSIDER LATER
//
// timer_fd can be made to sit inside twheel class and
// timerfd_init(), timerfd_prepare() can be inside the
// init() method of that class, timerfd() API on twheel
// can then give fd, so app can do select or timerfd_wait()
// this way most details of timerfd can be hidden inside twheel class
//
//------------------------------------------------------------------------------

namespace hal {
namespace periodic {

// global timer wheel for periodic thread's use
sdk::lib::twheel *g_twheel;
static volatile bool g_twheel_is_running = false;

// thread local variables
thread_local timerfd_info_t timerfd_info;

//------------------------------------------------------------------------------
// periodic thread starting point
//------------------------------------------------------------------------------
void *
periodic_thread_init (void *ctxt)
{
    SDK_THREAD_INIT(ctxt);

    // create a timer wheel
    g_twheel = sdk::lib::twheel::factory(TWHEEL_DEFAULT_SLICE_DURATION,
                                         TWHEEL_DEFAULT_DURATION, true);
    if (g_twheel == NULL) {
        HAL_TRACE_ERR("Periodic thread failed to create timer wheel");
        return NULL;
    }

    // prepare the timer fd(s)
    sdk::lib::timerfd_init(&timerfd_info);
    timerfd_info.usecs = TWHEEL_DEFAULT_SLICE_DURATION * TIME_USECS_PER_MSEC;
    if (sdk::lib::timerfd_prepare(&timerfd_info) < 0) {
        HAL_TRACE_ERR("Periodic thread failed to intiialize timerfd");
        return NULL;
    }

    g_twheel_is_running = true;
    return NULL;
}

//------------------------------------------------------------------------------
// periodic thread main loop
//------------------------------------------------------------------------------
void *
periodic_thread_run (void *ctxt)
{
    uint64_t                missed;

    while (TRUE) {
        // wait for timer to fire
        if (sdk::lib::timerfd_wait(&timerfd_info, &missed) < 0) {
            HAL_TRACE_ERR("Periodic thread failed to wait on timer");
            break;
        }

        // drive the timer wheel if enough time elapsed
        g_twheel->tick(missed * TWHEEL_DEFAULT_SLICE_DURATION);
    }
    g_twheel_is_running = false;
    HAL_TRACE_ERR("Periodic thread exiting !!!");

    return NULL;
}

//------------------------------------------------------------------------------
// returns true only if thread timer wheel is running
//------------------------------------------------------------------------------
bool
periodic_thread_is_running (void)
{
    return g_twheel_is_running;
}

//------------------------------------------------------------------------------
// API invoked by other threads to trigger cb after timeout
// Returns the timer entry used to update/delete the timer
//------------------------------------------------------------------------------
void *
timer_schedule (uint32_t timer_id, uint64_t timeout, void *ctxt,
                sdk::lib::twheel_cb_t cb, bool periodic)
{
    if (g_twheel) {
        return g_twheel->add_timer(timer_id, timeout, ctxt, cb, periodic);
    }
    return NULL;
}

//------------------------------------------------------------------------------
// API invoked by other threads to get timeout remaining for the timer.
//------------------------------------------------------------------------------
uint64_t
get_timeout_remaining (void *timer)
{
    if (g_twheel) {
        return g_twheel->get_timeout_remaining(timer);
    }
    return 0;
}

//------------------------------------------------------------------------------
// API invoked by other threads to delete the scheduled timer
//------------------------------------------------------------------------------
void *
timer_delete (void *timer)
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
timer_update (void *timer, uint64_t timeout, bool periodic, void *ctxt)
{
    if (g_twheel) {
        return g_twheel->upd_timer(timer, timeout, periodic, ctxt);
    }
    return NULL;
}

}    // namespace periodic
}    // namespace hal
