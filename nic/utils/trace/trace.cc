// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <chrono>
#include "nic/utils/trace/trace.hpp"

namespace hal {
namespace utils {

const auto LOG_ASYNC_QUEUE_SIZE = 64 * 1024;    // must be power of 2
const auto LOG_OVERFLOW_POLICY = spdlog::async_overflow_policy::discard_log_msg;
static std::shared_ptr<spdlog::logger> _logger = NULL;
static std::shared_ptr<spdlog::logger> _syslogger = NULL;
static int cpu_id = 0;
// const auto LOG_FLUSH_INTERVAL = std::chrono::milliseconds(100);
const auto LOG_FILENAME = "hal.log";
const auto LOG_MAX_FILESIZE = 10*1024*1024;
const auto LOG_MAX_FILES = 10;

// this will be run in the context of worker thread(s) spawned by spdlog
static void
set_cpu_affinity (void)
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(cpu_id, &cpus);
    pthread_t current_thread = pthread_self(); 
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpus);
}
const std::function<void()> worker_thread_pre_cb = set_cpu_affinity;

void
logger_init (uint32_t logger_cpu_id, bool sync_mode)
{
    // instantiate the logger
    cpu_id = logger_cpu_id;
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
    if (!sync_mode) {
        spdlog::set_async_mode(LOG_ASYNC_QUEUE_SIZE, LOG_OVERFLOW_POLICY,
                               worker_thread_pre_cb,
                               // LOG_FLUSH_INTERVAL,
                               std::chrono::milliseconds::zero(),
                               NULL);
    }
    //_logger = spdlog::stdout_logger_mt("hal");
    _logger = spdlog::rotating_logger_mt("hal", LOG_FILENAME,
                                         LOG_MAX_FILESIZE, LOG_MAX_FILES);
    // trigger flush if the log severity is error or higher
    _logger->flush_on(spdlog::level::err);

    // instantiate the syslogger now
    _syslogger = spdlog::syslog_logger("hal-syslog", "syslog", LOG_PID);
}

logger *
hal_logger (void)
{
    return _logger.get();
}

logger *
hal_syslogger (void)
{
    return _syslogger.get();
}

}    // utils
}    // namespace hal
