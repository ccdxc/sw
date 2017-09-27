#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace proxy {

void init();

fte::pipeline_action_t tcp_exec(fte::ctx_t &ctx);
} // namespace proxy
} // namespace hal

