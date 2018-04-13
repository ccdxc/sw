// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/qos_api.hpp"

namespace hal {
namespace pd {

typedef struct pd_qos_q_alloc_params_s {
    uint32_t         cnt_uplink_iq;
    uint32_t         cnt_txdma_iq;
    uint32_t         cnt_oq;
    pd_qos_oq_type_e dest_oq_type;
    bool             pcie_oq;
} pd_qos_q_alloc_params_t;

static hal_ret_t
pd_qos_get_alloc_q_count (qos_class_t *qos_class,
                          pd_qos_q_alloc_params_t *alloc_params)
{
    qos_group_t qos_group;

    qos_group = qos_class_get_qos_group(qos_class);
    memset(alloc_params, 0, sizeof(*alloc_params));

    switch(qos_group) {
        case QOS_GROUP_DEFAULT:
        case QOS_GROUP_USER_DEFINED_1:
        case QOS_GROUP_USER_DEFINED_2:
        case QOS_GROUP_USER_DEFINED_3:
        case QOS_GROUP_USER_DEFINED_4:
        case QOS_GROUP_USER_DEFINED_5:
        case QOS_GROUP_USER_DEFINED_6:
            alloc_params->cnt_uplink_iq = 1;
            if (qos_class_is_no_drop(qos_class)) {
                alloc_params->cnt_txdma_iq = HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS;
            } else {
                alloc_params->cnt_txdma_iq = 1;
            }
            alloc_params->cnt_oq = 1;
            alloc_params->dest_oq_type = HAL_PD_QOS_OQ_COMMON;
            alloc_params->pcie_oq = true;
            break;
        case QOS_GROUP_CONTROL:
            alloc_params->cnt_txdma_iq = 1;
            alloc_params->cnt_oq = 1;
            alloc_params->dest_oq_type = HAL_PD_QOS_OQ_RXDMA;
            alloc_params->pcie_oq = false;
            break;
        case QOS_GROUP_SPAN:
            alloc_params->cnt_oq = 1;
            alloc_params->dest_oq_type = HAL_PD_QOS_OQ_COMMON;
            alloc_params->pcie_oq = true;
            break;
        case QOS_GROUP_RX_PROXY_NO_DROP:
        case QOS_GROUP_RX_PROXY_DROP:
        case QOS_GROUP_TX_PROXY_NO_DROP:
        case QOS_GROUP_TX_PROXY_DROP:
            alloc_params->cnt_txdma_iq = 1;
            alloc_params->cnt_oq = 1;
            alloc_params->dest_oq_type = HAL_PD_QOS_OQ_RXDMA;
            alloc_params->pcie_oq = false;
            break;
        case QOS_GROUP_CPU_COPY:
            alloc_params->cnt_oq = 1;
            alloc_params->dest_oq_type = HAL_PD_QOS_OQ_RXDMA;
            alloc_params->pcie_oq = false;
            break;
        case NUM_QOS_GROUPS:
            HAL_ASSERT(0);
            break;
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static void
qos_class_pd_link_pi_pd (pd_qos_class_t *pd_qos_class, qos_class_t *pi_qos_class)
{
    pd_qos_class->pi_qos_class = pi_qos_class;
    qos_class_set_pd_qos_class(pi_qos_class, pd_qos_class);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
static void
qos_class_pd_delink_pi_pd (pd_qos_class_t *pd_qos_class, qos_class_t *pi_qos_class)
{
    pd_qos_class->pi_qos_class = NULL;
    qos_class_set_pd_qos_class(pi_qos_class, NULL);
}

static hal_ret_t
qos_class_pd_alloc_queues (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t               ret;
    pd_qos_q_alloc_params_t q_alloc_params;
    indexer::status         rs = indexer::SUCCESS;
    qos_class_t             *qos_class = pd_qos_class->pi_qos_class;
    qos_group_t             qos_group;
    uint32_t                p4_q_idx = 0;

    // Figure out the number of queues needed for class
    ret = pd_qos_get_alloc_q_count(qos_class, &q_alloc_params);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    qos_group = qos_class_get_qos_group(qos_class);

    HAL_ASSERT(q_alloc_params.cnt_uplink_iq <= 1);
    HAL_ASSERT(q_alloc_params.cnt_txdma_iq <= HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS);
    HAL_ASSERT(q_alloc_params.cnt_oq <= 1);

    HAL_TRACE_DEBUG("Qos-class {} resource required "
                    "iq: uplink {} txdma {}/dest_oq {} type {} pcie {}",
                    qos_class->key,
                    q_alloc_params.cnt_uplink_iq,
                    q_alloc_params.cnt_txdma_iq,
                    q_alloc_params.cnt_oq,
                    q_alloc_params.dest_oq_type,
                    q_alloc_params.pcie_oq);

    // Allocate queues in all the ports

    if (qos_group == QOS_GROUP_SPAN) {
        pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_COMMON] = HAL_TM_P4_SPAN_QUEUE;
    } else if (qos_group == QOS_GROUP_CPU_COPY) {
        pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_COMMON] = HAL_TM_P4_CPU_COPY_QUEUE;
    } else {
        // Allocate the iqs first in uplink and txdma
        if (q_alloc_params.cnt_uplink_iq) {
            rs = g_hal_state_pd->qos_uplink_iq_idxr()->alloc(
                (uint32_t*)&pd_qos_class->uplink.iq);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }

            pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX] =
                HAL_TM_P4_UPLINK_IQ_OFFSET + pd_qos_class->uplink.iq;
            if (pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX] == HAL_TM_P4_SPAN_QUEUE) {
                pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_RX] = HAL_TM_P4_EG_UPLINK_SPAN_QUEUE_REPLACEMENT;
            } else if (pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX] == HAL_TM_P4_CPU_COPY_QUEUE) {
                pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_RX] = HAL_TM_P4_EG_UPLINK_CPU_COPY_QUEUE_REPLACEMENT;
            } else {
                pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_RX] = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX];
            }
        }

        if (q_alloc_params.cnt_txdma_iq == 1) {
            p4_q_idx = HAL_PD_QOS_IQ_COMMON;
        } else {
            p4_q_idx = HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0;
        }
        for (unsigned i = 0; i < q_alloc_params.cnt_txdma_iq; i++, p4_q_idx++) {
            rs = g_hal_state_pd->qos_txdma_iq_idxr()->alloc(
                (uint32_t*)&pd_qos_class->txdma[i].iq);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }
            pd_qos_class->p4_ig_q[p4_q_idx] = pd_qos_class->txdma[i].iq;
            pd_qos_class->p4_eg_q[p4_q_idx] = pd_qos_class->txdma[i].iq;
        }
    }

    // Allocate oqs
    if (q_alloc_params.cnt_oq) {
        if (q_alloc_params.dest_oq_type == HAL_PD_QOS_OQ_COMMON) {
            // use the common dest_oq idxr
            rs = g_hal_state_pd->qos_common_oq_idxr()->alloc(
                (uint32_t*)&pd_qos_class->dest_oq);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }
        } else {
            // use the rxdma specific idxr
            rs = g_hal_state_pd->qos_rxdma_oq_idxr()->alloc(
                (uint32_t*)&pd_qos_class->dest_oq);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }
            pd_qos_class->dest_oq += HAL_TM_RXDMA_OQ_OFFSET;
        }
    }

    pd_qos_class->dest_oq_type = q_alloc_params.dest_oq_type;
    pd_qos_class->pcie_oq = q_alloc_params.pcie_oq;

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Qos-class
// ----------------------------------------------------------------------------
static hal_ret_t
qos_class_pd_alloc_res (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t               ret = HAL_RET_OK;
    qos_class_t             *qos_class = pd_qos_class->pi_qos_class;

    ret = qos_class_pd_alloc_queues(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error allocating queues for qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    // Verify the scheduler configuration inter-ops with other classes
    return ret;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Qos-class
// ----------------------------------------------------------------------------
static hal_ret_t
qos_class_pd_dealloc_res (pd_qos_class_t *pd_qos_class)
{

    if (capri_tm_q_valid(pd_qos_class->uplink.iq)) {
        g_hal_state_pd->qos_uplink_iq_idxr()->free(pd_qos_class->uplink.iq);
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->txdma); i++) {
        if (capri_tm_q_valid(pd_qos_class->txdma[i].iq)) {
            g_hal_state_pd->qos_txdma_iq_idxr()->free(pd_qos_class->txdma[i].iq);
        }
    }

    if (capri_tm_q_valid(pd_qos_class->dest_oq)) {
        if (pd_qos_class->dest_oq_type == HAL_PD_QOS_OQ_COMMON) {
            g_hal_state_pd->qos_common_oq_idxr()->free(pd_qos_class->dest_oq);
        } else {
            HAL_ASSERT(pd_qos_class->dest_oq >= HAL_TM_RXDMA_OQ_OFFSET);
            g_hal_state_pd->qos_rxdma_oq_idxr()->free(
                pd_qos_class->dest_oq - HAL_TM_RXDMA_OQ_OFFSET);
        }
    }
    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// PD Qos-class Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Qos-class
//  Note:
//      - Just free up whatever PD has.
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
static hal_ret_t
qos_class_pd_cleanup (pd_qos_class_t *qos_class_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!qos_class_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = qos_class_pd_dealloc_res(qos_class_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to dealloc res for qos_class: {}",
                      qos_class_pd->pi_qos_class->key);
        goto end;
    }

    // Delinking PI<->PD
    qos_class_pd_delink_pi_pd(qos_class_pd, qos_class_pd->pi_qos_class);

    // Freeing PD
    qos_class_pd_free(qos_class_pd);
end:
    return ret;
}

// Program the oq scheduler in the given port
static hal_ret_t
program_oq (tm_port_t port, tm_q_t oq, qos_class_t *qos_class)
{
    hal_ret_t ret = HAL_RET_OK;
    tm_queue_node_params_t q_node_params = {};
    tm_q_t parent_node = 0;

    if (!capri_tm_q_valid(oq)) {
        return HAL_RET_OK;
    }

    q_node_params.parent_node = parent_node;
    switch(qos_class->sched.type) {
        case QOS_SCHED_TYPE_DWRR:
            q_node_params.sched_type = TM_SCHED_TYPE_DWRR;
            q_node_params.dwrr.weight = qos_class->sched.dwrr.bw;
            break;
        case QOS_SCHED_TYPE_STRICT:
            q_node_params.sched_type = TM_SCHED_TYPE_STRICT;
            q_node_params.strict.rate = qos_class->sched.strict.bps;
            break;
    }

    // Update the oq config
    ret = capri_tm_scheduler_map_update(port, TM_QUEUE_NODE_TYPE_LEVEL_0,
                                        oq, &q_node_params);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_uplink_iq_params (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t             ret = HAL_RET_OK;
    tm_uplink_iq_params_t iq_params;
    tm_port_t             port;
    qos_class_t           *qos_class = pd_qos_class->pi_qos_class;
    tm_q_t                iq;

    iq = pd_qos_class->uplink.iq;
    if (!capri_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }

    memset(&iq_params, 0, sizeof(iq_params));

    iq_params.mtu = qos_class->mtu;
    iq_params.xoff_threshold = qos_class->pfc.xoff_threshold;
    iq_params.xon_threshold = qos_class->pfc.xon_threshold;
    HAL_ASSERT(capri_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX]));
    iq_params.p4_q = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX];

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        ret = capri_tm_uplink_iq_params_update(port, iq, &iq_params);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the iq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_uplink_iq_map (pd_qos_class_t *pd_qos_class)
{
    // Program the dscp, dot1q to iq map
    hal_ret_t                  ret = HAL_RET_OK;
    tm_port_t                  port;
    qos_class_t                *qos_class = pd_qos_class->pi_qos_class;
    tm_q_t                     iq;
    tm_uplink_input_dscp_map_t dscp_map = {0};

    iq = pd_qos_class->uplink.iq;
    if (!capri_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }

    HAL_ASSERT(sizeof(qos_class->uplink_cmap.ip_dscp) ==
               sizeof(dscp_map.ip_dscp));
    memcpy(dscp_map.ip_dscp, qos_class->uplink_cmap.ip_dscp,
           sizeof(qos_class->uplink_cmap.ip_dscp));
    dscp_map.dot1q_pcp = qos_class->uplink_cmap.dot1q_pcp;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        ret = capri_tm_uplink_input_map_update(port, 
                                               qos_class->uplink_cmap.dot1q_pcp,
                                               iq);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the uplink map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }

        ret = capri_tm_uplink_input_dscp_map_update(port, &dscp_map);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the uplink dscp map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_update_uplink_iq_map_remove (bool dot1q_remove, uint32_t dot1q_pcp,
                                          bool *ip_dscp_vals, uint32_t cnt_ip_dscp)
{
    hal_ret_t                  ret = HAL_RET_OK;
    tm_port_t                  port;
    qos_class_t                *default_qos_class = NULL;
    tm_uplink_input_dscp_map_t dscp_map = {0};
    uint32_t                   default_qos_class_dot1q_pcp = 0;
    tm_q_t                     default_qos_class_iq = 0;

    default_qos_class = find_qos_class_by_group(QOS_GROUP_DEFAULT);
    if (!default_qos_class) {
        HAL_TRACE_ERR("Default qos class is not created");
    } else {
        default_qos_class_dot1q_pcp = default_qos_class->uplink_cmap.dot1q_pcp;
        default_qos_class_iq = default_qos_class->pd->uplink.iq;
    }

    HAL_ASSERT(cnt_ip_dscp == HAL_ARRAY_SIZE(dscp_map.ip_dscp));
    memcpy(dscp_map.ip_dscp, ip_dscp_vals, sizeof(dscp_map.ip_dscp));
    dscp_map.dot1q_pcp = default_qos_class_dot1q_pcp;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        if (dot1q_remove) {
            ret = capri_tm_uplink_input_map_update(port, 
                                                   dot1q_pcp,
                                                   default_qos_class_iq);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming the uplink map "
                              "on port {} ret {}",
                              port, ret);
                return ret;
            }
        }

        ret = capri_tm_uplink_input_dscp_map_update(port, &dscp_map);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the uplink dscp map "
                          "on port {} ret {}",
                          port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_uplink_xoff (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t             ret = HAL_RET_OK;
    tm_port_t             port;
    qos_class_t           *qos_class = pd_qos_class->pi_qos_class;

    if (!capri_tm_q_valid(pd_qos_class->dest_oq) ||
        (pd_qos_class->dest_oq_type != HAL_PD_QOS_OQ_COMMON)) {
        return HAL_RET_OK;
    }

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        ret = capri_tm_uplink_oq_update(port, pd_qos_class->dest_oq, 
                                        qos_class->no_drop, qos_class->pfc.cos);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the xoff params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_scheduler (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    tm_port_t            port;
    qos_class_t          *qos_class = pd_qos_class->pi_qos_class;
    tm_q_t               oq;

    port = TM_PORT_INGRESS;
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_ig_q); i++) {
        oq = pd_qos_class->p4_ig_q[i];
        ret = program_oq(port, oq, qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the oq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }

    port = TM_PORT_EGRESS;
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_eg_q); i++) {
        oq = pd_qos_class->p4_eg_q[i];
        ret = program_oq(port, oq, qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the oq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }

    // On the DMA port
    for (port = TM_DMA_PORT_BEGIN; port <= TM_DMA_PORT_END; port++) {
        ret = program_oq(port, pd_qos_class->dest_oq, qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the oq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }

    // On the uplink port
    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        if (pd_qos_class->dest_oq_type == HAL_PD_QOS_OQ_COMMON) {
            ret = program_oq(port, pd_qos_class->dest_oq, qos_class);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming the oq params for "
                              "Qos-class {} on port {} ret {}",
                              qos_class->key, port, ret);
                return ret;
            }
        }
    }
    return HAL_RET_OK;
}

