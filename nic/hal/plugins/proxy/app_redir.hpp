#pragma once

#include "nic/include/fte.hpp"
#include "app_redir_headers.hpp"

namespace hal {
namespace proxy {


/*
 * Application redirect context
 */
class app_redir_ctx_t {

public:
    app_redir_ctx_t(fte::ctx_t& ctx) : fte_ctx(ctx)
    {
        flags = 0;
        hdr_len_total = 0;
        chain_qtype = 0;
        chain_wring_type = types::WRING_TYPE_NONE;
    };

    fte::ctx_t&         fte_ctx;
    uint16_t            flags;
    uint16_t            hdr_len_total;
    types::WRingType    chain_wring_type;
    uint8_t             chain_qtype;
};


fte::pipeline_action_t app_redir_miss_exec(fte::ctx_t &ctx);
fte::pipeline_action_t app_redir_exec(fte::ctx_t &ctx);

hal_ret_t app_redir_pkt_send(app_redir_ctx_t *app_ctx);

} // namespace proxy 
} // namespace hal

