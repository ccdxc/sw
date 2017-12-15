#pragma once
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/nwsec.hpp"

using session::FlowAction;
using nwsec::ALGName;

namespace hal {
namespace plugins {
namespace sfw {

const std::string FTE_FEATURE_SFW("pensando.io/sfw:sfw");

// Stateful firewall state 
typedef struct sfw_info_s {
    ALGName         alg_proto;
    bool            skip_sfw;
    bool            sfw_done;
} sfw_info_t;
    
typedef struct net_sfw_match_result_s {
    bool            valid;
    FlowAction      action;
    ALGName         alg;
    bool            log;
} net_sfw_match_result_t;

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
