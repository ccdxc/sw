// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_UTILS_UTEST_H_
#define _DELPHI_UTILS_UTEST_H_

#include <unistd.h>
#include "gtest/gtest.h"

// -------------- wrappers for asynchronous assertions -------------------

// ASSERT_EQ_EVENTUALLY asserts (left == right) eventually
// it retries the check every 50ms for up to 5sec
#define ASSERT_EQ_EVENTUALLY(left, right) \
for (int _tidx = 0; _tidx < 100; _tidx++) { \
    if ((left) == (right)) { \
        break; \
    } \
    usleep(1000 * 50); \
} \
ASSERT_EQ(left, right)

// ASSERT_NE_EVENTUALLY asserts (left != right) eventually
// it retries the check every 50ms for up to 5sec
#define ASSERT_NE_EVENTUALLY(left, right) \
for (int _tidx = 0; _tidx < 100; _tidx++) { \
    if ((left) != (right)) { \
        break; \
    } \
    usleep(1000 * 50); \
} \
ASSERT_NE(left, right)

// ASSERT_TRUE_EVENTUALLY asserts condition is true eventually
// it retries the check every 50ms for up to 5sec
#define ASSERT_TRUE_EVENTUALLY(condition) \
    for (int _tidx = 0; _tidx < 100; _tidx++) { \
        if (condition) { \
            break; \
        } \
        usleep(1000 * 50); \
    } \
    ASSERT_TRUE(condition)

// ASSERT_FALSE_EVENTUALLY asserts condition is false eventually
// it retries the check every 50ms for up to 5sec
#define ASSERT_FALSE_EVENTUALLY(condition) \
    for (int _tidx = 0; _tidx < 100; _tidx++) { \
        if (condition) { \
            break; \
        } \
        usleep(1000 * 50); \
    } \
    ASSERT_FALSE(condition)

#endif // _DELPHI_UTILS_UTEST_H_
