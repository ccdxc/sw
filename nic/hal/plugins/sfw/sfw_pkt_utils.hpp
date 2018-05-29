//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

namespace hal {
namespace plugins {
namespace sfw {

uint32_t net_sfw_build_tcp_rst(fte::ctx_t& ctx, uint8_t **pkt,
                               const fte::header_rewrite_info_t rewrite_info,
                               const fte::header_push_info_t push_info);

uint32_t net_sfw_build_tcp_pkt(fte::ctx_t& ctx, uint8_t *pkt, uint32_t len,
                               const fte::header_rewrite_info_t rewrite_info,
                               const fte::header_push_info_t push_info);

uint32_t net_sfw_build_udp_pkt(fte::ctx_t& ctx, uint8_t *pkt, uint32_t len,
                               const fte::header_rewrite_info_t rewrite_info,
                               const fte::header_push_info_t push_info);

uint32_t net_sfw_build_icmp_error(fte::ctx_t& ctx, uint8_t **pkt,
                                  const fte::header_rewrite_info_t rewrite_info,
                                  const fte::header_push_info_t push_info);

void net_sfw_free_reject_pkt(uint8_t *pkt);

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
