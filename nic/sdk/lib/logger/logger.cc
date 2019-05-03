//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// logger library for SDK
//------------------------------------------------------------------------------

#include <stdio.h>
#include "lib/logger/logger.hpp"

namespace sdk {
namespace lib {

logger::trace_cb_t logger::trace_cb_ = NULL;
logger::trace_cb_t logger::obfl_trace_cb_ = NULL;

void
logger::init(sdk::lib::logger::trace_cb_t trace_cb, sdk::lib::logger::trace_cb_t obfl_trace_cb)
{
    trace_cb_ = trace_cb;
    obfl_trace_cb = obfl_trace_cb;
}

}    // namespace lib
}    // namespace sdk

