//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the base class delcaration of all test classes
///
//----------------------------------------------------------------------------

#if !defined(__OCI_BASE_TEST_HPP__)
#define __OCI_BASE_TEST_HPP__

#include <gtest/gtest.h>

/// Base class for all gtests. Implements init and teardown routines common
/// to all test cases
class oci_test_base : public ::testing::Test {
protected:
    /// constructor
    oci_test_base() {}

    /// destructor
    virtual ~oci_test_base() {}

    /// called immediately after the constructor before each test
    virtual void SetUp(void) {}

    /// called immediately after each test before the destructor
    virtual void TearDown(void) {}

    /// called at the beginning of all test cases in this class
    static void SetUpTestCase(const char *cfgfile, bool enable_fte = false);

    /// called at the end of all test cases in this class
    static void TearDownTestCase(void);
};

#endif // end define __OCI_BASE_TEST_HPP__
