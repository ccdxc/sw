// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <time.h>

#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/platform/diag/rtc_test.h"
#include "nic/sdk/platform/diag/cpld_test.h"
#include "nic/sdk/platform/diag/temp_sensor_test.h"
#include "nic/sdk/platform/diag/diag_utils.h"
#include "nic/sdk/lib/logger/logger.hpp"

#define ARRAY_LEN(var)   (int)((sizeof(var)/sizeof(var[0])))

#define DIAG_TRACE_ERR(fmt...)             \
    if (GetCurrentLogger()) {              \
        GetCurrentLogger()->error(fmt);    \
    }

#define DIAG_TRACE_INFO(fmt...)            \
    if (GetCurrentLogger()) {              \
        GetCurrentLogger()->info(fmt);     \
    }

typedef std::shared_ptr<spdlog::logger> Logger;
static Logger current_logger;
using namespace sdk::platform::diag;

struct test_info
{
    const char *test_name;
    const char *test_banner;
    diag_ret_e       (*func)(test_mode_e mode, int argc, char *argv[]);
    const char *desc;
};

struct test_info tests_list[] = 
{
    {"cpld", "CPLD Test", cpld_test, "Run the CPLD Test"},
    {"temp-sensor", "Temperature Sensor Test", temp_sensor_test, "Run the Temperature Sensor Test"},
    {"rtc", "RTC Test", rtc_test, "Run the RTC Test"},
};

int usage(int argc, char* argv[]);

Logger CreateLogger(const char* log_name) {

    char log_path[64] = {0};
    snprintf(log_path, sizeof(log_path), "/var/log/pensando/%s", log_name);

    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
        (log_path, 1*1024*1024, 3);

    Logger _logger = std::make_shared<spdlog::logger>(log_name, rotating_sink);

    _logger->set_pattern("[%Y-%m-%d_%H:%M:%S.%e] %L %v");

#ifdef DEBUG_ENABLE
    spdlog::set_level(spdlog::level::debug);
#endif

    return _logger;
}

void SetCurrentLogger(Logger logger) {
    current_logger = logger;

    return;
}

Logger GetCurrentLogger() {
    return current_logger;
}

int diag_logger (uint32_t mod_id, sdk_trace_level_e trace_level,
                 const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    va_end(args);

    switch (trace_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        DIAG_TRACE_ERR("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        DIAG_TRACE_INFO("{}", logbuf);
        break;
    default:
        break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int i;
    test_mode_e mode = TEST_MODE_MAX;
    char diag_banner[64] = {0};
    char logname[128] = {0};
    char log_timestamp[64] = {0};
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    Logger offline_diag_logger;

    if (argc < 2)
        return usage(argc, argv);

    if (!strcmp(argv[1], "offline"))
        mode = OFFLINE_DIAG;
    else if (!strcmp(argv[1], "online"))
        mode = ONLINE_DIAG;
    else if (!strcmp(argv[1], "post"))
        mode = POST;
    else
    {
        printf("Invalid test mode\n");
        return usage(argc, argv);
    }

    sprintf(log_timestamp, "%d%d%d-%d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    //Create the logger
    sprintf(logname, "%s%s.%s.log", argv[1], (strcmp(argv[1], "post") ? "-diags" : ""), log_timestamp);
    offline_diag_logger = CreateLogger(logname);
    SetCurrentLogger(offline_diag_logger);

    sdk::lib::logger::init(diag_logger);

    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
  
    sprintf(diag_banner, "%s%s result", argv[1], (strcmp(argv[1], "post") ? "-diags" : ""));

    if ((argc == 2) || ((argc > 2) && (!strcmp(argv[2], "all"))))
    {
        //Print the Banner for diag test results
        PRINT_TEST_REPORT_BANNER(diag_banner);

        //Run all the tests in the tests_list
        for (i = 0; i < ARRAY_LEN(tests_list); i++)
        {
            tests_list[i].func(mode, argc-2, argv+2);
        }
    }
    else //argc > 2 && user selected test
    {
        //Run the specific test user has provided
        for (i = 0; i < ARRAY_LEN(tests_list); i++)
        {
            if (!strcmp(argv[2], tests_list[i].test_name))
            {
                tests_list[i].func(mode, argc-2, argv+2);
            }
        }
    }

    return 0;
}

int usage(int argc, char* argv[])
{
    int i;
    printf("Usage: %s <mode> [test] [test args]\n",argv[0]);
    printf("Possible modes are:\n");
    printf("   %-16s\n","offline");
    printf("   %-16s\n","online");
    printf("   %-16s\n","post");

    printf("Available tests are: \n");
    for( i = 0; i < ARRAY_LEN(tests_list); i++ )
        if( tests_list[i].desc )
            printf("   %-16s %s\n",tests_list[i].test_name,tests_list[i].desc);

    printf("\nRun '%s <mode> <test_name> -help' for more information on a specific test.\n", argv[0]);
    return 1;
}

