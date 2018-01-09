#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include "sdk/thread.hpp"
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

// timer fd information
struct periodic_info {
    int         timer_fd;
    uint64_t    usecs;
    uint64_t    missed_wakeups;
} __PACK__;

// thread local variables
thread_local struct periodic_info t_pinfo;

//------------------------------------------------------------------------------
// initiaize information about a given timer fd
//------------------------------------------------------------------------------
static void
timerfd_init (struct periodic_info *pinfo)
{
    pinfo->timer_fd = -1;
    pinfo->usecs = 0;
    pinfo->missed_wakeups = 0;
}

//------------------------------------------------------------------------------
// create and initialize a timer fd, this fd can then be used
// in poll/select system calls eventually
//------------------------------------------------------------------------------
static int
timerfd_prepare (struct periodic_info *pinfo)
{
    int                  fd;
    struct itimerspec    itspec;
    timespec_t           tspec;

    // create timer fd
    fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd == -1) {
        return fd;
    }
    pinfo->missed_wakeups = 0;
    pinfo->timer_fd = fd;

    // initialize the timeout
    timestamp_from_nsecs(&tspec, pinfo->usecs * TIME_NSECS_PER_USEC);
    itspec.it_interval = tspec;
    itspec.it_value = tspec;
    return timerfd_settime(fd, 0, &itspec, NULL);
}

//------------------------------------------------------------------------------
// wait on a given timer fd and return number of missed wakeups, if any
// TODO: in future, if we have multiple of these, we can use select()
//------------------------------------------------------------------------------
static int
timerfd_wait (struct periodic_info *pinfo, uint64_t *missed)
{
    int         rv;

    // wait for next timer event, and warn any missed events
    *missed = 0;
    rv = read(pinfo->timer_fd, missed, sizeof(*missed));
    if (rv == -1) {
        return -1;
    }
    if (*missed > 1) {
        HAL_TRACE_WARN("Periodic thread missed {} wakeups", *missed);
    }
    pinfo->missed_wakeups += *missed;
    return 0;
}

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
    timerfd_init(&t_pinfo);
    t_pinfo.usecs = TWHEEL_DEFAULT_SLICE_DURATION * TIME_USECS_PER_MSEC;
    if (timerfd_prepare(&t_pinfo) < 0) {
        HAL_TRACE_ERR("Periodic thread failed to intiialize timer fd");
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
        if (timerfd_wait(&t_pinfo, &missed) < 0) {
            HAL_TRACE_ERR("Periodic thread failed to wait on timer");
            break;
        }

        // drive the timer wheel if enough time elapsed
        if (TRUE) {
            g_twheel->tick(missed * TWHEEL_DEFAULT_SLICE_DURATION);
        }
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

}    // namespace periodic
}    // namespace hal
