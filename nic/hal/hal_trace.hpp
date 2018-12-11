// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __HAL_TRACE_HPP__
#define __HAL_TRACE_HPP__

#include "nic/include/trace.hpp"

namespace hal {
namespace utils {

extern ::utils::log *g_trace_logger;
extern ::utils::log *g_syslog_logger;
void trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                const char *trace_file, size_t file_size, size_t max_files,
                ::utils::trace_level_e trace_level);
void trace_deinit(void);

// wrapper APIs to get logger and syslogger
static inline std::shared_ptr<logger>
hal_logger (void)
{
    if (g_trace_logger) {
        return g_trace_logger->logger();
    }
    return NULL;
}

static inline std::shared_ptr<logger>
hal_syslogger (void)
{
    if (g_syslog_logger) {
        return g_syslog_logger->logger();
    }
    return NULL;
}

static inline ::utils::trace_level_e
hal_trace_level (void)
{
    if (g_trace_logger) {
        return g_trace_logger->trace_level();
    }
    return ::utils::trace_none;
}

}    // utils
}    // hal

using hal::utils::hal_logger;
using hal::utils::hal_syslogger;
using hal::utils::hal_trace_level;

//------------------------------------------------------------------------------
// HAL syslog macros
//------------------------------------------------------------------------------
#define HAL_SYSLOG_ERR(args...)                                                \
    if (likely(hal::utils::hal_syslogger())) {                                 \
        hal::utils::hal_syslogger()->error(args);                              \
    }                                                                          \

#define HAL_SYSLOG_WARN(args...)                                               \
    if (likely(hal::utils::hal_syslogger())) {                                 \
        hal::utils::hal_syslogger()->warn(args);                               \
    }                                                                          \

#define HAL_SYSLOG_INFO(args...)                                               \
    if (likely(hal::utils::hal_syslogger())) {                                 \
        hal::utils::hal_syslogger()->info(args);                               \
    }                                                                          \

//------------------------------------------------------------------------------
// HAL trace macros
// NOTE: we can't use printf() here if g_trace_logger is NULL, because printf()
// won't understand spdlog friendly formatters
//------------------------------------------------------------------------------
#define HAL_TRACE_ERR(fmt, ...)                                                \
    if (likely(hal::utils::hal_logger()) &&                                    \
        (hal_trace_level() >= ::utils::trace_err)) {                           \
        hal::utils::hal_logger()->error("[{}:{}] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_TRACE_ERR_NO_META(fmt...)                                          \
    if (likely(hal::utils::hal_logger()) &&                                    \
        (hal_trace_level() >= ::utils::trace_err)) {                           \
        hal::utils::hal_logger()->error(fmt);                                  \
    }                                                                          \

#define HAL_TRACE_WARN(fmt, ...)                                               \
    if (likely(hal::utils::hal_logger()) &&                                    \
        (hal_trace_level() >= ::utils::trace_warn)) {                          \
        hal::utils::hal_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,     \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_TRACE_INFO(fmt, ...)                                               \
    if (likely(hal::utils::hal_logger()) &&                                    \
        (hal_trace_level() >= ::utils::trace_info)) {                          \
        hal::utils::hal_logger()->info("[{}:{}] " fmt, __func__, __LINE__,     \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_TRACE_DEBUG(fmt, ...)                                              \
    if (likely(hal::utils::hal_logger())) {                                    \
        hal::utils::hal_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_TRACE_DEBUG_NO_META(fmt...)                                        \
    if (likely(hal::utils::hal_logger())) {                                    \
        hal::utils::hal_logger()->debug(fmt);                                  \
    }                                                                          \

#define HAL_ERR_IF(cond, fmt, ...)                                             \
    if (likely(hal::utils::hal_logger() &&                                     \
               (hal_trace_level() >= ::utils::trace_err) && (cond))) {         \
        hal::utils::hal_logger()->error("[{}:{}] "  fmt,  __func__, __LINE__,  \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_WARN_IF(cond, fmt, ...)                                            \
    if (likely(hal::utils::hal_logger() &&                                     \
               (hal_trace_level() >= ::utils::trace_warn) && (cond))) {        \
        hal::utils::hal_logger()->warn("[{}:{}] "  fmt, __func__, __LINE__,    \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_INFO_IF(cond, fmt, ...)                                            \
    if (likely(hal::utils::hal_logger() &&                                     \
               (hal_trace_level() >= ::utils::trace_info) && (cond))) {        \
        hal::utils::hal_logger()->info("[{}:{}] "  fmt, __func__, __LINE__,    \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_DEBUG_IF(cond, fmt, ...)                                           \
    if (likely(hal::utils::hal_logger() &&                                     \
               (hal_trace_level() >= ::utils::trace_debug) && (cond))) {       \
        hal::utils::hal_logger()->debug("[{}:{}] "  fmt, __func__, __LINE__,   \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_TRACE_FLUSH()                                                      \
    if (likely(hal::utils::hal_logger())) {                                    \
        hal::utils::hal_logger()->flush();                                     \
    }

#endif    // __HAL_TRACE_HPP__

