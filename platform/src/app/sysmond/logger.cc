/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */
#include "logger.h"
#include "LogMsg.h"
#include "nic/sdk/platform/capri/csrint/csr_init.hpp"

::utils::log *g_trace_logger;
::utils::log *obfl_trace_logger;
::utils::log *g_asicerr_trace_logger;
::utils::log *g_asicerr_obfl_trace_logger;
::utils::log *g_asicerr_obfl_onetime_trace_logger;

// wrapper APIs to get logger
std::shared_ptr<spdlog::logger>
GetLogger (void)
{
    if (g_trace_logger) {
        return g_trace_logger->logger();
    }
    return NULL;
}

std::shared_ptr<spdlog::logger>
GetObflLogger (void)
{
    if (obfl_trace_logger) {
        return obfl_trace_logger->logger();
    }
    return NULL;
}

// wrapper APIs to get logger
std::shared_ptr<spdlog::logger>
GetAsicErrLogger (void)
{
    if (g_asicerr_trace_logger) {
        return g_asicerr_trace_logger->logger();
    }
    return NULL;
}

std::shared_ptr<spdlog::logger>
GetAsicErrObflLogger (void)
{
    if (g_asicerr_obfl_trace_logger) {
        return g_asicerr_obfl_trace_logger->logger();
    }
    return NULL;
}

std::shared_ptr<spdlog::logger>
GetAsicErrObflOnetimeLogger (void)
{
    if (g_asicerr_obfl_onetime_trace_logger) {
        return g_asicerr_obfl_onetime_trace_logger->logger();
    }
    return NULL;
}

static int
sysmon_sdk_logger (std::shared_ptr<spdlog::logger> logger,
                   sdk_trace_level_e tracel_level,
                   const char *logbuf)
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

int
sysmond_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    sysmon_sdk_logger (GetLogger(), tracel_level, logbuf);
    va_end(args);
    return 0;
}

int
sysmond_obfl_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    sysmon_sdk_logger (GetObflLogger(), tracel_level, logbuf);
    va_end(args);
    return 0;
}

void
sysmond_flush_logger (void)
{
    GetLogger()->flush();
    GetObflLogger()->flush();
    GetAsicErrLogger()->flush();
    GetAsicErrObflLogger()->flush();
    GetAsicErrObflOnetimeLogger()->flush();
}

void
initializeLogger (void)
{
    static bool initDone = false;
    LogMsg::Instance().get()->setMaxErrCount(0);
    if (!initDone) {
        g_trace_logger = ::utils::log::factory("sysmond", 0x0,
                                        ::utils::log_mode_sync, false,
                                        NULL, LOG_FILENAME, LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::trace_debug,
                                        ::utils::log_none);
        obfl_trace_logger = ::utils::log::factory("sysmond_obfl", 0x0,
                                        ::utils::log_mode_sync, false,
                                        OBFL_LOG_FILENAME, NULL,
                                        OBFL_LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::trace_debug,
                                        ::utils::log_none);
        g_asicerr_trace_logger = ::utils::log::factory("asicerrord", 0x0,
                                        ::utils::log_mode_sync, false,
                                        NULL, ASICERR_LOG_FILENAME,
                                        LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::trace_debug,
                                        ::utils::log_none);
        g_asicerr_obfl_trace_logger = ::utils::log::factory(
                                        "asicerrord_obfl", 0x0,
                                        ::utils::log_mode_sync, false,
                                        ASICERR_OBFL_LOG_FILENAME, NULL,
                                        OBFL_LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::trace_debug,
                                        ::utils::log_none);
        g_asicerr_obfl_onetime_trace_logger = ::utils::log::factory(
                                        "asicerrord_obfl_onetime", 0x0,
                                        ::utils::log_mode_sync, false,
                                        ASICERR_OBFL_LOG_ONETIME_FILENAME, NULL,
                                        OBFL_LOG_MAX_FILESIZE,
                                        LOG_MAX_FILES, ::utils::trace_debug,
                                        ::utils::trace_debug,
                                        ::utils::log_none);
        initDone = true;
    }
}