#define QOS_ACTION(_arg) d.qos_action_u.qos_qos._arg
static hal_ret_t
qos_class_pd_program_qos_table (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t      ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    qos_class_t    *qos_class = pd_qos_class->pi_qos_class;
    directmap      *qos_tbl = NULL;
    qos_actiondata d;
    uint32_t       qos_class_id;

    qos_tbl = g_hal_state_pd->dm_table(P4TBL_ID_QOS);
    HAL_ASSERT_RETURN(qos_tbl != NULL, HAL_RET_ERR);

    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_ig_q); i++) {
        if (!capri_tm_q_valid(pd_qos_class->p4_ig_q[i])) {
            continue;
        }
        qos_class_id = pd_qos_class->p4_ig_q[i];

        memset(&d, 0, sizeof(d));
        QOS_ACTION(egress_tm_oq) = pd_qos_class->p4_eg_q[i];
        HAL_ASSERT(capri_tm_q_valid(pd_qos_class->p4_eg_q[i]));
        QOS_ACTION(dest_tm_oq) = pd_qos_class->dest_oq;
        if (i != HAL_PD_QOS_IQ_RX) {
            QOS_ACTION(cos_en) = qos_class->marking.pcp_rewrite_en;
            QOS_ACTION(cos) = qos_class->marking.pcp;
            QOS_ACTION(dscp_en) = qos_class->marking.dscp_rewrite_en;
            QOS_ACTION(dscp) = qos_class->marking.dscp;
        }

        if (qos_class_id == 0) {
            sdk_ret = qos_tbl->update(qos_class_id, &d);
        } else {
            sdk_ret = qos_tbl->update(qos_class_id, &d);
            //ret = qos_tbl->insert_withid(&d, qos_class_id);
        }
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("qos table write failure, qos-class {}, "
                          "qos-class-id {} ret {}",
                          qos_class->key, qos_class_id, ret);
            return ret;
        }
        HAL_TRACE_DEBUG("qos table qos-class {} qos-class-id {} programmed "
                        " egress_tm_oq {} dest_oq {} cos_en {} cos {} dscp_en {} dscp {} ",
                        qos_class->key, qos_class_id,
                        QOS_ACTION(egress_tm_oq),
                        QOS_ACTION(dest_tm_oq),
                        QOS_ACTION(cos_en),
                        QOS_ACTION(cos),
                        QOS_ACTION(dscp_en),
                        QOS_ACTION(dscp));
    }
    return ret;
}
#undef QOS_ACTION

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
static hal_ret_t
qos_class_pd_program_hw (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    qos_class_t          *qos_class = pd_qos_class->pi_qos_class;

    ret = qos_class_pd_program_uplink_iq_params(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming uplink iq params for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_uplink_iq_map(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming uplink iq map for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_uplink_xoff(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming uplink oq xoff for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_scheduler(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error programming the p4 ports for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_qos_table(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error programming the qos table for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Qos-class Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_create (pd_qos_class_create_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;;
    pd_qos_class_t *pd_qos_class;

    HAL_TRACE_DEBUG("creating pd state for qos_class: {}",
                    args->qos_class->key);

    // Create qos_class PD
    pd_qos_class = qos_class_pd_alloc_init();
    if (pd_qos_class == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    qos_class_pd_link_pi_pd(pd_qos_class, args->qos_class);

    // Allocate Resources
    ret = qos_class_pd_alloc_res(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("Unable to alloc. resources for Qos-class: {} ret {}",
                      args->qos_class->key, ret);
        goto end;
    }

    // Program HW
    ret = qos_class_pd_program_hw(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to program hw for Qos-class: {} ret {}",
                      args->qos_class->key, ret);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error in programming hw for Qos-class: {}: ret: {}",
                      args->qos_class->key, ret);
        // unlink_pi_pd(pd_qos_class, args->qos_class);
        // qos_class_pd_free(pd_qos_class);
        qos_class_pd_cleanup(pd_qos_class);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Qos-class Update
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_update (pd_qos_class_update_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_qos_class_t *pd_qos_class;
    qos_class_t *qos_class;

    HAL_TRACE_DEBUG("updating pd state for qos_class:{}",
                    args->qos_class->key);

    qos_class = args->qos_class;
    pd_qos_class = args->qos_class->pd;

    if (args->mtu_changed || args->threshold_changed) {
        ret = qos_class_pd_program_uplink_iq_params(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming uplink iq params for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

    if (args->dot1q_pcp_changed || args->ip_dscp_changed) {
        // Remove the old associations and program the new association
        ret = qos_class_pd_program_uplink_iq_map(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming uplink iq map for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }

        ret = qos_class_pd_update_uplink_iq_map_remove(args->dot1q_pcp_changed, 
                                                       args->dot1q_pcp_src,
                                                       args->ip_dscp_remove,
                                                       HAL_ARRAY_SIZE(args->ip_dscp_remove));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error removing uplink iq map for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

    if (args->pfc_changed) {
        ret = qos_class_pd_program_uplink_xoff(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming uplink oq xoff for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

    if (args->scheduler_changed) {
        ret = qos_class_pd_program_scheduler(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the p4 ports for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

    if (args->marking_changed) {
        ret = qos_class_pd_program_qos_table(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the qos table for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Qos-class Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_delete (pd_qos_class_delete_args_t *args)
{
    hal_ret_t      ret;
    pd_qos_class_t *qos_class_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->qos_class != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->qos_class->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for qos_class {}",
                    args->qos_class->key);
    qos_class_pd = (pd_qos_class_t *)args->qos_class->pd;

    // TODO: deprogram hw

    // free up the resource and memory
    ret = qos_class_pd_cleanup(qos_class_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd qos_class cleanup Qos-class {}, ret {}",
                      args->qos_class->key, ret);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_make_clone (pd_qos_class_make_clone_args_t *args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_qos_class_t *pd_qos_class_clone = NULL;
    qos_class_t *qos_class = args->qos_class;
    qos_class_t *clone = args->clone;

    pd_qos_class_clone = qos_class_pd_alloc_init();
    if (pd_qos_class_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_qos_class_clone, qos_class->pd, sizeof(pd_qos_class_t));

    qos_class_pd_link_pi_pd(pd_qos_class_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// pd qos_class get
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_get (pd_qos_class_get_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
#if 0
    qos_class_t         *qos_class = args->qos_class;
    pd_qos_class_t      *qos_class_pd = (pd_qos_class_t *)qos_class->pd;
    QosClassGetResponse *rsp = args->rsp;

    auto qos_class_info = rsp->mutable_status()->mutable_epd_status();
#endif

    return ret;
}

// ----------------------------------------------------------------------------
// pd qos_class restore from response
// ----------------------------------------------------------------------------
static hal_ret_t
qos_class_pd_restore_data (pd_qos_class_restore_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
#if 0
    qos_class_t    *qos_class = args->qos_class;
    pd_qos_class_t *qos_class_pd = (pd_qos_class_t *)qos_class->pd;

    auto qos_class_info = args->qos_class_status->epd_status();
#endif

    return ret;
}

//-----------------------------------------------------------------------------
// pd qos_class restore
//-----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_restore (pd_qos_class_restore_args_t *args)
{
    hal_ret_t      ret;
    pd_qos_class_t *qos_class_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("Restoring pd state for qos_class {}", args->qos_class->key);

    // allocate PD qos_class state
    qos_class_pd = qos_class_pd_alloc_init();
    if (qos_class_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // link pi & pd
    qos_class_pd_link_pi_pd(qos_class_pd, args->qos_class);

    ret = qos_class_pd_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore pd data for qos_class: {}, err: {}",
                      args->qos_class->key, ret);
        goto end;
    }

    // TODO: Eventually call table program hw and hw calls will be
    //       a NOOP in p4pd code
#if 0
    // program hw
    ret = qos_class_pd_program_hw(qos_class_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}:failed to program hw", __FUNCTION__);
        goto end;
    }
#endif

end:

    if (ret != HAL_RET_OK) {
        qos_class_pd_cleanup(qos_class_pd);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_mem_free (pd_qos_class_mem_free_args_t *args)
{
    pd_qos_class_t        *pd_qos_class;

    pd_qos_class = (pd_qos_class_t *)args->qos_class->pd;
    qos_class_pd_mem_free(pd_qos_class);

    return HAL_RET_OK;
}

hal_ret_t
#if 0
qos_class_get_qos_class_id (qos_class_t *qos_class,
                            if_t *dest_if,
                            uint32_t *qos_class_id)
#endif
pd_qos_class_get_qos_class_id (pd_qos_class_get_qos_class_id_args_t *q_args)
{
    pd_qos_class_t *pd_qos_class;
    tm_port_t      dest_port;
    uint32_t       group = 0;
    pd_if_get_tm_oport_args_t args;

    qos_class_t *qos_class = q_args->qos_class;
    if_t *dest_if = q_args->dest_if;
    uint32_t *qos_class_id = q_args->qos_class_id;

    if (!qos_class) {
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }

    if (dest_if) {
        args.pi_if = dest_if;
        pd_if_get_tm_oport(&args);
        dest_port = args.tm_oport;
        // dest_port = if_get_tm_oport(dest_if);
    } else {
        // If the dest if is not available, treat it as destined to uplink-0
        dest_port = TM_PORT_UPLINK_0;
    }
    if (dest_port == HAL_PORT_INVALID) {
        return HAL_RET_IF_NOT_FOUND;
    }

    pd_qos_class = qos_class->pd;

    *qos_class_id = HAL_TM_INVALID_Q;
    if (capri_tm_port_is_uplink_port(dest_port)) {
        group = dest_port % 2;
        if (capri_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0 + group])) {
            *qos_class_id = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0 + group];
        }
    } else if (capri_tm_port_is_dma_port(dest_port)) {
        if (capri_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX])) {
            *qos_class_id = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX];
        }
    }

    if (!capri_tm_q_valid(*qos_class_id) &&
        capri_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_COMMON])) {
        *qos_class_id = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_COMMON];
    }

    if (!capri_tm_q_valid(*qos_class_id)) {
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }
    return HAL_RET_OK;
}

uint32_t
qos_class_get_admin_cos (void)
{
    return HAL_QOS_ADMIN_COS;
}

hal_ret_t
pd_qos_class_get_admin_cos (pd_qos_class_get_admin_cos_args_t *args)
{
    args->cos = HAL_QOS_ADMIN_COS;
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
