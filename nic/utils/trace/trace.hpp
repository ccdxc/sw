// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#pragma once

#define SPDLOG_ENABLE_SYSLOG    1

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

/*
 * Formatting log messages
 *
 * In most cases just replacing the printf format specifiers with {} will just work.
 * For advanced formatting, syntax is similar to the printf formatting, with the
 * addition of the {} and  with : used instead of %. For example, "%03.2f" can be
 * translated to "{:03.2f}".
 *
 * Use douple braces to print braces
 *
 * Examples:
 *
 * 1. Basic usage
 *    log("{}, {}, {}", 'a', 100, "str" ); // Result: "a 100 str"
 *
 * 2. Accessing arguments by position:
 *    log("{0}, {1}, {2}", 'a', 'b', 'c'); // Result: "a, b, c"
 *    log("{2}, {1}, {0}", 'a', 'b', 'c'); // Result: "c, b, a"
 *    log("{0}{1}{0}", "abra", "cad");    // arguments' indices can be repeated, Result: "abracadabra"
 *
 * 3. Aligning the text and specifying a width:
 *    log("{:<30}", "left aligned");  // Result: "left aligned                  "
 *    log("{:>30}", "right aligned"); // Result: "                 right aligned"
 *    log("{:^30}", "centered");      // Result: "           centered           "
 *    log("{:*^30}", "centered");  // use '*' as a fill char // Result: "***********centered***********"
 *
 *  4. Replacing %x and %o and converting the value to different bases:
 *    log("int: {:d};  hex: {:x};  oct: {:o}; bin: {:b}", 42,42,42,42); // Result: "int: 42;  hex: 2a;  oct: 52; bin: 101010"
 *    log("int: {:d};  hex: {:#x};  oct: {:#o};  bin: {:#b}", 42,42,42,42); // Result: "int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010"
 *
 * Refer to http://fmtlib.net/latest/syntax.html for formatting trace messages.
 *
 *
 * Defining custom formatter to dump structures
 *
 * struct MyStruct {
 *    int a,
 *    string b;
 * };
 *
 * std::ostream& operator<<(std::ostream& os, const MyStruct& s)
 * {
 *    return os << fmt::format("{{a={}, b={}}}", s.a, s.b);
 * }
 *
 * MyStruct m = { 1, "foo" };
 * log("m={}", m); 
 */

namespace hal {
namespace utils {

using logger = spdlog::logger;

void logger_init(uint32_t cpu_id, bool sync_mode);
logger *hal_logger(void);
logger *hal_syslogger(void);

enum log_mode_e {
    log_mode_sync     = 0,     // write logs in the context of the caller thread
    log_mode_async    = 1,    // write logs/traces in the context of a backend thread
};

enum trace_level_e {
    trace_none     = 0,        // traces disabled completely
    trace_err      = 1,
    trace_debug    = 2,
};

enum syslog_level_e {
    log_none      = 0,
    log_emerg     = 1,
    log_alert     = 2,
    log_crit      = 3,
    log_err       = 4,
    log_warn      = 5,
    log_notice    = 6,
    log_info      = 7,
    log_debug     = 8,
};

class log {
public:
    static log *factory(uint32_t cpu_id, log_mode_e log_mode, bool syslogger,
                        trace_level_e trace_level, syslog_level_e syslog_level);
    static void destroy(log *logger_obj);
    void set_trace_level(trace_level_e level);
    trace_level_e trace_level(void) const;
    void set_syslog_level(syslog_level_e level);
    syslog_level_e syslog_level(void) const;
    void flush(void);

private:
    uint32_t                           cpu_id_;              // CPU this logger is tied to
    bool                               syslogger_;           // true, if this is for syslogs
    trace_level_e                      trace_level_;         // trace level, if this is for traces
    syslog_level_e                     log_level_;           // syslog level, if this is for syslogs
    spdlog::logger                     *logger_;             // logger instance
    const size_t                       k_async_qsize_;       // async queue size
    const std::chrono::milliseconds    k_flush_intvl_ms_;    // flush interval
    const size_t                       k_max_file_size_;     // max. trace file size
    const size_t                       k_max_files_;         // max. number of trace files before rotating

private:
    log() : k_async_qsize_(64 * 1024),
            k_flush_intvl_ms_(std::chrono::milliseconds(10)),
            k_max_file_size_(10*1024*1024),
            k_max_files_(10) {};
    ~log();
    bool init(uint32_t cpu_id, log_mode_e log_mode, bool syslogger,
              trace_level_e trace_level, syslog_level_e syslog_level);
    static void set_cpu_affinity(void);
    spdlog::level::level_enum trace_level_to_spdlog_level(trace_level_e level);
    spdlog::level::level_enum syslog_level_to_spdlog_level(syslog_level_e level);
};

extern log *trace_logger;
extern log *syslog_logger;

}    // namespace utils
}    // namespace hal

// HAL syslog macros
#define HAL_SYSLOG_ERR(args...)     hal::utils::hal_syslogger()->error(args)
#define HAL_SYSLOG_WARN(args...)    hal::utils::hal_syslogger()->warn(args)
#define HAL_SYSLOG_INFO(args...)    hal::utils::hal_syslogger()->info(args)

#define HAL_TRACE_ERR(args, ...)                                               \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->error("[{}:{}] " args, __func__, __LINE__,   \
                                        ##__VA_ARGS__);                        \
    }                                                                          \
} while (0)

#define HAL_TRACE_ERR_NO_META(args...)                                         \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->error(args);                                 \
    }                                                                          \
} while (0)

#define HAL_TRACE_WARN(args, ...)                                              \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->warn("[{}:{}] " args, __func__, __LINE__,    \
                                       ##__VA_ARGS__);                         \
    }                                                                          \
} while (0)

#define HAL_TRACE_INFO(args, ...)                                              \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->info("[{}:{}] " args, __func__, __LINE__,    \
                                       ##__VA_ARGS__);                         \
    }                                                                          \
} while (0)

#define HAL_TRACE_DEBUG(args, ...)                                             \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->debug("[{}:{}] " args, __func__, __LINE__,   \
                                        ##__VA_ARGS__);                        \
    }                                                                          \
} while (0)

#define HAL_TRACE_DEBUG_NO_META(args...)                                       \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->debug(args);                                 \
    }                                                                          \
} while (0)

#define HAL_ERR_IF(cond, args, ...)                                            \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->error_if("[{}:{}] "  args,                   \
                                           __func__, __LINE__, ##__VA_ARGS__); \
    }                                                                          \
} while (0)
#define HAL_WARN_IF(cond, args, ...)                                           \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->warn_if("[{}:{}] "  args,                    \
                                           __func__, __LINE__, ##__VA_ARGS__); \
    }                                                                          \
} while (0)

#define HAL_INFO_IF(cond, args, ...)                                           \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->info_if("[{}:{}] "  args,                    \
                                           __func__, __LINE__, ##__VA_ARGS__); \
    }                                                                          \
} while (0)

#define HAL_DEBUG_IF(cond, args, ...)                                          \
do {                                                                           \
    if (hal::utils::hal_logger()) {                                            \
        hal::utils::hal_logger()->debug_if("[{}:{}] "  args,                   \
                                           __func__, __LINE__, ##__VA_ARGS__); \
    }                                                                          \
} while (0)

