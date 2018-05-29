//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace proxy {

extern "C" hal_ret_t proxy_plugin_init(hal_cfg_t *hal_cfg);

fte::pipeline_action_t tcp_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tcp_exec_trigger_connection(fte::ctx_t &ctx);
fte::pipeline_action_t quiesce_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tls_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ipsec_exec(fte::ctx_t &ctx);
fte::pipeline_action_t p4pt_exec(fte::ctx_t &ctx);
void tcp_transmit_pkt(unsigned char* pkt, unsigned int len, uint16_t src_lif, uint16_t src_vlan_id);

hal_ret_t
tcp_create_cb(qid_t qid, qid_t other_qid, uint16_t src_lif, uint16_t src_vlan_id,
              ether_header_t *eth, vlan_header_t* vlan,
              ipv4_header_t *ip, tcp_header_t *tcp,
              bool is_itor_dir, types::AppRedirType l7_proxy_type);

hal_ret_t
tcp_create_cb_v6(qid_t qid, qid_t other_qid, uint16_t src_lif, uint16_t src_vlan_id, ether_header_t *eth, vlan_header_t* vlan, ipv6_header_t *ip, tcp_header_t *tcp, bool is_itor_dir, types::AppRedirType l7_proxy_type);

void tcp_update_cb(void *tcpcb, uint32_t qid, uint16_t src_lif);
hal_ret_t tcp_trigger_ack_send(uint32_t qid, tcp_header_t *tcp);
void tcp_ring_doorbell(uint32_t qid);

void quiesce_message_tx(void);
hal_ret_t tls_poll_asym_pend_req_q(void);

} // namespace proxy
} // namespace hal

