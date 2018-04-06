//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// logger library for SDK
//------------------------------------------------------------------------------

#ifndef __SDK_LOGGER_HPP__
#define __SDK_LOGGER_HPP__

namespace sdk {
namespace lib {

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class logger {
public:
    typedef int (*logger_cb_t)(const char *format, ...);
    static void init(logger::logger_cb_t error_log_func,
                     logger::logger_cb_t debug_log_func);
    static logger_cb_t error_log(void) { return log_error_cb_ ?
                                                    log_error_cb_ : printf; }
    static logger_cb_t debug_log(void) { return log_debug_cb_ ?
                                                    log_debug_cb_ : printf; }

private:
    static logger_cb_t    log_error_cb_;
    static logger_cb_t    log_debug_cb_;
};


}    // namespace lib
}    // namespace sdk

#define SDK_TRACE_ERR(fmt, ...)    sdk::lib::logger::error_log()  \
                          ("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define SDK_TRACE_DEBUG(fmt, ...)  sdk::lib::logger::debug_log()  \
                          ("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#endif  // __SDK_LOGGER_HPP__

