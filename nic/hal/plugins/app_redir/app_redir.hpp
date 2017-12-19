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
hal_ret_t appid_cleanup_flow(hal::appid_info_t& appid_info);

/* 
 * From network, packet will go to TCP, TLS (for decryption),
 * then *redirected* to app redir.
 * 
 * From host, packets will go to TCP then *redirected* to app redir.
 *
 * Currently TCP/TLS proxy does not support host-to-host and
 * network-to-network.
 */
static inline uint32_t
app_redir_proxyr_oper_cb_id_eval(const flow_key_t& flow_key,
                                 uint32_t cb_id)
{
    cb_id &= PROXYRCB_NUM_ENTRIES_MASK;
    return flow_key.dir == FLOW_DIR_FROM_UPLINK ?
           PROXYR_OPER_CB_ID(PROXYR_TLS_PROXY_DIR, cb_id) : 
           PROXYR_OPER_CB_ID(PROXYR_TCP_PROXY_DIR, cb_id);
}


/* 
 * From network, packet will go to TCP, TLS (for decryption)
 * app redir, then *chain* to TCP
 * 
 * From host, packets will go to TCP, app redir, then *chain* to
 * TLS (for encryption)
 *
 * Currently TCP/TLS proxy does not support host-to-host and
 * network-to-network.
 */
static inline uint16_t
app_redir_proxyc_chain_lif_eval(const flow_key_t& flow_key)
{
    return flow_key.dir == FLOW_DIR_FROM_UPLINK ?
           SERVICE_LIF_TCP_PROXY : SERVICE_LIF_TLS_PROXY;
}

static inline uint32_t
app_redir_proxyc_oper_cb_id_eval(const flow_key_t& flow_key,
                                 uint32_t cb_id)
{
    cb_id &= PROXYCCB_NUM_ENTRIES_MASK;
    return flow_key.dir == FLOW_DIR_FROM_UPLINK ?
           PROXYC_OPER_CB_ID(PROXYC_TCP_PROXY_DIR, cb_id) : 
           PROXYC_OPER_CB_ID(PROXYC_TLS_PROXY_DIR, cb_id);
}


} // namespace app_redir 
} // namespace hal

#endif  // __APP_REDIR_H__
