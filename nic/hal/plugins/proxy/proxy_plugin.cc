//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "proxy_plugin.hpp"

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
    fte::register_feature(FTE_FEATURE_IPSEC, ipsec_exec);
    fte::register_feature(FTE_FEATURE_P4PT, p4pt_exec);
    return HAL_RET_OK;
}

extern "C" void proxy_exit() {
}

}
}
