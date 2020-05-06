/*
 * Copyright (c) 2019-2020, Pensando Systems Inc.
 */

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <time.h>

int portmap_init_from_catalog(void);
int default_pcieport(void);

static inline uint64_t
gettimestamp(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

#endif /* __UTILS_HPP__ */
