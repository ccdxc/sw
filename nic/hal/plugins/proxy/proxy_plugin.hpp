#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace proxy {

void init();

fte::pipeline_action_t tcp_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tls_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ipsec_exec(fte::ctx_t &ctx);
void tcp_transmit_pkt(unsigned char* pkt, unsigned int len, bool is_connect_req, uint16_t dst_lif, uint16_t src_lif, hal::flow_direction_t dir, uint16_t hw_vlan_id);
hal_ret_t
tcp_create_cb(qid_t qid, uint16_t src_lif, ether_header_t *eth, vlan_header_t* vlan, ipv4_header_t *ip, tcp_header_t *tcp, bool is_itor_dir);

void tcp_update_cb(void *tcpcb, uint32_t qid, uint16_t src_lif);
hal_ret_t tcp_trigger_ack_send(uint32_t qid, tcp_header_t *tcp);
} // namespace proxy
} // namespace hal

