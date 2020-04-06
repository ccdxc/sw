//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <stdio.h>
#include "include/sdk/base.hpp"
#include "nic/utils/ftlite/ftlite.hpp"
#include "nic/utils/ftlite/test/p4pd_mock/ftlite_p4pd_mock.hpp"

#include "base.hpp"

void run_basic_tests();

static int
ftl_debug_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
                  const char *format, ...)
{
    char logbuf[1024];
    va_list args;
    static FILE *logfp = NULL;

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
    sdk::lib::logger::init(ftl_debug_logger);
    return RUN_ALL_TESTS();
}
