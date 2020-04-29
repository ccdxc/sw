//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __GTEST_TRACE_HPP__
#define __GTEST_TRACE_HPP__

#include "nic/sdk/lib/logger/logger.hpp"

static int
sdk_test_logger (uint32_t mod_id, sdk_trace_level_e tracel_level,
                 const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        printf("%s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        printf("%s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        printf("%s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        printf("%s\n", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        printf("%s\n", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}

#endif // __GTEST_TRACE_HPP__
