
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains athena_app test utility API
///
//----------------------------------------------------------------------------

#ifndef __ATHENA_APP_TEST_UTILS_HPP__
#define __ATHENA_APP_TEST_UTILS_HPP__

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
#include "script_parser.hpp"
#include "nic/apollo/p4/include/athena_defines.h"
#include "nic/include/ftl_dev_if.hpp"

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
class test_param_t
{
public:
    test_param_t(uint32_t num) : 
        type(TOKEN_TYPE_NUM),
        num(num)
    {
    }
    test_param_t(const std::string& str) : 
        type(TOKEN_TYPE_STR),
        str(str)
    {
    }

    friend class test_vparam_t;

private:
    token_type_t                type;
    uint32_t                    num;
    std::string                 str;
};

/**
 * Vector parameters
 */
class test_vparam_t
{
public:
    test_vparam_t()
    {
    }
    ~test_vparam_t()
    {
        clear();
    }

    void push_back(test_param_t param)
    {
        vparam.push_back(param);
    }

    void clear(void)
    {
        vparam.clear();
    }

    uint32_t size(void) const
    {
        return vparam.size();
    }

    sdk_ret_t num(uint32_t idx,
                  uint32_t *ret_num,
                  bool suppress_err_log = false) const;
    sdk_ret_t str(uint32_t idx,
                  std::string *ret_str,
                  bool suppress_err_log = false) const;
    bool expected_bool(bool dflt = false) const;
    uint32_t expected_num(uint32_t dflt = 0) const;
    const std::string& expected_str(void) const;

private:
    std::vector<test_param_t>   vparam;
};


typedef const test_vparam_t&    test_vparam_ref_t;
typedef bool (*test_fn_t)(test_vparam_ref_t vparam);

#define APP_TEST_NAME2STR(name)                     \
    #name
    
#define APP_TEST_NAME2FN_MAP_ENTRY(name)            \
    {APP_TEST_NAME2STR(name), name}

/*
 * Map of created session/conntrack IDs, for the purpose of
 * cross checking when IDs are aged out.
 */
class id_map_t
{
public:

    ~id_map_t()
    {
        clear();
    }

    void insert(uint32_t id)
    {
        id_map.insert(std::make_pair(id, 0));
    }

    void erase(uint32_t id)
    {
        id_map.erase(id);
    }

    uint32_t find(uint32_t id)
    {
        auto iter = id_map.find(id);
        return iter != id_map.end();
    }

    bool find_erase(uint32_t id)
    {
        if (find(id)) {
            erase(id);
            return true;
        }
        return false;
    }

    void clear(void)
    {
        id_map.clear();
    }

    uint32_t size(void)
    {
        return (uint32_t)id_map.size();
    }

private:
    std::map<uint32_t,uint32_t> id_map;
};

/**
 * Metrics
 */
class age_metrics_t
{
public:

    age_metrics_t(ftl_dev_if::ftl_qtype qtype) :
        qtype(qtype)
    {
        base = {0};
    }

    sdk_ret_t baseline(void);
    uint64_t delta_expired_entries(void) const;
    uint64_t delta_num_qfulls(void) const;

    void show(bool latest = true) const;

private:
    sdk_ret_t refresh(ftl_dev_if::lif_attr_metrics_t& m) const;

    enum ftl_dev_if::ftl_qtype     qtype;
    ftl_dev_if::lif_attr_metrics_t base;
};

#ifndef USEC_PER_SEC
#define USEC_PER_SEC    1000000L
#endif

#define APP_TIME_LIMIT_EXEC_SECS(s)         ((s) * USEC_PER_SEC)
#ifdef __x86_64__
#define APP_TIME_LIMIT_EXEC_DFLT            300 /* seconds */
#else
#define APP_TIME_LIMIT_EXEC_DFLT            120 /* seconds */
#endif

/**
 * Generic timestamp and expiry interval
 */
typedef struct {
    uint64_t                    timestamp;
    uint64_t                    expiry;
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

typedef bool (*time_limit_exec_fn_t)(void *);

static inline void
time_limit_exec(const test_timestamp_t& ts,
                time_limit_exec_fn_t fn,
                void *user_ctx = nullptr)
{
    while (!time_expiry_check(ts)) {
        if ((*fn)(user_ctx)) {
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
bool app_test_exit(test::athena_app::test_vparam_ref_t vparam);

#endif   // __ATHENA_APP_TEST_UTILS_HPP__
