//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// logger library for SDK
//------------------------------------------------------------------------------

#ifndef __SDK_LOGGER_HPP__
#define __SDK_LOGGER_HPP__

#include <stdio.h>
#include <stdarg.h>

namespace sdk {
namespace lib {

enum sdk_trace_level_e {
    SDK_TRACE_LEVEL_NONE    = 0,
    SDK_TRACE_LEVEL_ERR     = 1,
    SDK_TRACE_LEVEL_WARN    = 2,
    SDK_TRACE_LEVEL_INFO    = 3,
    SDK_TRACE_LEVEL_DEBUG   = 4,
    SDK_TRACE_LEVEL_VERBOSE = 5,
};

//------------------------------------------------------------------------------
// trace library that abstracts the app's logger
//------------------------------------------------------------------------------
class logger {
public:
    typedef int (*trace_cb_t)(sdk_trace_level_e trace_level,
                              const char *format, ...);
    static void init(trace_cb_t trace_cb);
    static trace_cb_t trace_cb(void) {
        return trace_cb_ ? trace_cb_ : null_logger_;
    }

private:
    static int null_logger_(sdk_trace_level_e trace_level,
                            const char *fmt, ...) {
        char       logbuf[1024];
        va_list    args;

        va_start(args, fmt);
        vsnprintf(logbuf, sizeof(logbuf), fmt, args);
        return printf("%s\n", logbuf);
    }

private:
    static trace_cb_t     trace_cb_;
};

}    // namespace lib
}    // namespace sdk

using sdk::lib::sdk_trace_level_e;
using sdk_logger = sdk::lib::logger;

#define SDK_TRACE_ERR(fmt, ...)   sdk::lib::logger::trace_cb()(                \
                                      sdk::lib::SDK_TRACE_LEVEL_ERR, "[%s:%d] "\
                                      fmt, __func__, __LINE__, ##__VA_ARGS__)

#define SDK_TRACE_WARN(fmt, ...)  sdk::lib::logger::trace_cb()(                \
                                      sdk::lib::SDK_TRACE_LEVEL_WARN,          \
                                      "[%s:%d] " fmt, __func__,                \
                                      __LINE__, ##__VA_ARGS__)

#define SDK_TRACE_INFO(fmt, ...)  sdk::lib::logger::trace_cb()(                \
                                      sdk::lib::SDK_TRACE_LEVEL_INFO,          \
                                      "[%s:%d] " fmt, __func__,                \
                                      __LINE__, ##__VA_ARGS__)

#define SDK_TRACE_DEBUG(fmt, ...) sdk::lib::logger::trace_cb()(                \
                                      sdk::lib::SDK_TRACE_LEVEL_DEBUG,         \
                                      "[%s:%d] " fmt, __func__,                \
                                      __LINE__, ##__VA_ARGS__)

#define SDK_TRACE_VERBOSE(fmt, ...) sdk::lib::logger::trace_cb()(              \
                                        sdk::lib::SDK_TRACE_LEVEL_VERBOSE,     \
                                        "[%s:%d] " fmt, __func__, __LINE__,    \
                                        ##__VA_ARGS__)

#define SDK_TRACE_PRINT             SDK_TRACE_DEBUG

#endif    // __SDK_LOGGER_HPP__

