#pragma once

#include "nic/include/fte.hpp"
#include "app_redir_headers.hpp"

namespace hal {
namespace proxy {

fte::pipeline_action_t app_redir_miss_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec_fini(fte::ctx_t& ctx);
void app_redir_pkt_verdict_set(fte::ctx_t& ctx,
                               fte::app_redir_verdict_t verdict);

} // namespace proxy 
} // namespace hal

