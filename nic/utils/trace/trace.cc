#include <chrono>
#include "trace.hpp"

namespace hal {
namespace utils {

const auto LOG_ASYNC_QUEUE_SIZE = 64 * 1024; // Must be power of 2
const auto LOG_OVERFLOW_POLICY = spdlog::async_overflow_policy::block_retry;
//const auto LOG_FLUSH_INTERVAL = std::chrono::milliseconds(10000);
//const auto LOG_FILENAME = "hal.log";
//const auto LOG_MAX_FILESIZE = 10*1024*1024;
//const auto LOG_MAX_FILES = 10;

logger& hal_logger() {
    static std::shared_ptr<spdlog::logger> _logger = []{
        spdlog::set_level(spdlog::level::debug);
        //spdlog::set_async_mode(LOG_ASYNC_QUEUE_SIZE, LOG_OVERFLOW_POLICY);
        spdlog::set_sync_mode();
        spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
        return spdlog::stdout_logger_mt("hal");
        //return spdlog::rotating_logger_mt("hal",LOG_FILENAME, LOG_MAX_FILESIZE, LOG_MAX_FILES);
    }();

    return *_logger;
}
}
}
