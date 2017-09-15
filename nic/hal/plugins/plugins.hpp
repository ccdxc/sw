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
    // outbound features - applied to local host intiated pkts:
    //   - iflow pkts of local host intiated sessions and
    //   - rflow pkts of remote host initiated sessions
    fte::feature_id_t flow_miss_outbound[] = {
        fte::FTE_FEATURE_DFW,
        fte::FTE_FEATURE_TCP_PROXY,
        fte::FTE_FEATURE_STAGE_MARKER,
        fte::FTE_FEATURE_LB,
        fte::FTE_FEATURE_FWDING,
        fte::FTE_FEATURE_TUNNEL,
    };

    // inbound features - applied to pkts destined to local host 
    //   - iflow pkts of remote host intiated sessions and
    //   - rflow pkts of local host initiated sessions
    fte::feature_id_t flow_miss_inbound[] = {
        fte::FTE_FEATURE_LB,
        fte::FTE_FEATURE_TCP_PROXY,
        fte::FTE_FEATURE_STAGE_MARKER,
        fte::FTE_FEATURE_DFW,
        fte::FTE_FEATURE_FWDING,
        fte::FTE_FEATURE_TUNNEL,
    };

    register_pipeline("flow-miss", fte::FLOW_MISS_LIFQ,
                      flow_miss_outbound, sizeof(flow_miss_outbound)/sizeof(fte::feature_id_t),
                      flow_miss_inbound, sizeof(flow_miss_inbound)/sizeof(fte::feature_id_t));

    return HAL_RET_OK;
}

} // namespace hal
