#include "logger.h"

// GetLogger returns a logger instance
Logger 
GetLogger()
{
    static Logger _logger = spdlog::rotating_logger_mt("asicerrord", LOG_FILENAME,
                                                       LOG_MAX_FILESIZE, LOG_MAX_FILES);
    static bool initDone = false;

    if (!initDone) {
        _logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
        initDone = true;
#ifdef DEBUG_ENABLE
        spdlog::set_level(spdlog::level::debug);
#endif
    }

    return _logger;
}
