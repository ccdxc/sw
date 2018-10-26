// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "nic/delphi/utils/log.hpp"

namespace delphi {

// global variable
Logger g_delphi_logger = NULL;

// GetLogger returns a logger instance
Logger GetLogger() {
    if (g_delphi_logger == NULL) {
        g_delphi_logger = spdlog::stdout_color_mt("delphi");
        g_delphi_logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
#ifdef DEBUG_ENABLE
        spdlog::set_level(spdlog::level::debug);
#endif
    }

    return g_delphi_logger;
}

// SetLogger sets the logger to use
void SetLogger(Logger lg) {
    if (g_delphi_logger != NULL) {
        spdlog::drop("delphi");
    }
    g_delphi_logger = lg;
}

} // namespace delphi
