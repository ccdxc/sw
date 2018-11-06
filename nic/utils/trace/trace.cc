// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <chrono>
#include "nic/include/hal_mem.hpp"
#include "nic/utils/trace/trace.hpp"

namespace utils {

// logger class static constants
const size_t log::k_async_qsize_ = 64 * 1024;    // must be power of 2
const spdlog::async_overflow_policy log::k_async_overflow_policy_ = spdlog::async_overflow_policy::discard_log_msg;
const std::chrono::milliseconds log::k_flush_intvl_ms_ = std::chrono::milliseconds(10);
uint64_t    g_logger_cpu_mask = 0;

// logger class methods
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
    cpu_set_t   cpus;
    uint64_t    cpu_mask = g_logger_cpu_mask;

    CPU_ZERO(&cpus);
    while (cpu_mask != 0) {
        CPU_SET(ffsl(cpu_mask) - 1, &cpus);
        cpu_mask = cpu_mask & (cpu_mask - 1);
    }
    pthread_t current_thread = pthread_self();
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpus);
}

bool
log::init(const char *name, uint64_t cpu_mask, log_mode_e log_mode,
          bool syslogger, const char *trace_file_name,
          size_t file_size, size_t max_files,
          trace_level_e trace_level, syslog_level_e syslog_level) {
    std::function<void()> worker_thread_cb = set_cpu_affinity;

    // first time when *any* logger is created, save the cpu mask and use it
    // for all other logger instances as well
    if (!g_logger_cpu_mask) {
        if (!cpu_mask) {
            g_logger_cpu_mask = 0x1;
        } else {
            g_logger_cpu_mask = cpu_mask;
        }
        g_logger_cpu_mask = cpu_mask;
    }
    syslogger_ = syslogger;
    trace_level_ = trace_level;
    log_level_ = syslog_level;
    if (log_mode == log_mode_async) {
        spdlog::set_async_mode(k_async_qsize_, k_async_overflow_policy_,
                               worker_thread_cb, k_flush_intvl_ms_, NULL);
    }
    if (syslogger) {
        logger_ = spdlog::syslog_logger(name, name, LOG_PID);
    } else {
        logger_ = spdlog::rotating_logger_mt(name, trace_file_name, file_size,
                                             max_files);
    }
    if (logger_) {
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
    return false;
}

log *
log::factory(const char *name, uint64_t cpu_mask, log_mode_e log_mode,
             bool syslogger, const char *trace_file_name,
             size_t file_size, size_t max_files,
             trace_level_e trace_level, syslog_level_e syslog_level) {
    void    *mem;
    log     *new_logger;

    if (!name || !trace_file_name) {
        return NULL;
    }

    mem = calloc(1, sizeof(log));
    if (!mem) {
        return NULL;
    }

    new_logger = new (mem) log();
    if (new_logger->init(name, cpu_mask, log_mode, syslogger, trace_file_name,
                         file_size, max_files,
                         trace_level, syslog_level) == false) {
        new_logger->~log();
        free(new_logger);
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
    free(logger_obj);
}

void
log::set_trace_level(trace_level_e level) {
    if (!syslogger_) {
        logger_->set_level(trace_level_to_spdlog_level(level));
        trace_level_ = level;
    }
}

void
log::set_syslog_level(syslog_level_e level) {
    if (syslogger_) {
        logger_->set_level(syslog_level_to_spdlog_level(level));
        log_level_ = level;
    }
}

void
log::flush(void) {
    logger_->flush();
}

std::shared_ptr<spdlog::logger>
log::logger(void) {
    return logger_;
}

}    // utils
