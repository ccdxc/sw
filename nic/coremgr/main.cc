//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/coremgr/coremgr.hpp"
#include "nic/sdk/platform/coremgr/src/coremgr.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/include/sdk/base.hpp"

//------------------------------------------------------------------------------
// globals
//------------------------------------------------------------------------------
typedef std::shared_ptr<spdlog::logger> Logger;
Logger g_coremgr_logger;

//------------------------------------------------------------------------------
// logger callback passed to SDK lib
//------------------------------------------------------------------------------
static int
coremgr_trace_cb (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        COREMGR_TRACE_ERR_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        COREMGR_TRACE_WARN_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        COREMGR_TRACE_INFO_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        COREMGR_TRACE_DEBUG_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        COREMGR_TRACE_VERBOSE_NO_META("{}", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}

void coremgr_logger_init(void)
{
    static bool initDone = false;

    if (!initDone) {
        char filename[PATH_MAX];

        snprintf(filename, PATH_MAX, "%s/coremgr.log", LOG_DIRECTORY);
        g_coremgr_logger = spdlog::basic_logger_mt("coremgr", filename, true);

        g_coremgr_logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
        spdlog::set_level(spdlog::level::debug);
    }
}

int main(int argc, char *argv[])
{
    coremgr_logger_init();
    sdk::lib::logger::init(coremgr_trace_cb);

    return sdk::platform::coremgr::dump_core(argc, argv);
}
