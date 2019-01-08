//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "proxy_plugin.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/hal/lkl/lkl_api.hpp"

namespace hal {
namespace proxy {

const std::string FTE_FEATURE_TCP_PROXY("pensando.io/proxy:tcp");
const std::string FTE_FEATURE_TCP_PROXY_TRIGGER_CONNECTION("pensando.io/proxy:tcp-trigger-connection");
const std::string FTE_FEATURE_TLS_PROXY("pensando.io/proxy:tls");
const std::string FTE_FEATURE_IPSEC("pensando.io/proxy:ipsec");
const std::string FTE_FEATURE_QUIESCE("pensando.io/proxy:quiesce");
const std::string FTE_FEATURE_P4PT("pensando.io/proxy:app-p4pt");

extern "C" hal_ret_t proxy_plugin_init(hal_cfg_t *hal_cfg) {
    fte::register_feature(FTE_FEATURE_TCP_PROXY, tcp_exec);
    fte::register_feature(FTE_FEATURE_TCP_PROXY_TRIGGER_CONNECTION, tcp_exec_trigger_connection);
    fte::register_feature(FTE_FEATURE_QUIESCE, quiesce_exec);
    fte::register_feature(FTE_FEATURE_TLS_PROXY, tls_exec);
    fte::feature_info_t ipsec_info = {
        state_size: sizeof(ipsec_info_t),
    };

    fte::register_feature(FTE_FEATURE_IPSEC, ipsec_exec, ipsec_info);
    fte::register_feature(FTE_FEATURE_P4PT, p4pt_exec);

    /*
     * Initialize the LKL for user-space TCP stack for TCP-proxy feature.
     */
    if (hal::g_hal_cfg.features == hal::HAL_FEATURE_SET_IRIS) {
        HAL_TRACE_DEBUG("lkl init");
        if (hal::pd::lkl_init() != HAL_RET_OK) {
            fprintf(stderr, "LKL initialization failed, quitting ...\n");
            return HAL_RET_ERR;
        }
    }

    return HAL_RET_OK;
}

extern "C" void proxy_exit() {
}

}
}
