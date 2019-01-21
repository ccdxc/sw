// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DIAG_UTILS_H_
#define _DIAG_UTILS_H_

#include <getopt.h>
#include <sys/types.h>
#include <stdlib.h>
#include "include/sdk/types.hpp"
#include "lib/logger/logger.hpp"

namespace sdk {
namespace platform {
namespace diag {

typedef enum {
    OFFLINE_DIAG,
    ONLINE_DIAG,
    POST,
    TEST_MODE_MAX
}test_mode_e;

typedef enum {
    TEST_PASS,
    TEST_FAIL,
    TEST_INVALID_OPTION,
    TEST_SHOW_HELP
}diag_ret_e;

void print_diag_result(const char *format, ...);
void LOG_TEST_BANNER(const char* test_name);
void LOG_TEST_RESULT(const char* test_name, int retval);
void PRINT_TEST_REPORT_BANNER(const char* diag_type);

} // namespace diag
} // namespace platform
} // namespace sdk
#endif
