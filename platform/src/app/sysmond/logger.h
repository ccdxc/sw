// {C} Copyright 201 Pensando Systems Inc. All rights reserved.

#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/utils/trace/trace.hpp"

const auto LOG_FILENAME = "/var/log/pensando/sysmond.log";
const auto LOG_MAX_FILESIZE = 1*1024*1024;
const auto LOG_MAX_FILES = 5;
const auto OBFL_LOG_FILENAME = "/obfl/sysmond_err.log";
const auto OBFL_LOG_MAX_FILESIZE = 1*1024*1024;
extern ::utils::log *g_trace_logger;
extern ::utils::log *obfl_trace_logger;

// wrapper APIs to get logger
static inline std::shared_ptr<logger>
GetLogger (void)
{
    if (g_trace_logger) {
        return g_trace_logger->logger();
    }
    return NULL;
}

static inline std::shared_ptr<logger>
GetObflLogger (void)
{
    if (obfl_trace_logger) {
        return obfl_trace_logger->logger();
    }
    return NULL;
}

static int
sysmon_sdk_logger (std::shared_ptr<spdlog::logger> logger, sdk_trace_level_e tracel_level, const char *logbuf)
{
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        TRACE_ERR(logger, "{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        TRACE_WARN(logger, "{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        TRACE_INFO(logger, "{}", logbuf);
        break;
    default:
        break;
    }
    return 0;
}

static int
local_sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    sysmon_sdk_logger (GetLogger(), tracel_level, logbuf);
    va_end(args);
    return 0;
}

static int
obfl_sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    sysmon_sdk_logger (GetObflLogger(), tracel_level, logbuf);
    va_end(args);
    return 0;
}
#endif
