/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef OSAL_CLOCK_H
#define OSAL_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#define OSAL_NSEC_PER_USEC 1000LL
#define OSAL_NSEC_PER_MSEC 1000000LL
#define OSAL_NSEC_PER_SEC  1000000000LL

#define OSAL_USEC_PER_MSEC 1000LL
#define OSAL_USEC_PER_SEC  1000000LL

#define OSAL_MSEC_PER_SEC  1000LL

#ifdef __KERNEL__

#include <linux/ktime.h>

static inline uint64_t osal_get_clock_nsec(void)
{
	return (uint64_t) ktime_get_raw_ns();
}

#else

#include <time.h>

static inline uint64_t osal_get_clock_nsec(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ((uint64_t) ts.tv_sec * OSAL_NSEC_PER_SEC) + (uint64_t) ts.tv_nsec;
}

#endif

static inline uint64_t osal_clock_delta(uint64_t ns1, uint64_t ns2)
{
	return (ns1 - ns2) > (ns2 - ns1) ? (ns2 - ns1) : (ns1 - ns2);
}

#ifdef __cplusplus
}
#endif

#endif
