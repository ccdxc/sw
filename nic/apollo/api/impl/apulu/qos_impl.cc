//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of qos
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/indexer/indexer.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/apollo/api/impl/apulu/qos_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_QOS_IMPL - qos functionality
/// \ingroup PDS_QOS
/// @{

sdk_ret_t
qos_impl::qos_program_tm_uplink_params_ (tm_uplink_q_params_t *q_params) {
    sdk_ret_t ret;
    tm_port_t port;
    tm_port_t uplink_port_start = sdk::asic::pd::asicpd_qos_uplink_port_start();
    tm_port_t uplink_port_end = sdk::asic::pd::asicpd_qos_uplink_port_end();

    for (port = uplink_port_start; port <= uplink_port_end; port++) {
        ret = sdk::asic::pd::asicpd_qos_uplink_q_params_update(port, q_params);
        if (ret != SDK_RET_OK) {
            SDK_TRACE_ERR("Error programming q params for tm-port %u, ret %u",
                          port, ret);
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
qos_impl::qos_get_alloc_q_count_ (qos_group_t qos_group, bool no_drop,
                                  qos_q_alloc_params_t *alloc_params) {
    memset(alloc_params, 0, sizeof(*alloc_params));

    switch (qos_group) {
    case QOS_GROUP_DEFAULT:
        alloc_params->cnt_uplink_iq = 1;
        alloc_params->cnt_txdma_iq = 1;
        alloc_params->cnt_oq = 1;
        alloc_params->dest_oq_type = sdk::qos::QOS_OQ_COMMON;
        alloc_params->pcie_oq = true;
        break;

    case QOS_GROUP_CONTROL:
        alloc_params->cnt_uplink_iq = 1;
        // for no_drop class, allocate 2 Qs in TxDMA
        if (no_drop) {
            alloc_params->cnt_txdma_iq =
                          sdk::asic::pd::asicpd_qos_max_tx_qs_per_class();
        } else {
            alloc_params->cnt_txdma_iq = 1;
        }
        alloc_params->cnt_oq = 1;
        alloc_params->dest_oq_type = sdk::qos::QOS_OQ_COMMON;
        alloc_params->pcie_oq = true;
        break;

    case QOS_GROUP_SPAN:
        alloc_params->cnt_oq = 1;
        alloc_params->dest_oq_type = sdk::qos::QOS_OQ_COMMON;
        alloc_params->pcie_oq = true;
        break;

    case QOS_GROUP_CPU_COPY:
        alloc_params->cnt_oq = 1;
        alloc_params->dest_oq_type = sdk::qos::QOS_OQ_RXDMA;
        alloc_params->pcie_oq = false;
        break;

    default:
        SDK_ASSERT(0);
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
qos_impl::qos_class_alloc_queues_ (qos_group_t qos_group, bool no_drop,
                                   qos_queues_t *qos_queue) {
    sdk_ret_t ret;
    qos_q_alloc_params_t q_alloc_params;

    ret = qos_get_alloc_q_count_(qos_group, no_drop, &q_alloc_params);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    switch (qos_group) {
    case QOS_GROUP_SPAN:
        qos_queue->p4eg_iq[sdk::qos::QOS_IQ_COMMON] =
                           sdk::asic::pd::asicpd_qos_span_queue();
        break;
    case QOS_GROUP_CPU_COPY:
        qos_queue->p4eg_iq[sdk::qos::QOS_IQ_COMMON] =
                           sdk::asic::pd::asicpd_qos_cpu_copy_queue();
        break;
    default:
        // allocate the iqs first in uplink and txdma
        if (q_alloc_params.cnt_uplink_iq) {
            ret = qos_impl_db()->qos_uplink_iq_idxr()->alloc(
                                 (uint32_t *)&qos_queue->uplink_iq);
            if (ret != SDK_RET_OK) {
                return SDK_RET_NO_RESOURCE;
            }

            qos_queue->p4ig_iq[sdk::qos::QOS_IQ_RX] =
                       sdk::asic::pd::asicpd_qos_p4ig_uplink_iq_offset() +
                       qos_queue->uplink_iq;

            if (qos_queue->p4ig_iq[sdk::qos::QOS_IQ_RX] ==
                                   sdk::asic::pd::asicpd_qos_span_queue()) {
                qos_queue->p4eg_iq[sdk::qos::QOS_IQ_RX] =
                    sdk::asic::pd::asicpd_qos_p4eg_uplink_span_q_replacement();
            } else if (qos_queue->p4ig_iq[sdk::qos::QOS_IQ_RX] ==
                                  sdk::asic::pd::asicpd_qos_cpu_copy_queue()) {
                qos_queue->p4eg_iq[sdk::qos::QOS_IQ_RX] =
                sdk::asic::pd::asicpd_qos_p4eg_uplink_cpu_copy_q_replacement();
            } else {
                qos_queue->p4eg_iq[sdk::qos::QOS_IQ_RX] = qos_queue->p4ig_iq[sdk::qos::QOS_IQ_RX];
            }
            SDK_TRACE_DEBUG("qos_group %u, p4ig iq %u, p4eg iq %u", qos_group,
                            qos_queue->p4ig_iq[sdk::qos::QOS_IQ_RX],
                            qos_queue->p4eg_iq[sdk::qos::QOS_IQ_RX]);
        }
        break;
    }

    // allocate oqs
    if (q_alloc_params.cnt_oq) {
        if (q_alloc_params.dest_oq_type == sdk::qos::QOS_OQ_COMMON) {
            // use the common dest_oq idxr
            ret = qos_impl_db()->qos_common_oq_idxr()->alloc(
                                 (uint32_t*)&qos_queue->dest_oq);
            if (ret != SDK_RET_OK) {
                return SDK_RET_NO_RESOURCE;
            }
        } else {
            // use the rxdma specific idxr
            ret = qos_impl_db()->qos_rxdma_oq_idxr()->alloc(
                                 (uint32_t*)&qos_queue->dest_oq);
            if (ret != SDK_RET_OK) {
                return SDK_RET_NO_RESOURCE;
            }
            qos_queue->dest_oq += sdk::asic::pd::asicpd_qos_rxdma_oq_offset();
        }
        SDK_TRACE_DEBUG("qos_group %u, dest_oq_type %u, dest_oq %u", qos_group,
                        q_alloc_params.dest_oq_type, qos_queue->dest_oq);
    }

    qos_queue->dest_oq_type = q_alloc_params.dest_oq_type;
    qos_queue->pcie_oq = q_alloc_params.pcie_oq;
    return SDK_RET_OK;
}

uint8_t
qos_impl::qos_class_group_get_dot1q_pcp_ (qos_class_t *qos_class) {
    switch (qos_class->qos_group) {
    case QOS_GROUP_CONTROL:
        return 1;

    case QOS_GROUP_DEFAULT:
    default:
        return 0;
    }
    return 0;
}

sdk_ret_t
qos_impl::qos_populate_q_params_ (tm_uplink_q_params_t *q_params,
                                  qos_class_t *qos_class,
                                  qos_queues_t *qos_queues) {
    bool has_pcp;
    bool has_dscp;
    uint32_t dot1q_pcp;

    memset(q_params, 0, sizeof(tm_uplink_q_params_t));
    q_params->iq = qos_queues->uplink_iq;
    if (sdk::qos::tm_q_valid(q_params->iq) == false) {
        // TODO scenario?
        SDK_TRACE_ERR("Invalid uplink iq");
        return SDK_RET_OK;
    }
    q_params->mtu = qos_class->mtu;
    q_params->xoff_threshold =
                   sdk::asic::pd::asicpd_qos_default_xoff_threshold();
    q_params->xon_threshold =
                  sdk::asic::pd::asicpd_qos_default_xon_threshold();

    SDK_ASSERT(sdk::qos::tm_q_valid(qos_queues->p4ig_iq[sdk::qos::QOS_IQ_RX]));
    q_params->p4_q = qos_queues->p4ig_iq[sdk::qos::QOS_IQ_RX];

    has_pcp = sdk::qos::cmap_type_pcp(qos_class->cmap.type);
    has_dscp = sdk::qos::cmap_type_dscp(qos_class->cmap.type);

    if (has_pcp) {
        dot1q_pcp = qos_class->cmap.dot1q_pcp;
    } else {
        // for dscp based classification, derive the dot1q_pcp
        // based on the qos_group
        dot1q_pcp = qos_class_group_get_dot1q_pcp_(qos_class);
    }
    q_params->dot1q_pcp = dot1q_pcp;
    q_params->no_drop = qos_class->no_drop;
    q_params->use_ip = has_dscp;
    memcpy(q_params->ip_dscp, qos_class->cmap.ip_dscp,
                     sizeof(qos_class->cmap.ip_dscp));
    SDK_TRACE_DEBUG("qos_group %u, mtu %u, xoff %u, xon %u, p4_q %u, "
                    "dot1q_pcp %u, no_drop %u, use_ip %u",
                    qos_class->qos_group, q_params->mtu,
                    q_params->xoff_threshold, q_params->xon_threshold,
                    q_params->p4_q, q_params->dot1q_pcp, q_params->no_drop,
                    q_params->use_ip);
    return SDK_RET_OK;
}

sdk_ret_t
qos_impl::qos_class_create_ (qos_class_t *qos_class) {
    tm_uplink_q_params_t q_params = { 0 };
    qos_queues_t qos_queues = { 0 };
    qos_group_t qos_group = qos_class->qos_group;

    // allocate resources
    qos_class_alloc_queues_(qos_group, qos_class->no_drop, &qos_queues);

    // TODO reset stats

    // q params
    qos_populate_q_params_(&q_params, qos_class, &qos_queues);

    qos_program_tm_uplink_params_(&q_params);
    return SDK_RET_OK;
}

sdk_ret_t
qos_impl::qos_init (void) {
    qos_class_t qos_class;
    uint32_t dscp_val = QOS_CONTROL_DEFAULT_DSCP;

    // create qos default class with cos 0
    memset(&qos_class, 0, sizeof(qos_class_t));
    qos_class.qos_group = QOS_GROUP_DEFAULT;
    qos_class.cmap.type = sdk::qos::QOS_CMAP_TYPE_PCP;
    qos_class.cmap.dot1q_pcp = 0;
    qos_class.no_drop = false;
    qos_class.mtu = 9216;
    qos_class_create_(&qos_class);

    // create qos control class with dscp value 63
    memset(&qos_class, 0, sizeof(qos_class_t));
    qos_class.qos_group = QOS_GROUP_CONTROL;
    qos_class.cmap.type = sdk::qos::QOS_CMAP_TYPE_DSCP;
    qos_class.cmap.ip_dscp[dscp_val] = true;
    qos_class.no_drop = false;
    qos_class.mtu = 9216;
    qos_class_create_(&qos_class);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
