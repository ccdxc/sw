// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __SDK_CAPRI_QOS_HPP__
#define __SDK_CAPRI_QOS_HPP__

#include "include/sdk/base.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "include/sdk/qos.hpp"

// -------------------------------------
// reserved queues
// -------------------------------------
// Using a non-zero value for admic cos so that any uninitialized
// rings do not use this 
#define CAPRI_QOS_ADMIN_COS_Q              1

#define CAPRI_QOS_MAX_P4_IQS               32
#define CAPRI_QOS_MAX_P4_OQS               CAPRI_QOS_MAX_P4_IQS

#define CAPRI_QOS_MAX_UPLINK_IQS           8
#define CAPRI_QOS_MAX_UPLINK_OQS           CAPRI_QOS_MAX_UPLINK_IQS

#define CAPRI_QOS_MAX_TXDMA_IQS            16
#define CAPRI_QOS_MAX_RXDMA_OQS            16
#define CAPRI_QOS_MAX_COMMON_OQS           16

#define CAPRI_QOS_MAX_TX_QUEUES_PER_CLASS  2

namespace sdk {
namespace platform {
namespace capri {

typedef struct qos_queues_s {
    tm_q_t         uplink_iq;
    tm_q_t         txdma_iq[CAPRI_QOS_MAX_TX_QUEUES_PER_CLASS];
    tm_q_t         p4ig_iq[sdk::qos::QOS_NUM_IQ_TYPES];
    tm_q_t         p4eg_iq[sdk::qos::QOS_NUM_IQ_TYPES];
    tm_q_t         dest_oq;
    bool           pcie_oq;
    sdk::qos::qos_oq_type_e  dest_oq_type;
} qos_queues_t;

tm_q_t capri_qos_span_queue(void);
tm_q_t capri_qos_p4eg_uplink_span_q_replacement(void);
tm_q_t capri_qos_cpu_copy_queue(void);
tm_q_t capri_qos_p4eg_uplink_cpu_copy_q_replacement(void);
tm_q_t capri_qos_max_tx_qs_per_class(void);
tm_q_t capri_qos_p4ig_uplink_iq_offset(void);
tm_q_t capri_qos_rxdma_oq_offset(void);
uint32_t capri_qos_default_xon_threshold(void);
uint32_t capri_qos_default_xoff_threshold(void);
tm_port_t capri_qos_uplink_port_start(void);
tm_port_t capri_qos_uplink_port_end(void);
uint32_t capri_qos_max_txdma_iqs(void);
uint32_t capri_qos_max_uplink_iqs(void);
uint32_t capri_qos_max_common_oqs(void);
uint32_t capri_qos_max_rxdma_oqs(void);
sdk_ret_t capri_qos_uplink_q_params_update(tm_port_t port,
                                           tm_uplink_q_params_t *q_params);
sdk_ret_t capri_qos_uplink_iq_nodrop_update(tm_port_t port, tm_q_t iq,
                                            bool no_drop);
sdk_ret_t capri_qos_uplink_input_map_update(tm_port_t port, uint32_t dot1q_pcp,
                                            tm_q_t iq);

}    // namespace sdk
}    // namespace platform
}    // namespace capri

using sdk::platform::capri::qos_queues_t;

#endif  // __SDK_CAPRI_QOS_HPP__
