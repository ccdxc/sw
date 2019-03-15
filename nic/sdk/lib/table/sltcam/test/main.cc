//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "include/sdk/base.hpp"
#include "lib/table/sltcam/sltcam.hpp"
#include "lib/table/sltcam/test/p4pd_mock/sltcam_p4pd_mock.hpp"
using sdk::table::sltcam;

FILE *logfp;

static int
sltcam_debug_logger (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;
    if (logfp == NULL) {
        logfp = fopen("run.log", "w");
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
    sdk::lib::logger::init(sltcam_debug_logger);
    sltcam_mock_init();
    return RUN_ALL_TESTS();
}
