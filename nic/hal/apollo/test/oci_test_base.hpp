/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    base class for all gtests
 * @brief   OCI base test class provides init and teardown routines
 *          common to all tests
 */

#if !defined (__OCI_BASE_TEST_HPP__)
#define __OCI_BASE_TEST_HPP__

#include <gtest/gtest.h>

class oci_test_base : public ::testing::Test {
protected:
  /**< @brief    constructor */
  oci_test_base() {}
  /**< @brief    destructor */
  virtual ~oci_test_base() {}
  /**< @brief    called immediately after the constructor before each test */
  virtual void SetUp(void) {}
  /**< @brief    called immediately after each test before the destructor */
  virtual void TearDown(void) {}
  /**< @brief    called at the beginning of all test cases in this class */
  static void SetUpTestCase(bool enable_fte=false);
  /**< @brief    called at the end of all test cases in this class */
  static void TearDownTestCase(void);
};

#endif    /**< __OCI_BASE_TEST_HPP__ */
