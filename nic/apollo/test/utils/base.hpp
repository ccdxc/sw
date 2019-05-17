//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the base class delcaration of all test classes
///
//----------------------------------------------------------------------------

#if !defined(__PDS_BASE_TEST_HPP__)
#define __PDS_BASE_TEST_HPP__

#include <gtest/gtest.h>
#include "nic/sdk/lib/logger/logger.hpp"

sdk_trace_level_e g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;

typedef struct test_case_params_t_ {
    const char  *cfg_file;
    bool        enable_fte;
    std::string profile;
} test_case_params_t;

#define BATCH_START()                                                   \
    batch_params.epoch = ++g_batch_epoch;                               \
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK)

#define BATCH_COMMIT()                                                  \
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK)

#define BATCH_COMMIT_FAIL()                                             \
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK)

#define BATCH_COMMIT_FAILURE(exp_reason)                                \
    ASSERT_TRUE(pds_batch_commit() == exp_reason)

#define BATCH_ABORT()                                                   \
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

/// Base class for all gtests. Implements init and teardown routines common
/// to all test cases
class pds_test_base : public ::testing::Test {
protected:
    /// constructor
    pds_test_base() {}

    /// destructor
    virtual ~pds_test_base() {}

    /// called immediately after the constructor before each test
    virtual void SetUp(void) {}

    /// called immediately after each test before the destructor
    virtual void TearDown(void) {}

    /// called at the beginning of all test cases in this class
    static void SetUpTestCase(test_case_params_t &params);

    /// called at the end of all test cases in this class
    static void TearDownTestCase(void);
};

#endif  // __PDS_BASE_TEST_HPP__
