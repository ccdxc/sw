#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace proxy {

extern "C" hal_ret_t proxy_plugin_init();

fte::pipeline_action_t tcp_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tcp_exec_trigger_connection(fte::ctx_t &ctx);
fte::pipeline_action_t quiesce_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tls_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ipsec_exec(fte::ctx_t &ctx);
fte::pipeline_action_t p4pt_exec(fte::ctx_t &ctx);
void tcp_transmit_pkt(unsigned char* pkt, unsigned int len, bool is_connect_req, uint16_t dst_lif, uint16_t src_lif, hal::flow_direction_t dir, uint16_t hw_vlan_id);
hal_ret_t
tcp_create_cb(qid_t qid, uint16_t src_lif, ether_header_t *eth, vlan_header_t* vlan, ipv4_header_t *ip, tcp_header_t *tcp, bool is_itor_dir, uint16_t hw_vlan_id,
              types::AppRedirType l7_proxy_type);

void tcp_update_cb(void *tcpcb, uint32_t qid, uint16_t src_lif);
hal_ret_t tcp_trigger_ack_send(uint32_t qid, tcp_header_t *tcp);
void tcp_ring_doorbell(uint32_t qid);

void quiesce_message_tx(void);
} // namespace proxy
} // namespace hal

