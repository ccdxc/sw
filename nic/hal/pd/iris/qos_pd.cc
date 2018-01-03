#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/qos_pd.hpp"
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
        case QOS_GROUP_ADMIN:
            alloc_params->cnt_txdma_iq = 1;
            break;
        case NUM_QOS_GROUPS:
            HAL_ASSERT(0);
            break;
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Qos-class Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_create(pd_qos_class_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;;
    pd_qos_class_t *pd_qos_class;

    HAL_TRACE_DEBUG("pd-qos::{}: creating pd state for qos_class: {}",
                    __func__, args->qos_class->key);

    // Create qos_class PD
    pd_qos_class = qos_class_pd_alloc_init();
    if (pd_qos_class == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    qos_class_link_pi_pd(pd_qos_class, args->qos_class);

    // Allocate Resources
    ret = qos_class_pd_alloc_res(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("pd-qos::{}: Unable to alloc. resources for Qos-class: {} ret {}",
                      __func__, args->qos_class->key, ret);
        goto end;
    }

    // Program HW
    ret = qos_class_pd_program_hw(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-qos::{}: Unable to program hw for Qos-class: {} ret {}",
                      __func__, args->qos_class->key, ret);
        goto end;
    }

end:
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-qos::{}: Error in programming hw for Qos-class: {}: ret: {}",
                      __func__, args->qos_class->key, ret);
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
pd_qos_class_update (pd_qos_class_args_t *pd_qos_class_upd_args)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pd-qos::{}: updating pd state for qos_class:{}",
                    __func__,
                    pd_qos_class_upd_args->qos_class->key);

    return ret;
}

//-----------------------------------------------------------------------------
// PD Qos-class Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_delete (pd_qos_class_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_qos_class_t *qos_class_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->qos_class != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->qos_class->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-qos:{}:deleting pd state for qos_class {}",
                    __func__, args->qos_class->key);
    qos_class_pd = (pd_qos_class_t *)args->qos_class->pd;

    // TODO: deprogram hw

    // free up the resource and memory
    ret = qos_class_pd_cleanup(qos_class_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-qos:{}:failed pd qos_class cleanup Qos-class {}, ret {}",
                      __func__, args->qos_class->key, ret);
    }

    return ret;
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
hal_ret_t
qos_class_pd_cleanup(pd_qos_class_t *qos_class_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!qos_class_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = qos_class_pd_dealloc_res(qos_class_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-qos:{}: unable to dealloc res for qos_class: {}",
                      __func__,
                      ((qos_class_t *)(qos_class_pd->pi_qos_class))->key);
        goto end;
    }

    // Delinking PI<->PD
    qos_class_delink_pi_pd(qos_class_pd, (qos_class_t *)qos_class_pd->pi_qos_class);

    // Freeing PD
    qos_class_pd_free(qos_class_pd);
end:
    return ret;
}


