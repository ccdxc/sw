#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace proxy {

void init();

fte::pipeline_action_t tcp_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tls_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ipsec_exec(fte::ctx_t &ctx);
void tcp_transmit_pkt(unsigned char* pkt, unsigned int len, bool is_connect_req, uint16_t dst_lif);
hal_ret_t tcp_create_cb(qid_t qid);
void tcp_update_cb(void *tcpcb, uint32_t qid);
void tcp_ring_doorbell(uint32_t qid);
} // namespace proxy
} // namespace hal

