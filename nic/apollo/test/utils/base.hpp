//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the declarations of global routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_BASE_HPP__
#define __TEST_UTILS_BASE_HPP__

#include <gtest/gtest.h>
#include "nic/sdk/lib/logger/logger.hpp"

// Returns pipeline string
static inline std::string
pipeline_get ()
{
#ifdef APOLLO
    return "apollo";
#elif ARTEMIS
    return "artemis";
#else
    SDK_ASSERT(0);
    return NULL;
#endif
}

// Returns true if pipeline is apollo
static inline bool
apollo ()
{
    return (pipeline_get() == "apollo");
}

// Returns true if pipeline is artemis
static inline bool
artemis ()
{
    return (pipeline_get() == "artemis");
}

// Returns true if mock_mode is enabled, otherwise false
static inline bool
capri_mock_mode (void)
{
    static bool input_check = false;
    static bool capri_mock_mode = true;

    if (!input_check) {
        if (getenv("CAPRI_MOCK_MODE") == NULL)
            capri_mock_mode = false;
        input_check = true;
    }

    return capri_mock_mode;
}

typedef struct test_case_params_t_ {
    const char      *cfg_file;
    bool            enable_fte;
    std::string     profile;
} test_case_params_t;

// Base class for all gtests. Implements init and teardown routines common
// to all test cases
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
    static void SetUpTestCase(test_case_params_t &params);

    // called at the end of all test cases in this class
    static void TearDownTestCase(void);
};

// Export variables
extern sdk_trace_level_e g_trace_level;

#endif  // __TEST_UTILS_BASE_HPP__
