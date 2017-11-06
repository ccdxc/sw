#pragma once

#include "nic/include/fte.hpp"
#include "nic/include/fte_ctx.hpp"
#include "app_redir_headers.hpp"
#include "app_redir_cb_ops.hpp"

namespace hal {
namespace proxy {

fte::pipeline_action_t app_redir_miss_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec(fte::ctx_t& ctx);
fte::pipeline_action_t app_redir_exec_fini(fte::ctx_t& ctx);
void app_redir_pkt_verdict_set(fte::ctx_t& ctx,
                               fte::app_redir_verdict_t verdict);
hal_ret_t app_redir_policy_applic_set(fte::ctx_t& ctx);

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


} // namespace proxy 
} // namespace hal

