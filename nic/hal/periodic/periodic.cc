#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <thread.hpp>
#include <twheel.hpp>
#include "periodic.hpp"

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
hal::utils::twheel *g_twheel;

// timer fd information
struct periodic_info {
    int         timer_fd;
    uint64_t    usecs;
    uint64_t    missed_wakeups;
} __PACK__;

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
periodic_thread_start (void *ctxt)
{
    struct periodic_info    pinfo;
    uint64_t                missed;

    t_curr_thread = (thread *)ctxt;
    HAL_TRACE_DEBUG("Thread {} initializing ...", t_curr_thread->name());

    // create a timer wheel
    g_twheel = hal::utils::twheel::factory(TWHEEL_DEFAULT_SLICE_DURATION,
                                           TWHEEL_DEFAULT_DURATION, true);
    if (g_twheel == NULL) {
        HAL_TRACE_ERR("Periodic thread failed to create timer wheel");
        return NULL;
    }

    // prepare the timer fd(s)
    timerfd_init(&pinfo);
    pinfo.usecs = TWHEEL_DEFAULT_SLICE_DURATION * TIME_USECS_PER_MSEC;
    if (timerfd_prepare(&pinfo) < 0) {
        HAL_TRACE_ERR("Periodic thread failed to intiialize timer fd");
        return NULL;
    }

    while (TRUE) {
        // wait for timer to fire
        if (timerfd_wait(&pinfo, &missed) < 0) {
            HAL_TRACE_ERR("Periodic thread failed to wait on timer");
            break;
        }

        // drive the timer wheel if enough time elapsed
        if (TRUE) {
            g_twheel->tick(missed * TWHEEL_DEFAULT_SLICE_DURATION);
        }
    }
    HAL_TRACE_ERR("Periodic thread exiting !!!");

    return NULL;
}

//------------------------------------------------------------------------------
// callback invoked by the timerwheel to release an object to its slab
//------------------------------------------------------------------------------
void
slab_delay_delete_cb (hal_slab_t slab_id, void *elem)
{
    hal_ret_t    ret;

    if (slab_id < HAL_SLAB_PI_MAX) {
        ret = hal::free_to_slab(slab_id, elem);
    } else if (slab_id < HAL_SLAB_PD_MAX) {
        ret = hal::pd::free_to_slab(slab_id, elem);
    } else {
        HAL_TRACE_ERR("Unexpected slab id {}", slab_id);
        ret = HAL_RET_INVALID_ARG;
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to release elem {} to slab id {}",
                      elem, slab_id);
    }
}

//------------------------------------------------------------------------------
// API invoked by slab library to delay delete its elements.
// NOTE: currently delay delete timeout is 2 seconds, it is expected that any
//       other threads using (a pointer to) this object should be done with this
//       object within this timeout or else this memory can be freed and
//       allocated for other objects and can result in corruptions. Hence, tune
//       this timeout, if needed
//------------------------------------------------------------------------------
hal_ret_t
delay_delete_to_slab (hal_slab_t slab_id, void *elem)
{
    g_twheel->add_timer(slab_id, TIME_MSECS_PER_SEC << 1, elem,
                        (hal::utils::twheel_cb_t)slab_delay_delete_cb, false);
    return HAL_RET_OK;
}

}    // namespace periodic
}    // namespace hal
