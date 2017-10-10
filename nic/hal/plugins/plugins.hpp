#pragma once

#include "nic/include/base.h"
#include "nic/include/fte.hpp"
#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"

namespace hal {

inline hal_ret_t init_plugins() {
    fte::init();
    hal::net::init();
    hal::proxy::init();

    // register pipelines
    // flow-miss pipeline
    // outbound features - applied to local host intiated pkts:
    //   - iflow pkts of local host intiated sessions and
    //   - rflow pkts of remote host initiated sessions
    fte::feature_id_t flow_miss_outbound[] = {
        fte::FTE_FEATURE_DOL_TEST_ONLY,
        fte::FTE_FEATURE_DFW,
        fte::FTE_FEATURE_ALG,
        fte::FTE_FEATURE_TCP_PROXY,
        fte::FTE_FEATURE_IPSEC,
        fte::FTE_FEATURE_STAGE_MARKER,
        fte::FTE_FEATURE_LB,
        fte::FTE_FEATURE_QOS,
        fte::FTE_FEATURE_FWDING,
        fte::FTE_FEATURE_TUNNEL,
        fte::FTE_FEATURE_APP_REDIR_MISS,
    };

    // inbound features - applied to pkts destined to local host 
    //   - iflow pkts of remote host intiated sessions and
    //   - rflow pkts of local host initiated sessions
    fte::feature_id_t flow_miss_inbound[] = {
        fte::FTE_FEATURE_LB,
        fte::FTE_FEATURE_TCP_PROXY,
        fte::FTE_FEATURE_IPSEC,
        fte::FTE_FEATURE_STAGE_MARKER,
        fte::FTE_FEATURE_DFW,
        fte::FTE_FEATURE_ALG,
        fte::FTE_FEATURE_QOS,
        fte::FTE_FEATURE_FWDING,
        fte::FTE_FEATURE_TUNNEL,
        fte::FTE_FEATURE_APP_REDIR_MISS,
    };

    register_pipeline("flow-miss", fte::FLOW_MISS_LIFQ,
                      flow_miss_outbound, sizeof(flow_miss_outbound)/sizeof(fte::feature_id_t),
                      flow_miss_inbound, sizeof(flow_miss_inbound)/sizeof(fte::feature_id_t));
    // TCP Proxy Pipeline
    fte::feature_id_t tcp_proxy_outbound[] = {
    	fte::FTE_FEATURE_TCP_PROXY,
    };

    register_pipeline("tcp-proxy", fte::TCP_PROXY_LIFQ,
    				  tcp_proxy_outbound, sizeof(tcp_proxy_outbound)/sizeof(fte::feature_id_t),
					  {}, 0, {0x7FF, 0, 0});

    // TLS Proxy Pipeline
    fte::feature_id_t tls_proxy_outbound[] = {
    	fte::FTE_FEATURE_TLS_PROXY,
    };

    register_pipeline("tls-proxy", fte::TLS_PROXY_LIFQ,
        				  tls_proxy_outbound, sizeof(tls_proxy_outbound)/sizeof(fte::feature_id_t),
						  {}, 0, {0x7FF, 0, 0});

    // ALG Control flow monitor Pipeline
    fte::feature_id_t alg_cflow_outbound[] = {
        fte::FTE_FEATURE_ALG,
    };

    register_pipeline("alg-cflow", fte::ALG_CFLOW_LIFQ,
                                    alg_cflow_outbound, sizeof(alg_cflow_outbound)/sizeof(fte::feature_id_t),
                                    {}, 0, {0x7FF, 0, 0}); 
    // L7 App Redirect Pipeline
    fte::feature_id_t app_redir_inbound[] = {
    	fte::FTE_FEATURE_APP_REDIR,
    };

    fte::feature_id_t app_redir_outbound[] = {
    	fte::FTE_FEATURE_APP_REDIR,
    };

    register_pipeline("app-redir", fte::APP_REDIR_LIFQ,
                      app_redir_outbound, sizeof(app_redir_outbound)/sizeof(fte::feature_id_t),
                      app_redir_inbound, sizeof(app_redir_inbound)/sizeof(fte::feature_id_t),
                      {0x7FF, 0, 0});

    return HAL_RET_OK;
}

} // namespace hal
