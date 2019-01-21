// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "diag_utils.h"
#include <stdarg.h>
namespace sdk {
namespace platform {
namespace diag {

void print_diag_result(const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    printf("%s", logbuf);
    va_end(args);

}

void LOG_TEST_BANNER(const char* test_name)
{
    print_diag_result("%s:\n", test_name);
}

void LOG_TEST_RESULT(const char* test_name, int retval)
{
    print_diag_result("    %-32.32s: %4s\n", test_name, (retval ? "FAIL":"PASS"));
}

void PRINT_TEST_REPORT_BANNER(const char* diag_type)
{
    printf("\n");
    printf("==========================================\n");
    printf(" %s :\n", diag_type);
    printf("==========================================\n");

}

} // namespace diag
} // namespace platform
} // namespace sdk
