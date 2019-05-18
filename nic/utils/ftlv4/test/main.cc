//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "include/sdk/base.hpp"
#include "nic/utils/ftlv4/ftlv4.hpp"
#include "nic/utils/ftlv4/test/p4pd_mock/ftlv4_p4pd_mock.hpp"
using sdk::table::ftlv4;

FILE *logfp;

static int
ftlv4_debug_logger (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;
    if (logfp == NULL) {
        logfp = fopen("run.log", "w");
        assert(logfp);
    }

    if (trace_level <= sdk::lib::SDK_TRACE_LEVEL_VERBOSE) {
        va_start(args, format);
        vsnprintf(logbuf, sizeof(logbuf), format, args);
        fprintf(logfp, "%s\n", logbuf);
        va_end(args);
    }
    return 0;
}

int 
main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    sdk::lib::logger::init(ftlv4_debug_logger);
    return RUN_ALL_TESTS();
}
