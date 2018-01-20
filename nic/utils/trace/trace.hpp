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

#define HAL_LOG(level, args...) \
    if(hal::utils::hal_logger()) hal::utils::hal_logger()->log(level, args)
#define HAL_LOG_IF(cond, level, args...) \
    if(hal::utils::hal_logger()) hal::utils::hal_logger()->log(cond, level, args)

#define HAL_TRACE_ERR(args...) HAL_LOG(spdlog::level::err, args)
#define HAL_TRACE_WARN(args...) HAL_LOG(spdlog::level::warn, args)
#define HAL_TRACE_INFO(args...) HAL_LOG(spdlog::level::info, args)
#define HAL_TRACE_DEBUG(args...) HAL_LOG(spdlog::level::debug, args)

#define HAL_ERR_IF(cond, args...) HAL_LOG_IF(cond, spdlog::level::err, args)
#define HAL_WARN_IF(cond, args...)  HAL_LOG_IF(cond, spdlog::level::warn, args)
#define HAL_INFO_IF(cond, args...)  HAL_LOG_IF(cond, spdlog::level::info, args)
#define HAL_DEBUG_IF(cond, args...) HAL_LOG_IF(cond, spdlog::level::debug, args)

namespace hal {
namespace utils {

using logger = spdlog::logger;

void logger_init(int cpu_id, bool async_en);
logger* hal_logger();

} // namespace utils
} // namespace hal