// ----------------------------------------------------------------------------
// Allocate resources for PD Qos-class
// ----------------------------------------------------------------------------
hal_ret_t
qos_class_pd_alloc_res (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_qos_q_alloc_params_t q_alloc_params;
    indexer::status         rs = indexer::SUCCESS;
    qos_class_t             *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;
    qos_group_t             qos_group;
    uint32_t                p4_q_idx = 0;

    pd_qos_class->cells_per_mtu = capri_tm_buffer_bytes_to_cells(qos_class->mtu);

    // Figure out the number of queues needed for class
    ret = pd_qos_get_alloc_q_count(qos_class, &q_alloc_params);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    qos_group = qos_class_get_qos_group(qos_class);

    HAL_ASSERT(q_alloc_params.cnt_uplink_iq <= 1);
    HAL_ASSERT(q_alloc_params.cnt_txdma_iq <= HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS);
    HAL_ASSERT(q_alloc_params.cnt_oq <= 1);

    HAL_TRACE_DEBUG("pd-qos::{}: Qos-class {} resource required "
                    "iq: uplink {} txdma {}/dest_oq {} type {} pcie {}",
                    __func__,
                    qos_class->key,
                    q_alloc_params.cnt_uplink_iq,
                    q_alloc_params.cnt_txdma_iq, 
                    q_alloc_params.cnt_oq,
                    q_alloc_params.dest_oq_type,
                    q_alloc_params.pcie_oq);

    // Allocate queues in all the ports
    //

    if (qos_group == QOS_GROUP_SPAN) {
        pd_qos_class->p4_q[HAL_PD_QOS_IQ_COMMON] = HAL_TM_P4_SPAN_QUEUE;
        pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_COMMON] = HAL_TM_P4_SPAN_QUEUE;
    } else if (qos_group == QOS_GROUP_CPU_COPY) {
        pd_qos_class->p4_q[HAL_PD_QOS_IQ_COMMON] = HAL_TM_P4_CPU_COPY_QUEUE;
        pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_COMMON] = HAL_TM_P4_CPU_COPY_QUEUE;
    } else {
        // Allocate the iqs first in uplink and txdma
        if (q_alloc_params.cnt_uplink_iq) {
            rs = g_hal_state_pd->qos_iq_idxr(TM_PORT_TYPE_UPLINK)->alloc(
                (uint32_t*)&pd_qos_class->uplink_iq);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }

            pd_qos_class->p4_q[HAL_PD_QOS_IQ_RX] = 
                    HAL_TM_P4_UPLINK_IQ_OFFSET + pd_qos_class->uplink_iq;
            pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_RX] = 
                    HAL_TM_P4_UPLINK_EGRESS_OQ_OFFSET + pd_qos_class->uplink_iq;
        }

        if (q_alloc_params.cnt_txdma_iq == 1) {
            p4_q_idx = HAL_PD_QOS_IQ_COMMON;
        } else {
            p4_q_idx = HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0;
        }
        for (unsigned i = 0; i < q_alloc_params.cnt_txdma_iq; i++, p4_q_idx++) {
            rs = g_hal_state_pd->qos_iq_idxr(TM_PORT_TYPE_DMA)->alloc(
                (uint32_t*)&pd_qos_class->txdma_iq[i]);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }
            pd_qos_class->p4_q[p4_q_idx]= pd_qos_class->txdma_iq[i];
            pd_qos_class->p4_eg_q[p4_q_idx]= pd_qos_class->txdma_iq[i];
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

    // Verify the buffer configuration fits within the budget

    // Verify the scheduler configuration inter-ops with other classes

    return ret;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD Qos-class
// ----------------------------------------------------------------------------
hal_ret_t
qos_class_pd_dealloc_res(pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;

    if (tm_q_valid(pd_qos_class->uplink_iq)) {
        g_hal_state_pd->qos_iq_idxr(TM_PORT_TYPE_UPLINK)->free(pd_qos_class->uplink_iq);
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->txdma_iq); i++) {
        if (tm_q_valid(pd_qos_class->txdma_iq[i])) {
            g_hal_state_pd->qos_iq_idxr(TM_PORT_TYPE_DMA)->free(pd_qos_class->txdma_iq[i]);
        }
    }

    if (tm_q_valid(pd_qos_class->dest_oq)) {
        if (pd_qos_class->dest_oq_type == HAL_PD_QOS_OQ_COMMON) {
            g_hal_state_pd->qos_common_oq_idxr()->free(pd_qos_class->dest_oq);
        } else {
            g_hal_state_pd->qos_rxdma_oq_idxr()->free(pd_qos_class->dest_oq);
        }
    }
    return ret;
}

// Program the mapping from the traffic class to the iq for accounting purpose
static hal_ret_t
program_iq (tm_port_t port, uint32_t tc, tm_q_t iq)
{
    hal_ret_t           ret = HAL_RET_OK;
    tm_tc_to_pg_map_t   tc_to_pg_map;

    if (!tm_q_valid(iq)) {
        return HAL_RET_OK;
    }

    tc_to_pg_map.tc = tc;
    tc_to_pg_map.pg = iq;

    ret = capri_tm_tc_map_update(port, 1, &tc_to_pg_map);
    return ret;
}

