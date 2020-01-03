//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the declarations of global routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_BASE_HPP__
#define __TEST_BASE_HPP__

#include <gtest/gtest.h>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

// globals
static const std::string k_sim_arch = "x86";
static const std::string k_hw_arch = "arm";

// Returns target arch string
static inline std::string
aarch_get (void)
{
#ifdef __x86_64__
    return k_sim_arch;
#elif __aarch64__
    return k_hw_arch;
#else
    SDK_ASSERT(0);
    return NULL;
#endif
}

// Returns true on hw
static inline bool
hw (void)
{
    return (aarch_get() == k_hw_arch);
}

// Returns pipeline string
static inline std::string
pipeline_get (void)
{
#ifdef APOLLO
    return "apollo";
#elif defined(ARTEMIS)
    return "artemis";
#elif defined(APULU)
    return "apulu";
#elif defined(ATHENA)
    return "athena";
#else
    SDK_ASSERT(0);
    return NULL;
#endif
}

// Returns true if pipeline is athena
static inline bool
athena (void)
{
    return (pipeline_get() == "athena");
}

// Returns true if pipeline is apulu
static inline bool
apulu (void)
{
    return (pipeline_get() == "apulu");
}

// Returns true if pipeline is apollo
static inline bool
apollo (void)
{
    return (pipeline_get() == "apollo");
}

// Returns true if pipeline is artemis
static inline bool
artemis (void)
{
    return (pipeline_get() == "artemis");
}

// Returns true if CAPRI_MOCK_MODE is enabled, otherwise false
static inline bool
capri_mock_mode (void)
{
    static bool input_check1 = false;
    static bool capri_mock_mode = true;

    if (!input_check1) {
        if (getenv("CAPRI_MOCK_MODE") == NULL)
            capri_mock_mode = false;
        input_check1 = true;
    }

    return capri_mock_mode;
}

static inline bool
agent_mode (void)
{
    static bool input_check2 = false;
    static bool agent_mode = false;

    if (!input_check2) {
        if (getenv("AGENT_MODE") != NULL)
            agent_mode = true;
        input_check2 = true;
    }

    return agent_mode;
}

// test case parameters
typedef struct test_case_params_t_ {
    const char      *cfg_file;        ///< config file
    bool            enable_fte;       ///< Unused
    bool            disable_vpp_mock; ///< vpp mock is not started if true
    std::string     profile;          ///< config profile
} test_case_params_t;

// base class for all gtests.
// implements init and teardown routines common to all test cases
class pds_test_base : public ::testing::Test {
protected:
    // constructor
    pds_test_base() {}

    // destructor
    virtual ~pds_test_base() {}

    // called immediately after the constructor before each test
    virtual void SetUp(void) {}

    // called immediately after each test before the destructor
    virtual void TearDown(void) {}

    // called at the beginning of all test cases in this class
    static void SetUpTestCase(test_case_params_t& params);

    // called at the end of all test cases in this class
    static void TearDownTestCase(void);
};

// externs
extern sdk_trace_level_e g_trace_level;  // trace level, default is DEBUG

#endif  // __TEST_BASE_HPP__
