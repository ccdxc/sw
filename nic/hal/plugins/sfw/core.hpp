//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"

using session::FlowAction;
using nwsec::ALGName;
using nwsec::SecurityAction;

namespace hal {
namespace plugins {
namespace sfw {

const std::string FTE_FEATURE_SFW("pensando.io/sfw:sfw");
const std::string FTE_FEATURE_CONNTRACK("pensando.io/sfw:conntrack");

// Stateful firewall state
typedef struct sfw_info_s {
    ALGName         alg_proto;
    bool            skip_sfw;
    bool            sfw_done;
} sfw_info_t;
std::ostream& operator<<(std::ostream& os, const sfw_info_t& val);

typedef struct net_sfw_match_result_s {
    bool            valid;
    FlowAction      action;
    ALGName         alg;
    bool            log;
    SecurityAction  sfw_action;
} net_sfw_match_result_t;
std::ostream& operator<<(std::ostream& os, const net_sfw_match_result_t& val);

uint32_t
net_sfw_build_tcp_rst (fte::ctx_t& ctx, uint8_t **pkt,
                       const fte::header_rewrite_info_t rewrite_info,
                       const fte::header_push_info_t push_info);

uint32_t
net_sfw_build_icmp_error (fte::ctx_t& ctx, uint8_t **pkt,
                          const fte::header_rewrite_info_t rewrite_info,
                          const fte::header_push_info_t push_info);

void
net_sfw_free_reject_pkt (uint8_t *pkt);

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
