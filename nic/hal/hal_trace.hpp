// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/include/trace.hpp"

namespace hal {
namespace utils {

logger *hal_logger(void);
logger *hal_syslogger(void);
::utils::trace_level_e hal_trace_level(void);
extern ::utils::log *g_trace_logger;
extern ::utils::log *g_syslog_logger;
void trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                const char *trace_file, size_t file_size, size_t max_files,
                ::utils::trace_level_e trace_level);
void trace_deinit(void);

}    // utils
}    // hal

//------------------------------------------------------------------------------
// HAL syslog macros
//------------------------------------------------------------------------------
#define HAL_SYSLOG_ERR(args...)                                                \
    if (unlikely(hal::utils::hal_syslogger())) {                               \
        hal::utils::hal_syslogger()->error(args);                              \
    }                                                                          \

#define HAL_SYSLOG_WARN(args...)                                               \
    if (unlikely(hal::utils::hal_syslogger())) {                               \
        hal::utils::hal_syslogger()->warn(args);                               \
    }                                                                          \

#define HAL_SYSLOG_INFO(args...)                                               \
    if (unlikely(hal::utils::hal_syslogger())) {                               \
        hal::utils::hal_syslogger()->info(args);                               \
    }                                                                          \

//------------------------------------------------------------------------------
// HAL trace macros
// NOTE: we can't use printf() here if g_trace_logger is NULL, because printf()
// won't understand spdlog friendly formatters
//------------------------------------------------------------------------------
#define HAL_TRACE_ERR(fmt, ...)                                                \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->error("[{}:{}] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_TRACE_ERR_NO_META(fmt...)                                          \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->error(fmt);                                  \
    }                                                                          \

#define HAL_TRACE_WARN(fmt, ...)                                               \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->warn("[{}:{}] " fmt, __func__, __LINE__,     \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_TRACE_INFO(fmt, ...)                                               \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->info("[{}:{}] " fmt, __func__, __LINE__,     \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_TRACE_DEBUG(fmt, ...)                                              \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->debug("[{}:{}] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__);                        \
        hal::utils::hal_logger()->flush();                                     \
    }                                                                          \

#define HAL_TRACE_DEBUG_NO_META(fmt...)                                        \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->debug(fmt);                                  \
    }                                                                          \

#define HAL_ERR_IF(cond, fmt, ...)                                             \
    if (unlikely(hal::utils::hal_logger() && (cond))) {                        \
        hal::utils::hal_logger()->error("[{}:{}] "  fmt,  __func__, __LINE__,  \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_WARN_IF(cond, fmt, ...)                                            \
    if (unlikely(hal::utils::hal_logger() && (cond))) {                        \
        hal::utils::hal_logger()->warn("[{}:{}] "  fmt, __func__, __LINE__,    \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_INFO_IF(cond, fmt, ...)                                            \
    if (unlikely(hal::utils::hal_logger() && (cond))) {                        \
        hal::utils::hal_logger()->info("[{}:{}] "  fmt, __func__, __LINE__,    \
                                       ##__VA_ARGS__);                         \
    }                                                                          \

#define HAL_DEBUG_IF(cond, fmt, ...)                                           \
    if (unlikely(hal::utils::hal_logger() && (cond))) {                        \
        hal::utils::hal_logger()->debug("[{}:{}] "  fmt, __func__, __LINE__,   \
                                        ##__VA_ARGS__);                        \
    }                                                                          \

#define HAL_TRACE_FLUSH()                                                      \
    if (unlikely(hal::utils::hal_logger())) {                                  \
        hal::utils::hal_logger()->flush();                                     \
    }
