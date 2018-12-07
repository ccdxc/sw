/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    base class for all gtests
 * @brief   OCI base test class provides init and teardown routines
 *          common to all tests
 */

#include "oci_test_base.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/include/api/oci_init.hpp"

/**  @brief    called at the beginning of all test cases in this class,
 *             initialize OCI HAL
 */
void
oci_test_base::SetUpTestCase(bool enable_fte) {
    // oci_init_params_t    init_params = { 0 };
    // oci_init(&init_params);
}

/**  @brief    called at the end of all test cases in this class,
 *             cleanup OCI HAL and quit
 */
void
oci_test_base::TearDownTestCase(void) {
    //oci_teardown();
}
