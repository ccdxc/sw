#pragma once
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/nwsec.hpp"

using session::FlowAction;
using nwsec::ALGName;
namespace fte {
    
typedef struct net_dfw_match_result_s {
    bool            valid;
    FlowAction      action;
    ALGName         alg;
    bool            log;
} net_dfw_match_result_t;

}
