//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "include/sdk/base.hpp"
#include "lib/table/memhash/mem_hash.hpp"
#include "lib/table/memhash/test/p4pd_mock/mem_hash_p4pd_mock.hpp"
using sdk::table::mem_hash;

FILE *logfp;

static int
memhash_debug_logger (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;
    if (logfp == NULL) {
        logfp = fopen("run.log", "w");
        assert(logfp);
    }

    if (trace_level <= sdk::lib::SDK_TRACE_LEVEL_DEBUG) {
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
    sdk::lib::logger::init(memhash_debug_logger);
    return RUN_ALL_TESTS();
}
