#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {


// GetLogger returns a logger instance
Logger GetLogger() {
    static Logger _logger = spdlog::stdout_color_mt("upgrade");
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

} // namespace
