#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace classic {

void init();

fte::pipeline_action_t fwding_exec(fte::ctx_t &ctx);

} //namespace classic
} // namedspace hal
