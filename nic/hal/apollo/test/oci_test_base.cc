/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    base class for all gtests
 * @brief   OCI base test class provides init and teardown routines
 *          common to all tests
 */

#include <stdarg.h>
#include "oci_test_base.hpp"
#include "nic/hal/apollo/include/api/oci.hpp"
#include "nic/hal/apollo/include/api/oci_init.hpp"

/**  @brief    callback invoked for error traces
 */
static int
error_trace_cb (const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    printf(logbuf);
    printf("\n");
    va_end(args);

    return 0;
}

/**  @brief    callback invoked for debug traces
 */
static int
debug_trace_cb (const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    printf(logbuf);
    printf("\n");
    va_end(args);

    return 0;
}

/**  @brief    called at the beginning of all test cases in this class,
 *             initialize OCI HAL
 */
void
oci_test_base::SetUpTestCase(bool enable_fte) {
    oci_init_params_t     init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = OCI_INIT_MODE_FRESH_START;
    init_params.debug_trace_cb = debug_trace_cb;
    init_params.error_trace_cb = error_trace_cb;
    init_params.cfg_file = "hal.json";
    oci_init(&init_params);
}

/**  @brief    called at the end of all test cases in this class,
 *             cleanup OCI HAL and quit
 */
void
oci_test_base::TearDownTestCase(void) {
    //oci_teardown();
}
