// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __APP_REDIR_H__
#define __APP_REDIR_H__

#include "nic/include/fte.hpp"
#include "app_redir_headers.hpp"

namespace hal {
namespace app_redir {

void app_redir_pkt_verdict_set(fte::ctx_t& ctx,
                               fte::app_redir_verdict_t verdict);
hal_ret_t app_redir_policy_applic_set(fte::ctx_t& ctx);
hal_ret_t appid_cleanup_flow(fte::appid_info_t& appid_info);

/*
 * Return pointer to current packet, taking into account of inserted
 * app redirect headers in the flow miss case.
 */
static inline uint8_t *
app_redir_pkt(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

    return redir_ctx.redir_miss_pkt_p() ?
           (uint8_t *)&redir_ctx.redir_miss_hdr() : ctx.pkt();
}

/*
 * Return length of current packet, taking into account of inserted
 * app redirect headers in the flow miss case.
 */
static inline size_t
app_redir_pkt_len(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

    return redir_ctx.redir_miss_pkt_p() ?
           ctx.pkt_len() + redir_ctx.hdr_len_total() : ctx.pkt_len();
}


} // namespace app_redir 
} // namespace hal

#endif  // __APP_REDIR_H__
