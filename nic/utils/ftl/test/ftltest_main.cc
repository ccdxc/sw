//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "ftltest_main.hpp"

sdk_trace_level_e g_trace_level = sdk::lib::SDK_TRACE_LEVEL_VERBOSE;
uint32_t g_no_of_threads = 2;
FILE *logfp;

static int
ftl_debug_logger (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;
    if (logfp == NULL) {
        logfp = fopen("run.log", "w");
        assert(logfp);
    }

    if (trace_level <= g_trace_level) {
        va_start(args, format);
        vsnprintf(logbuf, sizeof(logbuf), format, args);
        fprintf(logfp, "%s\n", logbuf);
        va_end(args);
        fflush(logfp);
    }
    return 0;
}

static void
get_thread_count (int argc, char **argv)
{
    std::string ptrn = "threads=";
    for (auto i=0; i<argc; i++) {
        std::string arv = argv[i];
        auto found = arv.find(ptrn);
        if (found != std::string::npos) {
            found += ptrn.length();
            auto thread_count = arv.substr(found);
            std::cout << "thread count : " << thread_count << "\n";
            try {
                g_no_of_threads = std::stoi(thread_count);
            } catch (...) {
                g_no_of_threads = 2;
            }
        }
    }
}

int
main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    sdk::lib::logger::init(ftl_debug_logger);
    get_thread_count(argc, argv);
    return RUN_ALL_TESTS();
}
