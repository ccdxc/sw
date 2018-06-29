//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/src/firewall/nwsec_group.hpp"

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
    hal::rule_key_t rule_id;
} net_sfw_match_result_t;
std::ostream& operator<<(std::ostream& os, const net_sfw_match_result_t& val);

static inline sfw_info_t* sfw_feature_state(fte::ctx_t &ctx) {
    // cache the feature id for faster lookups
    static uint16_t fid = fte::feature_id(FTE_FEATURE_SFW.c_str());

    return (sfw_info_t*)ctx.feature_state(fid);
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
