// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <chrono>
#include "nic/include/hal_mem.hpp"
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
const std::function<void()> worker_thread_cb = set_cpu_affinity;

void
logger_init (uint32_t logger_cpu_id, bool sync_mode)
{
    // instantiate the logger
    cpu_id = logger_cpu_id;
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
    if (!sync_mode) {
        spdlog::set_async_mode(LOG_ASYNC_QUEUE_SIZE, LOG_OVERFLOW_POLICY,
                               worker_thread_cb,
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

//const size_t log::k_async_qsize_ = 64 * 1024;    // must be power of 2
//const spdlog::async_overflow_policy log::k_async_overflow_policy = spdlog::async_overflow_policy::discard_log_msg;
//const std::chrono::milliseconds log::k_flush_intvl_ms_ = std::chrono::milliseconds(10);
//const std::string log::k_trace_file_name_ = std::string("hal.log");
//const size_t log::k_max_file_size_ = 10*1024*1024;
//const size_t log::k_max_files_ = 10;

spdlog::level::level_enum
log::trace_level_to_spdlog_level(trace_level_e level) {
    switch (level) {
    case trace_none:
        return spdlog::level::off;

    case trace_err:
        return spdlog::level::err;

    case trace_debug:
        return spdlog::level::debug;

    default:
        return spdlog::level::off;
    }
}

spdlog::level::level_enum
log::syslog_level_to_spdlog_level(syslog_level_e level) {
    switch (level) {
    case log_none:
        return spdlog::level::off;

    case log_alert:
    case log_emerg:
    case log_crit:
        return spdlog::level::critical;

    case log_err:
        return spdlog::level::err;

    case log_warn:
        return spdlog::level::warn;

    case log_notice:
    case log_info:
        return spdlog::level::info;

    case log_debug:
        return spdlog::level::debug;

    default:
        return spdlog::level::off;
    }
}

void
log::set_cpu_affinity(void)
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(cpu_id, &cpus);
    pthread_t current_thread = pthread_self(); 
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpus);
}

bool
log::init(uint32_t cpu_id, log_mode_e log_mode, bool syslogger,
          trace_level_e trace_level, syslog_level_e syslog_level) {
    std::function<void()> worker_thread_cb = set_cpu_affinity;

    cpu_id_ = cpu_id;
    syslogger_ = syslogger;
    trace_level_ = trace_level;
    log_level_ = syslog_level;
    if (log_mode == log_mode_async) {
        spdlog::set_async_mode(k_async_qsize_,
                               spdlog::async_overflow_policy::discard_log_msg,
                               worker_thread_cb,
                               // k_flush_intvl_ms_,
                               std::chrono::milliseconds::zero(),
                               NULL);
    }
    if (syslogger) {
        logger_ = spdlog::syslog_logger("hal-syslog", "syslog", LOG_PID).get();
    } else {
        logger_ = spdlog::rotating_logger_mt("hal", "hal.log",
                                             k_max_file_size_,
                                             k_max_files_).get();
    }
    HAL_ASSERT(logger_ != NULL);
    logger_->set_pattern("%L [%Y-%m-%d %H:%M:%S.%e%z] %v");
    if (syslogger) {
        logger_->set_level(syslog_level_to_spdlog_level(syslog_level));
    } else {
        logger_->set_level(trace_level_to_spdlog_level(trace_level));
    }

    // trigger flush if the log severity is error or higher
    logger_->flush_on(spdlog::level::err);

    return true;
}

log *
log::factory(uint32_t cpu_id, log_mode_e log_mode, bool syslogger,
             trace_level_e trace_level, syslog_level_e syslog_level) {
    void    *mem;
    log     *new_logger;

    mem = HAL_CALLOC(HAL_MEM_ALLOC_LIB_LOGGER, sizeof(log));
    if (!mem) {
        return NULL;
    }
    new_logger = new (mem) log();
    if (new_logger->init(cpu_id, log_mode, syslogger,
                         trace_level, syslog_level) == false) {
        new_logger->~log();
        HAL_FREE(HAL_MEM_ALLOC_LIB_LOGGER, new_logger);
        return NULL;
    }
    return new_logger;
}

log::~log() {
    //spdlog::drop(name_);
}

void
log::destroy(log *logger_obj) {
    if (!logger_obj) {
        return;
    }
    logger_obj->~log();
    HAL_FREE(HAL_MEM_ALLOC_LIB_LOGGER, logger_obj);
}

void
log::set_trace_level(trace_level_e level) {
    if (!syslogger_) {
        logger_->set_level(trace_level_to_spdlog_level(level));
        trace_level_ = level;
    }
}

trace_level_e
log::trace_level(void) const {
    return trace_level_;
}

void
log::set_syslog_level(syslog_level_e level) {
    if (syslogger_) {
        logger_->set_level(syslog_level_to_spdlog_level(level));
        log_level_ = level;
    }
}

syslog_level_e
log::syslog_level(void) const {
    return log_level_;
}

void
log::flush(void) {
    logger_->flush();
}

}    // utils
}    // namespace hal
