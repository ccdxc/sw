#include "logger.h"

// GetLogger returns a logger instance
Logger 
GetLogger()
{
    static Logger _logger = NULL;
    static bool initDone = false;

    if (!initDone) {

        auto info_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
            (INFO_LOG_FILENAME, LOG_MAX_FILESIZE, LOG_MAX_FILES);
        auto fatal_err_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
            (ERR_LOG_FILENAME, ERR_LOG_MAX_FILESIZE, 3);

        std::vector<spdlog::sink_ptr> sinks {info_sink, fatal_err_sink};

        _logger = std::make_shared<spdlog::logger>("asicerrord", begin(sinks), end(sinks));

        info_sink->set_level(spdlog::level::trace);
        fatal_err_sink->set_level(spdlog::level::err);
        _logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
        _logger->flush_on(spdlog::level::info);

        initDone = true;
    }

    return _logger;
}

int
asicerrord_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        ERR("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        WARN("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        INFO("{}", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}
