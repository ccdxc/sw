// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#pragma once

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

namespace hal {
namespace utils {

using logger = spdlog::logger;

void logger_init(int cpu_id, bool async_en);
logger* hal_logger();

} // namespace utils
} // namespace hal
