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

using logger = spdlog::logger;

namespace utils {

enum log_mode_e {
    log_mode_sync     = 0,     // write logs in the context of the caller thread
    log_mode_async    = 1,    // write logs/traces in the context of a backend thread
};

enum trace_level_e {
    trace_none     = 0,        // traces disabled completely
    trace_err      = 1,
    trace_warn     = 2,
    trace_info     = 3,
    trace_debug    = 4,
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

#define TRACE_FILE_SIZE_DEFAULT        (10 << 20)     // 10 MB trace file by default
#define TRACE_NUM_FILES_DEFAULT        1              // 1 file by default

// logger class with support for log rotation
class log {
public:
    // cpu_mask is the CPU to which the logger backend threads will be pinned to
    static log *factory(const char *name, uint64_t cpu_mask,
                        log_mode_e log_mode, bool syslogger,
                        const char *trace_file_name,
                        size_t file_size = TRACE_FILE_SIZE_DEFAULT,
                        size_t max_files = TRACE_NUM_FILES_DEFAULT,
                        trace_level_e trace_level = trace_err,
                        syslog_level_e syslog_level = log_notice);
    static void destroy(log *logger_obj);
    void set_trace_level(trace_level_e level);
    trace_level_e trace_level(void) const { return trace_level_; }
    void set_syslog_level(syslog_level_e level);
    syslog_level_e syslog_level(void) const { return log_level_; }
    void flush(void);
    std::shared_ptr<spdlog::logger> logger(void);

private:
    bool                                          syslogger_;                  // true, if this is for syslogs
    trace_level_e                                 trace_level_;                // trace level, if this is for traces
    syslog_level_e                                log_level_;                  // syslog level, if this is for syslogs
    std::shared_ptr<spdlog::logger>               logger_;                     // logger instance
    static const size_t                           k_async_qsize_;              // async queue size
    static const spdlog::async_overflow_policy    k_async_overflow_policy_;    // overflow policy
    static const std::chrono::milliseconds        k_flush_intvl_ms_;           // flush interval

private:
    log() {}
    ~log();
    bool init(const char *name, uint64_t cpu_mask, log_mode_e log_mode,
              bool syslogger, const char *trace_file_name,
              size_t file_size, size_t max_files,
              trace_level_e trace_level, syslog_level_e syslog_level);
    static void set_cpu_affinity(void);
    spdlog::level::level_enum trace_level_to_spdlog_level(trace_level_e level);
    spdlog::level::level_enum syslog_level_to_spdlog_level(syslog_level_e level);
};

void trace_init(const char *name, uint64_t cpu_mask, bool sync_mode,
                const char *trace_file, size_t file_size, size_t max_files,
                trace_level_e trace_level);

void trace_deinit(void);

#define TRACE_ERR(logobj, fmt, ...)                          \
    if (logobj) {                                            \
        logobj->error("[{}:{}] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__);      \
    }                                                        \

#define TRACE_ERR_NO_META(logobj, fmt, ...)                  \
    if (logobj) {                                            \
        logobj->error(fmt);                                  \
    }                                                        \

#define TRACE_WARN(logobj, fmt, ...)                         \
    if (logobj) {                                            \
        logobj->warn("[{}:{}] " fmt, __func__, __LINE__,     \
                                       ##__VA_ARGS__);       \
    }                                                        \

#define TRACE_INFO(logobj, fmt, ...)                         \
    if (logobj) {                                            \
        logobj->info("[{}:{}] " fmt, __func__, __LINE__,     \
                                       ##__VA_ARGS__);       \
    }                                                        \

#define TRACE_DEBUG(logobj, fmt, ...)                        \
    if (logobj) {                                            \
        logobj->debug("[{}:{}] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__);      \
    }                                                        \

#define TRACE_DEBUG_NO_META(logobj, fmt, ...)                \
    if (logobj) {                                            \
        logobj->->debug(fmt);                                \
    }                                                        \

#define ERR_IF(logobj, cond, fmt, ...)                       \
    if (logobj && (cond))) {                                 \
        logobj->error("[{}:{}] "  fmt,  __func__, __LINE__,  \
                                        ##__VA_ARGS__);      \
    }                                                        \

#define WARN_IF(logobj, cond, fmt, ...)                      \
    if (logobj && (cond))) {                                 \
        logobj->warn("[{}:{}] "  fmt, __func__, __LINE__,    \
                                       ##__VA_ARGS__);       \
    }                                                        \

#define INFO_IF(logobj, cond, fmt, ...)                      \
    if (logobj && (cond))) {                                 \
        logobj->info("[{}:{}] "  fmt, __func__, __LINE__,    \
                                       ##__VA_ARGS__);       \
    }                                                        \

#define DEBUG_IF(logobj, cond, fmt, ...)                     \
    if (logobj && (cond))) {                                 \
        logobj->debug("[{}:{}] "  fmt, __func__, __LINE__,   \
                                        ##__VA_ARGS__);      \
    }                                                        \

#define TRACE_FLUSH(logobj)                                  \
    if (logobj) {                                            \
        logobj->flush();                                     \
    }

}    // namespace utils