// Program the dscp to tc and the p4_oq map on uplinks
static hal_ret_t
program_uplink_map (tm_port_t port, pd_qos_class_t *pd_qos_class)
{
    hal_ret_t             ret = HAL_RET_OK;
    tm_uplink_input_map_t tm_uplink_input_map = {};
    qos_class_t           *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;

    HAL_ASSERT(HAL_ARRAY_SIZE(qos_class->uplink_cmap.ip_dscp) ==
               HAL_ARRAY_SIZE(tm_uplink_input_map.ip_dscp));
    memcpy(tm_uplink_input_map.ip_dscp, qos_class->uplink_cmap.ip_dscp, 
           HAL_ARRAY_SIZE(qos_class->uplink_cmap.ip_dscp));
    tm_uplink_input_map.tc = qos_class->uplink_cmap.dot1q_pcp;
    tm_uplink_input_map.p4_oq = pd_qos_class->p4_q[HAL_PD_QOS_IQ_RX];
    HAL_ASSERT(tm_q_valid(pd_qos_class->p4_q[HAL_PD_QOS_IQ_RX]));

    ret = capri_tm_uplink_input_map_update(port, &tm_uplink_input_map);
    return ret;
}

// Program the dest_oq scheduler in the given port
static hal_ret_t
program_oq (tm_port_t port, tm_q_t dest_oq, qos_class_t *qos_class)
{
    hal_ret_t ret = HAL_RET_OK;
    tm_queue_node_params_t q_node_params = {};
    tm_q_t parent_node = 0;

    if (!tm_q_valid(dest_oq)) {
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

    // Update the dest_oq config
    ret = capri_tm_scheduler_map_update(port, TM_QUEUE_NODE_TYPE_LEVEL_0,
                                        dest_oq, &q_node_params);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

static hal_ret_t
qos_class_pd_program_uplink_ports (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    tm_pg_params_t       pg_params;
    tm_port_t            port;
    qos_class_t          *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        if (tm_q_valid(pd_qos_class->uplink_iq)) {
            // Update the buffer pool parameters
            memset(&pg_params, 0, sizeof(pg_params));
            pg_params.mtu = pd_qos_class->cells_per_mtu;

            pg_params.reserved_min = qos_class->buffer.reserved_mtus * 
                                        pd_qos_class->cells_per_mtu;
            pg_params.xon_threshold = 
                capri_tm_buffer_bytes_to_cells(qos_class->buffer.xon_threshold);
            pg_params.headroom = qos_class->buffer.headroom_mtus *
                                    pd_qos_class->cells_per_mtu;
            pg_params.low_limit = 
                capri_tm_buffer_bytes_to_cells(qos_class->buffer.xoff_clear_limit);

            ret = capri_tm_pg_params_update(port, pd_qos_class->uplink_iq, &pg_params);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pd-qos::{}: Error programming the buffer params for "
                              "Qos-class {} on port {} ret {}",
                              __func__, qos_class->key, port, ret);
                return ret;
            }

            ret = program_iq(port, qos_class->uplink_cmap.dot1q_pcp, 
                             pd_qos_class->uplink_iq);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pd-qos::{}: Error programming the iq params for "
                              "Qos-class {} on port {} ret {}",
                              __func__, qos_class->key, port, ret);
                return ret;
            }

            ret = program_uplink_map(port, pd_qos_class);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pd-qos::{}: Error programming the uplink map params for "
                              "Qos-class {} on port {} ret {}",
                              __func__, qos_class->key, port, ret);
                return ret;
            }
        }

        if (tm_q_valid(pd_qos_class->dest_oq) && 
            (pd_qos_class->dest_oq_type == HAL_PD_QOS_OQ_COMMON)) {
            ret = program_oq(port, pd_qos_class->dest_oq, qos_class);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pd-qos::{}: Error programming the dest_oq params for "
                              "Qos-class {} on port {} ret {}",
                              __func__, qos_class->key, port, ret);
                return ret;
            }
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_p4_ports (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    tm_port_t            port;
    qos_class_t          *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;

    port = TM_PORT_INGRESS;
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_q); i++) {
        ret = program_iq(port, pd_qos_class->p4_q[i], 
                         pd_qos_class->p4_q[i]);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: Error programming the iq params for "
                          "Qos-class {} on port {} ret {}",
                          __func__, qos_class->key, port, ret);
            return ret;
        }
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_q); i++) {
        ret = program_oq(port, pd_qos_class->p4_q[i], qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: Error programming the dest_oq params for "
                          "Qos-class {} on port {} ret {}",
                          __func__, qos_class->key, port, ret);
            return ret;
        }
    }

    port = TM_PORT_EGRESS;
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_eg_q); i++) {
        ret = program_iq(port, pd_qos_class->p4_eg_q[i], 
                         pd_qos_class->p4_eg_q[i]);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: Error programming the iq params for "
                          "Qos-class {} on port {} ret {}",
                          __func__, qos_class->key, port, ret);
            return ret;
        }
    }
    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_eg_q); i++) {
        ret = program_oq(port, pd_qos_class->p4_eg_q[i], qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: Error programming the dest_oq params for "
                          "Qos-class {} on port {} ret {}",
                          __func__, qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_dma_ports (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    tm_port_t            port;
    qos_class_t          *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;

    for (port = TM_DMA_PORT_BEGIN; port <= TM_DMA_PORT_END; port++) {
        for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->txdma_iq); i++) {
            ret = program_iq(port, pd_qos_class->txdma_iq[i], 
                             pd_qos_class->txdma_iq[i]);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pd-qos::{}: Error programming the iq params for "
                              "Qos-class {} on port {} ret {}",
                              __func__, qos_class->key, port, ret);
                return ret;
            }
        }
        ret = program_oq(port, pd_qos_class->dest_oq, qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: Error programming the dest_oq params for "
                          "Qos-class {} on port {} ret {}",
                          __func__, qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

#define QOS_ACTION(_arg) d.qos_action_u.qos_qos._arg
static hal_ret_t
qos_class_pd_program_qos_table (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t      ret = HAL_RET_OK;
    qos_class_t    *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;
    DirectMap      *qos_tbl = NULL;
    qos_actiondata d;
    uint32_t       qos_class_id; 

    qos_tbl = g_hal_state_pd->dm_table(P4TBL_ID_QOS);
    HAL_ASSERT_RETURN(qos_tbl != NULL, HAL_RET_ERR);

    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pd_qos_class->p4_q); i++) {
        if (!tm_q_valid(pd_qos_class->p4_q[i])) {
            continue;
        }
        qos_class_id = pd_qos_class->p4_q[i];

        memset(&d, 0, sizeof(d));
        QOS_ACTION(egress_tm_oq) = pd_qos_class->p4_eg_q[i];
        HAL_ASSERT(tm_q_valid(pd_qos_class->p4_eg_q[i]));
        QOS_ACTION(dest_tm_oq) = pd_qos_class->dest_oq;
        if (i != HAL_PD_QOS_IQ_RX) {
            QOS_ACTION(cos_en) = qos_class->marking.pcp_rewrite_en;
            QOS_ACTION(cos) = qos_class->marking.pcp;
            QOS_ACTION(dscp_en) = qos_class->marking.dscp_rewrite_en;
            QOS_ACTION(dscp) = qos_class->marking.dscp;
        }

        if (qos_class_id == 0) {
            ret = qos_tbl->update(qos_class_id, &d);
        } else {
            ret = qos_tbl->insert_withid(&d, qos_class_id);
        }
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-qos::{}: qos table write failure, qos-class {}, "
                          "qos-class-id {} ret {}",
                          __func__, qos_class->key, qos_class_id, ret);
            return ret;
        }
        HAL_TRACE_DEBUG("pd-qos::{}: qos table qos-class {} qos-class-id {} programmed "
                        " egress_tm_oq {} dest_oq {} cos_en {} cos {} dscp_en {} dscp {} ",
                        __func__, qos_class->key, qos_class_id, 
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
hal_ret_t
qos_class_pd_program_hw(pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    qos_class_t          *qos_class = (qos_class_t *)pd_qos_class->pi_qos_class;

    ret = qos_class_pd_program_uplink_ports(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("pd-qos::{}: Error programming the uplink ports for "
                      "Qos-class {} ret {}",
                      __func__, qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_p4_ports(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("pd-qos::{}: Error programming the p4 ports for "
                      "Qos-class {} ret {}",
                      __func__, qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_dma_ports(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("pd-qos::{}: Error programming the dma ports for "
                      "Qos-class {} ret {}",
                      __func__, qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_qos_table(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("pd-qos::{}: Error programming the qos table for "
                      "Qos-class {} ret {}",
                      __func__, qos_class->key, ret);
        return ret;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void
qos_class_link_pi_pd(pd_qos_class_t *pd_qos_class, qos_class_t *pi_qos_class)
{
    pd_qos_class->pi_qos_class = pi_qos_class;
    qos_class_set_pd_qos_class(pi_qos_class, pd_qos_class);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
void
qos_class_delink_pi_pd(pd_qos_class_t *pd_qos_class, qos_class_t *pi_qos_class)
{
    pd_qos_class->pi_qos_class = NULL;
    qos_class_set_pd_qos_class(pi_qos_class, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_make_clone(qos_class_t *qos_class, qos_class_t *clone)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_qos_class_t *pd_qos_class_clone = NULL;

    pd_qos_class_clone = qos_class_pd_alloc_init();
    if (pd_qos_class_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_qos_class_clone, qos_class->pd, sizeof(pd_qos_class_t));

    qos_class_link_pi_pd(pd_qos_class_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_mem_free(pd_qos_class_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_qos_class_t        *qos_class_pd;

    qos_class_pd = (pd_qos_class_t *)args->qos_class->pd;
    qos_class_pd_mem_free(qos_class_pd);

    return ret;
}

hal_ret_t
qos_class_get_qos_class_id (qos_class_t *qos_class, 
                            if_t *dest_if, 
                            uint32_t *qos_class_id) 
{
    pd_qos_class_t *pd_qos_class;
    tm_port_t      dest_port;
    uint32_t       group = 0;
        
    if (!qos_class) {
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }

    if (dest_if) {
        dest_port = if_get_tm_oport(dest_if);
    } else {
        // If the dest if is not available, treat it as destined to uplink-0
        dest_port = TM_PORT_UPLINK_0;
    }
    if (dest_port == HAL_PORT_INVALID) {
        return HAL_RET_IF_NOT_FOUND;
    }

    pd_qos_class = qos_class->pd;

    *qos_class_id = HAL_TM_INVALID_Q;
    if (tm_port_is_uplink_port(dest_port)) {
        group = dest_port % 2;
        if (tm_q_valid(pd_qos_class->p4_q[HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0 + group])) {
            *qos_class_id = pd_qos_class->p4_q[HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0 + group];
        }
    } else if (tm_port_is_dma_port(dest_port)) {
        if (tm_q_valid(pd_qos_class->p4_q[HAL_PD_QOS_IQ_RX])) {
            *qos_class_id = pd_qos_class->p4_q[HAL_PD_QOS_IQ_RX];
        }
    }

    if (!tm_q_valid(*qos_class_id) &&
        tm_q_valid(pd_qos_class->p4_q[HAL_PD_QOS_IQ_COMMON])) {
        *qos_class_id = pd_qos_class->p4_q[HAL_PD_QOS_IQ_COMMON];
    }

    if (!tm_q_valid(*qos_class_id)) {
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
