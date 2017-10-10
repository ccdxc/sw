#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace proxy {

fte::pipeline_action_t app_redir_miss_exec(fte::ctx_t &ctx);
fte::pipeline_action_t app_redir_exec(fte::ctx_t &ctx);

} // namespace proxy
} // namespace hal

