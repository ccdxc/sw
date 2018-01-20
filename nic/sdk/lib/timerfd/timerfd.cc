//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sdk/timerfd.hpp>
#include <sdk/timestamp.hpp>

namespace sdk {
namespace lib {

//------------------------------------------------------------------------------
// initiaize information about a given timer fd
//------------------------------------------------------------------------------
void
timerfd_init (timerfd_info_t *pinfo)
{
    pinfo->timer_fd = -1;
    pinfo->usecs = 0;
    pinfo->missed_wakeups = 0;
}

//------------------------------------------------------------------------------
// create and initialize a timer fd, this fd can then be used
// in poll/select system calls eventually
//------------------------------------------------------------------------------
int
timerfd_prepare (timerfd_info_t *pinfo)
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
    sdk::timestamp_from_nsecs(&tspec, pinfo->usecs * TIME_NSECS_PER_USEC);
    itspec.it_interval = tspec;
    itspec.it_value = tspec;
    return timerfd_settime(fd, 0, &itspec, NULL);
}

//------------------------------------------------------------------------------
// wait on a given timer fd and return number of missed wakeups, if any
// TODO: in future, if we have multiple of these, we can use select()
//------------------------------------------------------------------------------
int
timerfd_wait (timerfd_info_t *pinfo, uint64_t *missed)
{
    int         rv;

    // wait for next timer event, and warn any missed events
    *missed = 0;
    rv = read(pinfo->timer_fd, missed, sizeof(*missed));
    if (rv == -1) {
        return -1;
    }
    if (*missed > 1) {
        SDK_TRACE_DEBUG("Periodic thread missed {} wakeups", *missed);
    }
    pinfo->missed_wakeups += *missed;
    return 0;
}

}    // namespace lib
}    // namespace sdk
