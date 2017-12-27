// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __APP_REDIR_H__
#define __APP_REDIR_H__

#include <stdint.h>
#include "nic/include/fte.hpp"
#include "app_redir_headers.hpp"
#include "app_redir_ctx.hpp"

namespace hal {
namespace app_redir {

void app_redir_pkt_verdict_set(fte::ctx_t& ctx,
                               app_redir_verdict_t verdict);
hal_ret_t app_redir_policy_applic_set(fte::ctx_t& ctx);
uint8_t *app_redir_pkt(fte::ctx_t& ctx);
size_t app_redir_pkt_len(fte::ctx_t& ctx);
hal_ret_t appid_cleanup_flow(appid_info_t& appid_info);


} // namespace app_redir 
} // namespace hal

#endif  // __APP_REDIR_H__
