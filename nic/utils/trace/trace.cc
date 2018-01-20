#include <chrono>
#include "nic/utils/trace/trace.hpp"

namespace hal {
namespace utils {

const auto LOG_ASYNC_QUEUE_SIZE = 64 * 1024; // Must be power of 2
const auto LOG_OVERFLOW_POLICY  = spdlog::async_overflow_policy::block_retry;
static std::shared_ptr<spdlog::logger> _logger = NULL;
static int cpu_id               = 0;
//const auto LOG_FLUSH_INTERVAL = std::chrono::milliseconds(10000);
//const auto LOG_FILENAME = "hal.log";
//const auto LOG_MAX_FILESIZE = 10*1024*1024;
//const auto LOG_MAX_FILES = 10;

// This will be run in the worker thread spawned by spdlog. Return values
// are ignored for setting the affinity, as this is not a critical operation.
static void
set_affinity (void) {
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(cpu_id, &cpus);
    pthread_t current_thread = pthread_self(); 
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpus);
}
const std::function<void()> worker_thread_pre_cb = set_affinity;

void
logger_init (int input_cpu_id, bool async_en)
{
    cpu_id = input_cpu_id;
    spdlog::set_level(spdlog::level::debug);
    if (async_en) {
        spdlog::set_async_mode(LOG_ASYNC_QUEUE_SIZE, LOG_OVERFLOW_POLICY,
                               worker_thread_pre_cb, std::chrono::milliseconds::zero(), NULL);
    }
    spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
    _logger = spdlog::stdout_logger_mt("hal");
    //return spdlog::rotating_logger_mt("hal",LOG_FILENAME, LOG_MAX_FILESIZE, LOG_MAX_FILES);
}

logger* hal_logger() {
    return _logger.get();
}
}
}
