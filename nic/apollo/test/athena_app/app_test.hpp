//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains
///
//----------------------------------------------------------------------------

#ifndef __ATHENA_APP_TEST_HPP__
#define __ATHENA_APP_TEST_HPP__

#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#endif

#define TEST_LOG_INFO(args...)                      \
    printf(args)


namespace test {
namespace athena_app {

/**
 * Generic test function and parameters
 */
typedef bool (*app_test_fn_t)(void *);
typedef std::pair<app_test_fn_t,void *>     app_test_fn_with_param_t;

#define APP_TEST_NAME2STR(name)                     \
    #name
    
#define APP_TEST_NAME2FN_MAP_ENTRY(name, param)     \
    {APP_TEST_NAME2STR(name), {name, param}}

#define APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(name) \
    APP_TEST_NAME2FN_MAP_ENTRY(name, nullptr)


#ifndef USEC_PER_SEC
#define USEC_PER_SEC    1000000L
#endif

#define APP_TIME_LIMIT_EXEC_SECS(s)         ((s) * USEC_PER_SEC)

/**
 * Generic timestamp and expiry interval
 */
typedef struct {
    uint64_t                timestamp;
    uint64_t                expiry;
} test_timestamp_t;

static inline uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * USEC_PER_SEC + tv.tv_usec);
}

static inline void
time_expiry_set(test_timestamp_t& ts,
                uint64_t expiry)
{
    ts.timestamp = timestamp();
    ts.expiry = expiry;
}

static inline bool
time_expiry_check(const test_timestamp_t& ts)
{
    return (ts.expiry == 0) ||
           ((timestamp() - ts.timestamp) > ts.expiry);
}

typedef bool (*time_limit_exec_fn_t)(void);

static inline void
time_limit_exec(const test_timestamp_t& ts,
                time_limit_exec_fn_t fn)
{
    while (!time_expiry_check(ts)) {
        if ((*fn)()) {
            break;
        }
        usleep(10000);
    }
}

static inline void
randomize_seed(void)
{
    srand(timestamp());
}

static inline uint32_t
randomize_max(uint32_t val_max)
{
    uint32_t rand_num = rand() % (val_max + 1);
    return rand_num ? rand_num : 1;
}

void script_exec(const std::string& scripts_dir,
                 const std::string& script_fname);

}    // namespace athena_app
}    // namespace test

/*
 * Special well-known app_test exit function
 */
#define APP_TEST_EXIT_FN        app_test_exit
#define APP_TEST_EXIT_FN_STR    APP_TEST_NAME2STR(app_test_exit)
bool app_test_exit(void *arg);

#endif   // __ATHENA_APP_TEST_HPP__
