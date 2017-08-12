#pragma once

#include <base.h>
#include <fte.hpp>
#include "network/net_plugin.hpp"

namespace hal {

const uint16_t ARM_LIF = 1;
const uint32_t FLOW_MISS_LIFQ = {ARM_LIF, 0, 0};

inline hal_ret_t init_plugins() {
    hal::net::init();

    // register pipelines
    // flow-miss pipeline
    feature_id_t flow_miss_features[] = {
        fte::FTE_FEATURE_FWDING,
        fte::FTE_FEATURE_DFW,
        fte::FTE_FEATURE_LB,
    };

    register_pipeline("flow-miss", FLOW_MISS_LIFQ,
                      flow_miss_features, sizeof(flow_miss_features));
}

} // namespace hal
