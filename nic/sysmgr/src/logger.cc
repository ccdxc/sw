#include <stdio.h>
#include <iostream>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "logger.h"

// GetLogger returns a logger instance
Logger GetLogger() {
    static Logger _logger = spdlog::stdout_color_mt("sysmgr");
    static bool initDone = false;

    if (!initDone) {
        _logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
        initDone = true;
#ifdef DEBUG_ENABLE
        spdlog::set_level(spdlog::level::debug);
#endif
    }

    return _logger;
}