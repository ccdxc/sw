//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// logger library for SDK
//------------------------------------------------------------------------------

#include <stdio.h>
#include "lib/logger/logger.hpp"

namespace sdk {
namespace lib {

logger::logger_cb_t logger::log_error_cb_ = printf;
logger::logger_cb_t logger::log_debug_cb_ = printf;

void
logger::init(sdk::lib::logger::logger_cb_t error_log_func,
             sdk::lib::logger::logger_cb_t debug_log_func)
{
    log_error_cb_ = error_log_func;
    log_debug_cb_ = debug_log_func;
}

}    // namespace lib
}    // namespace sdk

