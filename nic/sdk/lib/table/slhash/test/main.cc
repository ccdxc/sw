//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/sdk/base.hpp"
#include "lib/table/slhash/slhash.hpp"
#include "lib/table/slhash/test/p4pd_mock/slhash_p4pd_mock.hpp"
using sdk::table::slhash;

FILE *logfp;

static int
slhash_debug_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
                     const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;
    if (logfp == NULL) {
        char *filename = strcat(getenv("SDKDIR"), "/slhash_test.log");
        logfp = fopen(filename, "w");
        assert(logfp);
    }

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    fprintf(logfp, "%s\n", logbuf);
    va_end(args);
    return 0;
}

int
main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    sdk::lib::logger::init(slhash_debug_logger);
    slhash_mock_init();
    return RUN_ALL_TESTS();
}
