//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/network/net_plugin.hpp"

namespace hal {
namespace plugins {
namespace network {

//------------------------------------------------------------------------------
// Network plugin
//------------------------------------------------------------------------------
const std::string FTE_FEATURE_QOS("pensando.io/network:qos");
const std::string FTE_FEATURE_INGRESS_CHECKS("pensando.io/network:ingress-checks");

extern "C" hal_ret_t network_init(hal_cfg_t *hal_cfg) {
    // Register update for ingress checks
    fte::feature_info_t ingress_info = {
        state_size:  0,
        state_init_fn: NULL,
        sess_del_cb: NULL,
        sess_get_cb: NULL,
    };

    fte::register_feature(FTE_FEATURE_QOS,  qos_exec);
    fte::register_feature(FTE_FEATURE_INGRESS_CHECKS,  ingress_checks_exec, ingress_info);
    return HAL_RET_OK;
}

extern "C" void network_exit() {
}


}
}
}
