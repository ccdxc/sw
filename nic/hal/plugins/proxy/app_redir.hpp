#pragma once

#include "nic/include/fte.hpp"
#include "app_redir_headers.hpp"

namespace hal {
namespace proxy {

fte::pipeline_action_t app_redir_miss_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec(fte::ctx_t& ctx);

hal_ret_t app_redir_pkt_send(fte::ctx_t& ctx);

} // namespace proxy 
} // namespace hal

