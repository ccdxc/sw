//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// timestamp related helper functions
//-----------------------------------------------------------------------------

#ifndef __TIMESTAMP_HPP__
#define __TIMESTAMP_HPP__

#include <assert.h>
#include <time.h>
#include "sdk/base.hpp"

// define our typedef on top of struct timespec
typedef struct timespec timespec_t;

#define TIME_MSECS_PER_SEC                           1000
#define TIME_MSECS_PER_MIN                           (60 * TIME_MSECS_PER_SEC)
#define TIME_NSECS_PER_SEC                           1000000000ULL
#define TIME_USECS_PER_SEC                           1000000
#define TIME_USECS_PER_MSEC                          1000
#define TIME_NSECS_PER_MSEC                          1000000
#define TIME_NSECS_PER_USEC                          1000

//--------------------------------------------------------------------------
// return true if ts1 < ts2
//--------------------------------------------------------------------------
static inline bool
timestamp_before (timespec_t *ts1, timespec_t *ts2)
{
    if ((ts1->tv_sec < ts2->tv_sec) ||
        ((ts1->tv_sec == ts2->tv_sec) && (ts1->tv_nsec < ts2->tv_nsec))) {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------
// return true if ts1 > ts2
//--------------------------------------------------------------------------
static inline bool
timestamp_later (timespec_t *ts1, timespec_t *ts2)
{
    if ((ts1->tv_sec > ts2->tv_sec) ||
        ((ts1->tv_sec == ts2->tv_sec) && (ts1->tv_nsec > ts2->tv_nsec))) {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------
// return true if ts1 = ts2
//--------------------------------------------------------------------------
static inline bool
timestamp_equal (timespec_t *ts1, timespec_t *ts2)
{
    return ((ts1->tv_sec == ts2->tv_sec) &&
                (ts1->tv_nsec == ts2->tv_nsec));
}

//--------------------------------------------------------------------------
// return true if ts1 <= ts2
//--------------------------------------------------------------------------
static inline bool
timestamp_before_or_equal (timespec_t *ts1, timespec_t *ts2)
{
    if ((ts1->tv_sec <= ts2->tv_sec) ||
        ((ts1->tv_sec == ts2->tv_sec) && (ts1->tv_nsec <= ts2->tv_nsec))) {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------
// return true if ts1 >= ts2
//--------------------------------------------------------------------------
static inline bool
timestamp_equal_or_later (timespec_t *ts1, timespec_t *ts2)
{
    if ((ts1->tv_sec >= ts2->tv_sec) ||
        ((ts1->tv_sec == ts2->tv_sec) && (ts1->tv_nsec >= ts2->tv_nsec))) {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------
// do ts1 += ts2(seconds, nseconds)
//--------------------------------------------------------------------------
static inline void
timestamp_add (timespec_t *ts, long secs, unsigned long long nsecs)
{
    ts->tv_nsec += nsecs;
    if ((unsigned long long)ts->tv_nsec >= TIME_NSECS_PER_SEC) {
        ts->tv_sec++;
        ts->tv_nsec -= TIME_NSECS_PER_SEC;
    }
    ts->tv_sec += secs;
}

//--------------------------------------------------------------------------
// do ts1 += ts2
//--------------------------------------------------------------------------
static inline void
timestamp_add_timespecs (timespec_t *ts1, timespec_t *ts2)
{
    timestamp_add(ts1, ts2->tv_sec, ts2->tv_nsec);
}

//--------------------------------------------------------------------------
// do ts1 -= ts2 if ts1 > ts2 or else return zero timestamp
//--------------------------------------------------------------------------
static inline void
timestamp_subtract (timespec_t *ts1, timespec_t *ts2)
{
    if (timestamp_later(ts1, ts2)) {
        if (ts1->tv_nsec < ts2->tv_nsec) {
            ts1->tv_sec -= 1;
            ts1->tv_nsec += TIME_NSECS_PER_SEC;
        }
        ts1->tv_sec -= ts2->tv_sec;
        ts1->tv_nsec -= ts2->tv_nsec;
    } else {
        ts1->tv_sec = ts2->tv_nsec = 0;
    }
}

//--------------------------------------------------------------------------
// convert time from nanoseconds to timespec format
//--------------------------------------------------------------------------
static inline void
timestamp_from_nsecs (timespec_t *ts, unsigned long long nsecs)
{
    assert((ts != NULL) && (nsecs != 0));
    ts->tv_sec = nsecs/TIME_NSECS_PER_SEC;
    ts->tv_nsec = nsecs % TIME_NSECS_PER_SEC;
}

//--------------------------------------------------------------------------
// convert time from timespec format to nanoseconds
//--------------------------------------------------------------------------
static inline void
timestamp_to_nsecs (timespec_t *ts, unsigned long long *nsecs)
{
    assert((ts != NULL) && (nsecs != NULL));
    *nsecs = ts->tv_nsec;
    *nsecs += (unsigned long long) ts->tv_sec * TIME_NSECS_PER_SEC;
}

#endif    // __TIMESTAMP_HPP__

