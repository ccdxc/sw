//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/utils/trace/trace.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

const auto LOG_FILENAME = "/var/log/pensando/p4ctl.log";
const auto LOG_MAX_FILESIZE = 1*1024*1024;
const auto LOG_MAX_FILES = 1;

::utils::log *g_trace_logger;

#define P4CTL_LOGGER g_trace_logger->logger()

/*
 * SDK Logger for CLI:
 * - Only Warnings and Errors are shown on console.
 */
sdk_trace_level_e g_cli_trace_level = sdk::lib::SDK_TRACE_LEVEL_WARN;
static int
cli_sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    if ((int)g_cli_trace_level >= (int)tracel_level)  {
        va_start(args, format);
        vsnprintf(logbuf, sizeof(logbuf), format, args);
        switch (tracel_level) {
        case sdk::lib::SDK_TRACE_LEVEL_ERR:
            TRACE_ERR(P4CTL_LOGGER, "%s\n", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_WARN:
            TRACE_WARN(P4CTL_LOGGER, "%s\n", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_INFO:
            TRACE_INFO(P4CTL_LOGGER, "%s\n", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
            TRACE_DEBUG(P4CTL_LOGGER, "%s\n", logbuf);
            break;
        case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
            TRACE_INFO(P4CTL_LOGGER, "%s\n", logbuf);
            break;
        default:
            break;
        }
        va_end(args);
    }

    return 0;
}

sdk_ret_t
cli_logger_init (void)
{
    g_trace_logger = ::utils::log::factory("p4ctl", 0x0,
            ::utils::log_mode_sync, false,
            NULL, LOG_FILENAME, LOG_MAX_FILESIZE,
            LOG_MAX_FILES, ::utils::trace_debug,
            ::utils::trace_debug,
            ::utils::log_none);

    sdk::lib::logger::init(cli_sdk_logger);

    return SDK_RET_OK;
}
