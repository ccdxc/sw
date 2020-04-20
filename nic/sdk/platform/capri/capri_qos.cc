// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "capri_qos.hpp"
#include "capri_tm_rw.hpp"
#include "capri_p4.hpp"

namespace sdk {
namespace platform {
namespace capri {

// qos related asic APIs
tm_q_t
capri_qos_span_queue (void)
{
    return CAPRI_TM_P4_SPAN_QUEUE;
}

tm_q_t
capri_qos_p4eg_uplink_span_q_replacement (void)
{
    return CAPRI_TM_P4_EG_UPLINK_SPAN_QUEUE_REPLACEMENT;
}

tm_q_t
capri_qos_cpu_copy_queue (void)
{
    return CAPRI_TM_P4_CPU_COPY_QUEUE;
}

tm_q_t
capri_qos_p4eg_uplink_cpu_copy_q_replacement (void)
{
    return CAPRI_TM_P4_EG_UPLINK_CPU_COPY_QUEUE_REPLACEMENT;
}

tm_q_t
capri_qos_max_tx_qs_per_class (void)
{
    return CAPRI_QOS_MAX_TX_QUEUES_PER_CLASS;
}

tm_q_t
capri_qos_p4ig_uplink_iq_offset (void)
{
    return CAPRI_TM_P4_UPLINK_IQ_OFFSET;
}

tm_q_t
capri_qos_rxdma_oq_offset (void)
{
    return CAPRI_TM_RXDMA_OQ_OFFSET;
}

uint32_t
capri_qos_default_xon_threshold (void)
{
    return CAPRI_TM_DEFAULT_XON_THRESHOLD;
}

uint32_t
capri_qos_default_xoff_threshold (void)
{
    return CAPRI_TM_DEFAULT_XOFF_THRESHOLD;
}

tm_port_t
capri_qos_uplink_port_start (void)
{
    return CAPRI_TM_UPLINK_PORT_BEGIN;
}

tm_port_t
capri_qos_uplink_port_end (void)
{
    return CAPRI_TM_UPLINK_PORT_END;
}

uint32_t
capri_qos_max_txdma_iqs (void)
{
    return CAPRI_QOS_MAX_TXDMA_IQS;
}

uint32_t
capri_qos_max_uplink_iqs (void)
{
    return CAPRI_QOS_MAX_UPLINK_IQS;
}

uint32_t
capri_qos_max_common_oqs (void)
{
    return CAPRI_QOS_MAX_COMMON_OQS;
}

uint32_t
capri_qos_max_rxdma_oqs (void)
{
    return CAPRI_QOS_MAX_RXDMA_OQS;
}

sdk_ret_t
capri_qos_uplink_q_params_update (tm_port_t port,
                                  tm_uplink_q_params_t *q_params)
{
    sdk_ret_t ret;
    tm_q_t iq = q_params->iq;
    bool no_drop = q_params->no_drop;
    uint32_t dot1q_pcp = q_params->dot1q_pcp;

    // update no_drop for iq
    ret = capri_tm_uplink_iq_no_drop_update(port, iq, no_drop);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming iq no_drop for port %u, iq %u, "
                      "no_drop %u, ret %u",
                      port, iq, no_drop, ret);
        return ret;
    }

    // update iq params
    ret = capri_tm_uplink_q_params_update(port, q_params);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming iq params for port %u, iq %u, "
                      "ret %u", port, iq, ret);
        return ret;
    }

    ret = capri_tm_uplink_input_map_update(port, dot1q_pcp, iq);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming uplink map for port %u, "
                      "dot1q_pcp %u, iq %u, ret %u",
                      port, dot1q_pcp, iq, ret);
    }

    if (q_params->use_ip) {
        ret = capri_tm_uplink_input_dscp_map_update(port, q_params->dot1q_pcp,
                                                    q_params->ip_dscp);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Error programming uplink dscp map for port %u, "
                          "ret %u", port, ret);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
capri_qos_uplink_iq_nodrop_update (tm_port_t port, tm_q_t iq,
                                   bool no_drop)
{
    return capri_tm_uplink_iq_no_drop_update(port, iq, no_drop);
}

sdk_ret_t
capri_qos_uplink_input_map_update (tm_port_t port, uint32_t dot1q_pcp,
                                   tm_q_t iq)
{
    return capri_tm_uplink_input_map_update(port, dot1q_pcp, iq);
}

sdk_ret_t
capri_qos_uplink_input_dscp_map_update (tm_port_t port, uint32_t dot1q_pcp,
                                        bool *ip_dscp)
{
    return capri_tm_uplink_input_dscp_map_update(port, dot1q_pcp, ip_dscp);
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
