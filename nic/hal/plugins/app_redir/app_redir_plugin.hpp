// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __APP_REDIR_PLUGIN_H__
#define __APP_REDIR_PLUGIN_H__

#include "nic/include/fte.hpp"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"

namespace hal {
namespace app_redir {

void init();
void destroy();
void thread_init();
void thread_destroy();

fte::pipeline_action_t app_redir_miss_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec_fini(fte::ctx_t& ctx);
fte::pipeline_action_t stage_exec(fte::ctx_t &ctx);
fte::pipeline_action_t appid_exec(fte::ctx_t &ctx);

}  // namespace app_redir
}  // namespace hal

#endif  // __APP_REDIR_PLUGIN_H__
