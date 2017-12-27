#pragma once

#include "nic/hal/src/rawrcb.hpp"
#include "nic/hal/src/rawccb.hpp"
#include "nic/hal/src/proxyrcb.hpp"
#include "nic/hal/src/proxyccb.hpp"

namespace hal {
namespace app_redir {

hal_ret_t app_redir_rawrcb_create(const rawrcb_t& rawrcb);
hal_ret_t app_redir_rawrcb_update(const rawrcb_t& rawrcb);
static inline void
app_redir_rawrcb_init(uint32_t cb_id,
                      rawrcb_t& rawrcb)
{
    memset(&rawrcb, 0, sizeof(rawrcb));
    rawrcb.cb_id = cb_id;
}

hal_ret_t app_redir_rawccb_create(const rawccb_t& rawccb);
hal_ret_t app_redir_rawccb_update(const rawccb_t& rawccb);
static inline void
app_redir_rawccb_init(uint32_t cb_id,
                      rawccb_t& rawccb)
{
    memset(&rawccb, 0, sizeof(rawccb));
    rawccb.cb_id = cb_id;
}

hal_ret_t app_redir_proxyrcb_create(const proxyrcb_t& proxyrcb);
hal_ret_t app_redir_proxyrcb_update(const proxyrcb_t& proxyrcb);
hal_ret_t app_redir_proxyrcb_flow_key_build(proxyrcb_t& proxyrcb,
                                            const flow_key_t& flow_key);
static inline void
app_redir_proxyrcb_init(uint32_t cb_id,
                        proxyrcb_t& proxyrcb)
{
    memset(&proxyrcb, 0, sizeof(proxyrcb));
    proxyrcb.cb_id = cb_id;
}

hal_ret_t app_redir_proxyccb_create(const proxyccb_t& proxyccb);
hal_ret_t app_redir_proxyccb_update(const proxyccb_t& proxyccb);
hal_ret_t app_redir_proxyccb_chain_txq_build(proxyccb_t& proxyccb,
                                             uint16_t chain_txq_lif,
                                             uint8_t chain_txq_qtype,
                                             uint32_t chain_txq_qid,
                                             uint8_t chain_txq_ring);
static inline void
app_redir_proxyccb_init(uint32_t cb_id,
                        proxyccb_t& proxyccb)
{
    memset(&proxyccb, 0, sizeof(proxyccb));
    proxyccb.cb_id = cb_id;
}


} // namespace app_redir
} // namespace hal

