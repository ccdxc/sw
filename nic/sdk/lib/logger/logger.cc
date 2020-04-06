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

void
logger::init(sdk::lib::logger::trace_cb_t trace_cb)
{
    trace_cb_ = trace_cb;
}

}    // namespace lib
}    // namespace sdk

