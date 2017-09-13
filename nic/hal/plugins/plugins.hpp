#pragma once

#include <base.h>
#include <fte.hpp>
#include "network/net_plugin.hpp"
#include "proxy/proxy_plugin.hpp"

namespace hal {

inline hal_ret_t init_plugins() {
    hal::net::init();
    hal::proxy::init();

    // register pipelines
    // flow-miss pipeline
    fte::feature_id_t flow_miss_features[] = {
        fte::FTE_FEATURE_DFW,
        fte::FTE_FEATURE_LB,
        fte::FTE_FEATURE_FWDING,
        fte::FTE_FEATURE_TCP_PROXY,
    };

    register_pipeline("flow-miss", fte::FLOW_MISS_LIFQ,
                      flow_miss_features, sizeof(flow_miss_features)/sizeof(fte::feature_id_t));

    return HAL_RET_OK;
}

} // namespace hal
