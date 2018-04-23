// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "nic/delphi/utils/log.hpp"

namespace delphi {


// GetLogger returns a logger instance
Logger GetLogger() {
    static Logger _logger = spdlog::stdout_color_mt("delphi");
    static bool initDone = false;

    if (!initDone) {
        _logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
        initDone = true;
    }

    return _logger;
}

} // namespace delphi
