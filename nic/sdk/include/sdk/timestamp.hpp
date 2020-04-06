//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// timestamp related helper functions
//-----------------------------------------------------------------------------

#ifndef __SDK_TIMESTAMP_HPP__
#define __SDK_TIMESTAMP_HPP__

#include <assert.h>
#include <time.h>
#include "include/sdk/base.hpp"

namespace sdk {

// define our typedef on top of struct timespec
typedef struct timespec timespec_t;

#define TIME_MSECS_PER_SEC                           1000
#define TIME_MSECS_PER_MIN                           (60 * TIME_MSECS_PER_SEC)
#define TIME_NSECS_PER_SEC                           1000000000ULL
#define TIME_USECS_PER_SEC                           1000000
#define TIME_USECS_PER_MSEC                          1000
#define TIME_NSECS_PER_MSEC                          1000000L
#define TIME_NSECS_PER_USEC                          1000

#define TIME_STR_SIZE 30

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
timestamp_add (timespec_t *ts, long secs, uint64_t nsecs)
{
    ts->tv_nsec += nsecs;
    if ((uint64_t)ts->tv_nsec >= TIME_NSECS_PER_SEC) {
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
// helper function to compute timestamp diff
//--------------------------------------------------------------------------
static inline void
timestamp_subtract_common (timespec_t *ts1, timespec_t *ts2)
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
// do ts1 -= ts2 if ts1 > ts2 or else return zero timestamp
//--------------------------------------------------------------------------
static inline void
timestamp_subtract (timespec_t *ts1, timespec_t *ts2)
{
    timestamp_subtract_common(ts1, ts2);
}

//---------------------------------------------------------------------------
// return ts = ts1 - ts2 if ts1 > ts2 and else return zero timestamp
//---------------------------------------------------------------------------
static inline timespec_t
timestamp_diff (timespec_t *ts1, timespec_t *ts2)
{
    timespec_t    ts_diff;

    ts_diff = *ts1;
    timestamp_subtract_common(&ts_diff, ts2);
    return ts_diff;
}

//--------------------------------------------------------------------------
// convert time from nanoseconds to timespec format
//--------------------------------------------------------------------------
static inline void
timestamp_from_nsecs (timespec_t *ts, uint64_t nsecs)
{
    assert((ts != NULL) && (nsecs != 0));
    ts->tv_sec = nsecs/TIME_NSECS_PER_SEC;
    ts->tv_nsec = nsecs % TIME_NSECS_PER_SEC;
}

//--------------------------------------------------------------------------
// convert time from timespec format to nanoseconds
//--------------------------------------------------------------------------
static inline void
timestamp_to_nsecs (timespec_t *ts, uint64_t *nsecs)
{
    assert((ts != NULL) && (nsecs != NULL));
    *nsecs = ts->tv_nsec;
    *nsecs += (uint64_t) (ts->tv_sec * TIME_NSECS_PER_SEC);
}

//--------------------------------------------------------------------------
// convert time from timespec format to nanoseconds
//--------------------------------------------------------------------------
static inline void
timestamp_to_nsecs (timespec_t *ts, int64_t *nsecs)
{
    assert((ts != NULL) && (nsecs != NULL));
    *nsecs = ts->tv_nsec;
    *nsecs += (int64_t) (ts->tv_sec * TIME_NSECS_PER_SEC);
}

//--------------------------------------------------------------------------
// 1. calculate the duration between ts1 and ts2.
// 2. print the duration in 1.xxxxxxxxx format to buff
//--------------------------------------------------------------------------
static inline size_t
timestamp_diff_to_str (timespec_t *ts1, timespec_t *ts2, char *buff,
                       size_t size)
{
    SDK_ASSERT((ts1 != NULL) && (ts2 != NULL) && (buff != NULL));
    timespec_t diff = timestamp_diff(ts1, ts2);

    return snprintf(buff, size, "%lu.%.9lu", diff.tv_sec, diff.tv_nsec);
}

}    // namespace sdk

using sdk::timespec_t;

#endif    // __SDK_TIMESTAMP_HPP__
