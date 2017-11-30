#pragma once
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/nwsec.hpp"

using session::FlowAction;
using nwsec::ALGName;
namespace hal {
namespace firewall {

const std::string FTE_FEATURE_FIREWALL("pensando.io/firewall:firewall");

// firewall state 
typedef struct firewall_info_s {
    ALGName         alg_proto;
    bool            skip_firewall;
} firewall_info_t;
    
typedef struct net_dfw_match_result_s {
    bool            valid;
    FlowAction      action;
    ALGName         alg;
    bool            log;
} net_dfw_match_result_t;

}
}
