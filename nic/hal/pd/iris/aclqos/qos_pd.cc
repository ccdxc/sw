// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos_api.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/platform/capri/capri_tm_utils.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

using qos::QosClassStatusEpd;

namespace hal {
namespace pd {

//TODO: Move this to HAL slabs.
pd_qos_dscp_cos_map_t dscp_cos_txdma_iq_map[64];

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
            alloc_params->cnt_uplink_iq = 1;
            alloc_params->cnt_txdma_iq = 1;
            alloc_params->cnt_oq = 1;
            alloc_params->dest_oq_type = HAL_PD_QOS_OQ_COMMON;
            alloc_params->pcie_oq = true;
            break;
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
            SDK_ASSERT(0);
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
    uint32_t                p4_q_idx = 0, i = 0;

    // Figure out the number of queues needed for class
    ret = pd_qos_get_alloc_q_count(qos_class, &q_alloc_params);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    qos_group = qos_class_get_qos_group(qos_class);

    SDK_ASSERT(q_alloc_params.cnt_uplink_iq <= 1);
    SDK_ASSERT(q_alloc_params.cnt_txdma_iq <= HAL_PD_QOS_MAX_TX_QUEUES_PER_CLASS);
    SDK_ASSERT(q_alloc_params.cnt_oq <= 1);

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
        pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_COMMON] = CAPRI_TM_P4_SPAN_QUEUE;
    } else if (qos_group == QOS_GROUP_CPU_COPY) {
        pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_COMMON] = CAPRI_TM_P4_CPU_COPY_QUEUE;
    } else {
        // Allocate the iqs first in uplink and txdma
        if (q_alloc_params.cnt_uplink_iq) {
            rs = g_hal_state_pd->qos_uplink_iq_idxr()->alloc(
                (uint32_t*)&pd_qos_class->uplink.iq);
            if (rs != indexer::SUCCESS) {
                return HAL_RET_NO_RESOURCE;
            }

            pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX] =
                CAPRI_TM_P4_UPLINK_IQ_OFFSET + pd_qos_class->uplink.iq;
            if (pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX] == CAPRI_TM_P4_SPAN_QUEUE) {
                pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_RX] = CAPRI_TM_P4_EG_UPLINK_SPAN_QUEUE_REPLACEMENT;
            } else if (pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX] == CAPRI_TM_P4_CPU_COPY_QUEUE) {
                pd_qos_class->p4_eg_q[HAL_PD_QOS_IQ_RX] = CAPRI_TM_P4_EG_UPLINK_CPU_COPY_QUEUE_REPLACEMENT;
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
 
    // Update dscp/pcp to tx-iq map for user-defined classes.
    if (qos_is_user_defined_class(qos_group)) {
       if (qos_class->cmap.type == QOS_CMAP_TYPE_DSCP) {
           for (i = 0; i < HAL_MAX_IP_DSCP_VALS; i++) {
               if (qos_class->cmap.ip_dscp[i]) {
                   dscp_cos_txdma_iq_map[0].is_dscp = true; // use default-cos dscp as global dscp/pcp config
                   dscp_cos_txdma_iq_map[i].is_dscp = true;
                   dscp_cos_txdma_iq_map[i].txdma_iq = pd_qos_class->txdma[0].iq;
                   dscp_cos_txdma_iq_map[i].no_drop  = qos_class->no_drop;

               }
           }
       } else {
           for (i = 0; i < HAL_MAX_DOT1Q_PCP_VALS; i++) {
               if (i == qos_class->cmap.dot1q_pcp) {
                   dscp_cos_txdma_iq_map[0].is_dscp = false;
                   dscp_cos_txdma_iq_map[i].is_dscp = false;
                   dscp_cos_txdma_iq_map[i].txdma_iq = pd_qos_class->txdma[0].iq; // use first queue for now.
                   dscp_cos_txdma_iq_map[i].no_drop  = qos_class->no_drop;
               }
           }
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
            pd_qos_class->dest_oq += CAPRI_TM_RXDMA_OQ_OFFSET;
        }
    }

    pd_qos_class->dest_oq_type = q_alloc_params.dest_oq_type;
    pd_qos_class->pcie_oq = q_alloc_params.pcie_oq;

    HAL_TRACE_DEBUG("Allocated pd qos class {}", *pd_qos_class);

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Program HBM table for DSCP/PCP to txdma-iq mapping
// ----------------------------------------------------------------------------

static hal_ret_t
qos_class_pd_program_dscp_cos_map_table ()
{
    hal_ret_t      ret = HAL_RET_OK;
        
    uint64_t       dscp_cos_map_hbm_base_addr;
    dscp_cos_map_hbm_base_addr =  (uint64_t)get_mem_addr(CAPRI_HBM_REG_QOS_DSCP_COS_MAP);
        
    HAL_TRACE_DEBUG("Programming DSCP-PCP to TxDMA IQ mapping "
                    "dscp_cos_map_hbm_base_addr is {} and size of dscp-cos-map-table is {}", 
                        dscp_cos_map_hbm_base_addr, sizeof(dscp_cos_txdma_iq_map));
    
    p4plus_hbm_write(dscp_cos_map_hbm_base_addr, (uint8_t *)&dscp_cos_txdma_iq_map, sizeof(dscp_cos_txdma_iq_map),
            P4PLUS_CACHE_INVALIDATE_BOTH);

    return ret;
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
    qos_group_t             qos_group;
    qos_class_t             *qos_class = pd_qos_class->pi_qos_class;
    uint32_t                i;
    hal_ret_t               ret = HAL_RET_OK;
    
    qos_group = qos_class_get_qos_group(qos_class);

    // Reset dscp/pcp to tx-iq map for user-defined classes.
    if (qos_is_user_defined_class(qos_group)) {
       if (qos_class->cmap.type == QOS_CMAP_TYPE_DSCP) {
           for (i = 0; i < HAL_MAX_IP_DSCP_VALS; i++) {
               if (qos_class->cmap.ip_dscp[i]) {
                   memset(&dscp_cos_txdma_iq_map[i], 0, sizeof(pd_qos_dscp_cos_map_t));
               }
           }
       } else {
           for (i = 0; i < HAL_MAX_DOT1Q_PCP_VALS; i++) {
               if (i == qos_class->cmap.dot1q_pcp) {
                   memset(&dscp_cos_txdma_iq_map[i], 0, sizeof(pd_qos_dscp_cos_map_t));
               }
           }
       }
    }

    // Update HBM state about reset.
    ret = qos_class_pd_program_dscp_cos_map_table();
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error programming DSCP COS MAP table for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }                

    if (capri_tm_q_valid(pd_qos_class->uplink.iq)) {
        g_hal_state_pd->qos_uplink_iq_idxr()->free(pd_qos_class->uplink.iq);
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->txdma); i++) {
        if (capri_tm_q_valid(pd_qos_class->txdma[i].iq)) {
            g_hal_state_pd->qos_txdma_iq_idxr()->free(pd_qos_class->txdma[i].iq);
        }
    }

    if (capri_tm_q_valid(pd_qos_class->dest_oq)) {
        if (pd_qos_class->dest_oq_type == HAL_PD_QOS_OQ_COMMON) {
            g_hal_state_pd->qos_common_oq_idxr()->free(pd_qos_class->dest_oq);
        } else {
            SDK_ASSERT(pd_qos_class->dest_oq >= CAPRI_TM_RXDMA_OQ_OFFSET);
            g_hal_state_pd->qos_rxdma_oq_idxr()->free(
                pd_qos_class->dest_oq - CAPRI_TM_RXDMA_OQ_OFFSET);
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
    sdk_ret_t sdk_ret;
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
    sdk_ret = capri_tm_scheduler_map_update(port, TM_QUEUE_NODE_TYPE_LEVEL_1,
                                        oq, &q_node_params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_program_uplink_iq_params (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
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
    iq_params.xoff_threshold = qos_class->pause.xoff_threshold;
    iq_params.xon_threshold = qos_class->pause.xon_threshold;
    SDK_ASSERT(capri_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX]));
    iq_params.p4_q = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX];

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        sdk_ret = capri_tm_uplink_iq_no_drop_update(
                                    port, iq, qos_class->no_drop);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the iq no_drop for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
        sdk_ret = capri_tm_uplink_iq_params_update(port, iq, &iq_params);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the iq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

/**
 * @brief      return dot1q_pcp based on qos group
 * @param[in]  PI qos class representing the qos group
 * @return     dot1q_pcp value
 */
static uint8_t
qos_class_group_get_dot1q_pcp (qos_class_t *qos_class)
{
    qos_group_t qos_group = qos_class_get_qos_group(qos_class);

    switch(qos_group) {
        case QOS_GROUP_USER_DEFINED_1:
            return 1;

        case QOS_GROUP_USER_DEFINED_2:
            return 2;

        case QOS_GROUP_USER_DEFINED_3:
            return 3;

        case QOS_GROUP_USER_DEFINED_4:
            return 4;

        case QOS_GROUP_USER_DEFINED_5:
            return 5;

        case QOS_GROUP_USER_DEFINED_6:
            return 6;

        case QOS_GROUP_DEFAULT:
        default:
            return 0;
    }

    return 0;
}

static hal_ret_t
qos_class_pd_program_uplink_iq_map (pd_qos_class_t *pd_qos_class)
{
    // Program the dscp, dot1q to iq map
    hal_ret_t                  ret = HAL_RET_OK;
    sdk_ret_t                  sdk_ret;
    tm_port_t                  port;
    qos_class_t                *qos_class = pd_qos_class->pi_qos_class;
    bool                       has_pcp = false;
    bool                       has_dscp = false;
    tm_q_t                     iq;
    uint32_t                   dot1q_pcp = 0;
    tm_uplink_input_dscp_map_t dscp_map = {0};

    iq = pd_qos_class->uplink.iq;
    if (!capri_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }
    has_pcp = cmap_type_pcp(qos_class->cmap.type);
    has_dscp = cmap_type_dscp(qos_class->cmap.type);

    SDK_ASSERT(sizeof(qos_class->cmap.ip_dscp) == sizeof(dscp_map.ip_dscp));

    memcpy(dscp_map.ip_dscp, qos_class->cmap.ip_dscp,
               sizeof(qos_class->cmap.ip_dscp));
    if (has_pcp) {
        dot1q_pcp = qos_class->cmap.dot1q_pcp;
    } else {
        dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
    }
    dscp_map.dot1q_pcp = dot1q_pcp;
    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        sdk_ret = capri_tm_uplink_input_map_update(
                                               port,
                                               dot1q_pcp,
                                               iq);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the uplink map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
        if (has_dscp) {
            sdk_ret = capri_tm_uplink_input_dscp_map_update(port, &dscp_map);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming the uplink dscp map for "
                              "Qos-class {} on port {} ret {}",
                              qos_class->key, port, ret);
                return ret;
            }
        }
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_update_uplink_iq_map_remove (bool dot1q_remove, uint32_t dot1q_pcp,
                                          bool *ip_dscp_vals, uint32_t cnt_ip_dscp)
{
    hal_ret_t                  ret = HAL_RET_OK;
    sdk_ret_t                  sdk_ret;
    tm_port_t                  port;
    qos_class_t                *default_qos_class = NULL;
    tm_uplink_input_dscp_map_t dscp_map = {0};
    uint32_t                   default_qos_class_dot1q_pcp = 0;
    tm_q_t                     default_qos_class_iq = 0;

    default_qos_class = find_qos_class_by_group(QOS_GROUP_DEFAULT);
    if (!default_qos_class) {
        HAL_TRACE_ERR("Default qos class is not created");
    } else {
        default_qos_class_dot1q_pcp = default_qos_class->cmap.dot1q_pcp;
        default_qos_class_iq = default_qos_class->pd->uplink.iq;
    }

    SDK_ASSERT(cnt_ip_dscp == SDK_ARRAY_SIZE(dscp_map.ip_dscp));
    memcpy(dscp_map.ip_dscp, ip_dscp_vals, sizeof(dscp_map.ip_dscp));
    dscp_map.dot1q_pcp = default_qos_class_dot1q_pcp;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        if (dot1q_remove) {
            sdk_ret = capri_tm_uplink_input_map_update(port,
                                                   dot1q_pcp,
                                                   default_qos_class_iq);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming the uplink map "
                              "on port {} ret {}",
                              port, ret);
                return ret;
            }
        }

        sdk_ret = capri_tm_uplink_input_dscp_map_update(port, &dscp_map);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
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
    sdk_ret_t             sdk_ret;
    tm_port_t             port;
    bool                  reset_pfc_xoff = false;    // reset current PFC xoff
    bool                  reset_all_xoff = false;    // reset all xoff
    bool                  set_pfc_xoff = false;      // set xoff for PFC
    bool                  set_all_xoff = false;      // set all xoff
    qos_class_t           *qos_class = pd_qos_class->pi_qos_class;

    if (!capri_tm_q_valid(pd_qos_class->dest_oq) ||
        (pd_qos_class->dest_oq_type != HAL_PD_QOS_OQ_COMMON)) {
        return HAL_RET_OK;
    }

    // no_drop = true (pause enabled):
    //      Incoming config: PFC
    //      set xoff for pfc_cos (react only to PFC frames with pfc_cos)
    //
    // no_drop = false:
    //      Incoming config: no pause
    //      reset xoff for pfc_cos (dont react to PFC frames with pfc_cos)

    if (qos_class->no_drop == true) {
        if (qos_class->pause.pfc_enable == true) {
            set_pfc_xoff = true;
        }
    } else {
        reset_pfc_xoff = true;
    }

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        sdk_ret = capri_tm_uplink_oq_update(port,
                                            pd_qos_class->dest_oq,
                                            qos_class->pause.pfc_cos);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming xoff2oq map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
        sdk_ret = capri_tm_set_uplink_mac_xoff(port,
                                               reset_all_xoff,
                                               set_all_xoff,
                                               reset_pfc_xoff,
                                               set_pfc_xoff,
                                               (1 << qos_class->pause.pfc_cos));
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming mac xoff for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

hal_ret_t
qos_class_pd_program_scheduler (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    tm_port_t            port;
    qos_class_t          *qos_class = pd_qos_class->pi_qos_class;
    tm_q_t               oq;

    port = TM_PORT_INGRESS;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->p4_ig_q); i++) {
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
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->p4_eg_q); i++) {
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

#define QOS_ACTION(_arg) d.action_u.qos_qos._arg
static hal_ret_t
qos_class_pd_program_qos_table (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t      ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    qos_class_t    *qos_class = pd_qos_class->pi_qos_class;
    directmap      *qos_tbl = NULL;
    qos_actiondata_t d;
    uint32_t       qos_class_id;

    qos_tbl = g_hal_state_pd->dm_table(P4TBL_ID_QOS);
    SDK_ASSERT_RETURN(qos_tbl != NULL, HAL_RET_ERR);

    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->p4_ig_q); i++) {
        if (!capri_tm_q_valid(pd_qos_class->p4_ig_q[i])) {
            continue;
        }
        qos_class_id = pd_qos_class->p4_ig_q[i];

        memset(&d, 0, sizeof(d));
        QOS_ACTION(egress_tm_oq) = pd_qos_class->p4_eg_q[i];
        SDK_ASSERT(capri_tm_q_valid(pd_qos_class->p4_eg_q[i]));
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

static hal_ret_t
qos_class_pd_sched_pgm_oq (tm_port_t port,
                           tm_q_t    oq,
                           qos_class_t *qos_class)
{
    hal_ret_t ret     = HAL_RET_OK;
    sdk_ret_t sdk_ret = SDK_RET_OK;
    pb_sched_node_input_info_t input_info;

    if (!capri_tm_q_valid(oq)) {
        return HAL_RET_OK;
    }

    memset(&input_info, 0, sizeof(pb_sched_node_input_info_t));

    switch(qos_class->sched.type) {
    case QOS_SCHED_TYPE_DWRR:
        input_info.weight = qos_class->sched.dwrr.bw;
        break;

    default:
    case QOS_SCHED_TYPE_STRICT:
        input_info.is_strict = true;
        HAL_TRACE_DEBUG("sp_rate_mbps: {}", qos_class->sched.strict.bps);
        input_info.sp_rate_mbps = qos_class->sched.strict.bps;
        sdk_ret = cap_pb_sched_spq_pgm (0, 0, port, oq, &input_info);

        // TODO MBT: revert after fixing delete of scheduler programming
        sdk_ret = SDK_RET_OK;

        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        break;
    }

    return ret;
}

static hal_ret_t
qos_class_pd_sched_pgm (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    tm_port_t            port;
    qos_class_t          *qos_class = pd_qos_class->pi_qos_class;
    tm_q_t               oq;

    port = TM_PORT_INGRESS;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->p4_ig_q); i++) {
        oq = pd_qos_class->p4_ig_q[i];
        ret = qos_class_pd_sched_pgm_oq(port, oq, qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the oq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }

    port = TM_PORT_EGRESS;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->p4_eg_q); i++) {
        oq = pd_qos_class->p4_eg_q[i];
        ret = qos_class_pd_sched_pgm_oq(port, oq, qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the oq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }

    // On the DMA port
    for (port = TM_DMA_PORT_BEGIN; port <= TM_DMA_PORT_END; port++) {
        ret = qos_class_pd_sched_pgm_oq(port, pd_qos_class->dest_oq, qos_class);
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
            ret = qos_class_pd_sched_pgm_oq(port, pd_qos_class->dest_oq, qos_class);
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

    ret = qos_class_pd_sched_pgm(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error programming the scheduler for "
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
    ret = qos_class_pd_program_dscp_cos_map_table();

    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error programming DSCP COS MAP table for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }
    return HAL_RET_OK;
}

static hal_ret_t
qos_class_pd_deprogram_uplink_iq_map (pd_qos_class_t *pd_qos_class)
{
    // Program the dscp, dot1q to iq map
    hal_ret_t                  ret = HAL_RET_OK;
    sdk_ret_t                  sdk_ret;
    tm_port_t                  port;
    qos_class_t                *qos_class = pd_qos_class->pi_qos_class;
    bool                       has_pcp = false;
    bool                       has_dscp = false;
    tm_q_t                     iq;
    uint32_t                   dot1q_pcp = 0;
    tm_uplink_input_dscp_map_t dscp_map = {0};

    // reset the default Q to 0
    iq = QOS_QUEUE_DEFAULT;
    if (!capri_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }

    has_pcp = cmap_type_pcp(qos_class->cmap.type);
    has_dscp = cmap_type_dscp(qos_class->cmap.type);

    SDK_ASSERT(sizeof(qos_class->cmap.ip_dscp) ==
               sizeof(dscp_map.ip_dscp));
    memcpy(dscp_map.ip_dscp, qos_class->cmap.ip_dscp,
           sizeof(qos_class->cmap.ip_dscp));

    if (has_pcp) {
        dot1q_pcp = qos_class->cmap.dot1q_pcp;
    } else {
        dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
    }

    // program the default COS for dscp map
    dscp_map.dot1q_pcp = QOS_COS_DEFAULT;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        sdk_ret = capri_tm_uplink_input_map_update(
                                               port,
                                               dot1q_pcp,
                                               iq);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error deprogramming the uplink map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }

        if (has_dscp) {
            sdk_ret = capri_tm_uplink_input_dscp_map_update(port, &dscp_map);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error deprogramming the uplink dscp map for "
                              "Qos-class {} on port {} ret {}",
                              qos_class->key, port, ret);
                return ret;
            }
        }
    }

    return HAL_RET_OK;
}

#define QOS_ACTION(_arg) d.action_u.qos_qos._arg
static hal_ret_t
qos_class_pd_deprogram_qos_table (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t      ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    qos_class_t    *qos_class = pd_qos_class->pi_qos_class;
    directmap      *qos_tbl = NULL;
    qos_actiondata_t d = {0};
    uint32_t       qos_class_id;

    qos_tbl = g_hal_state_pd->dm_table(P4TBL_ID_QOS);
    SDK_ASSERT_RETURN(qos_tbl != NULL, HAL_RET_ERR);

    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->p4_ig_q); i++) {
        if (!capri_tm_q_valid(pd_qos_class->p4_ig_q[i])) {
            continue;
        }
        qos_class_id = pd_qos_class->p4_ig_q[i];

        memset(&d, 0, sizeof(d));

        sdk_ret = qos_tbl->update(qos_class_id, &d);

        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("qos table write failure, qos-class {}, "
                          "qos-class-id {} ret {}",
                          qos_class->key, qos_class_id, ret);
            return ret;
        }

        HAL_TRACE_DEBUG("qos table qos-class {} qos-class-id {} deprogrammed ",
                        qos_class->key, qos_class_id);
    }
    return ret;
}
#undef QOS_ACTION

// ----------------------------------------------------------------------------
// DeProgram HW
// ----------------------------------------------------------------------------
static hal_ret_t
qos_class_pd_deprogram_hw (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t            ret = HAL_RET_OK;
    qos_class_t          *qos_class = pd_qos_class->pi_qos_class;

    // TODO no need to update the iq params since the packets wont
    // arrive in that Q?
#if 0
    ret = qos_class_pd_program_uplink_iq_params(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error deprogramming uplink iq params for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }
#endif

    ret = qos_class_pd_deprogram_uplink_iq_map(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error deprogramming uplink iq map for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

#if 0
    ret = qos_class_pd_program_uplink_xoff(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error deprogramming uplink oq xoff for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    ret = qos_class_pd_program_scheduler(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error deprogramming the p4 ports for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }
#endif

    ret = qos_class_pd_deprogram_qos_table(pd_qos_class);
    if (ret != HAL_RET_OK) {
        // TODO: What to do in case of hw programming error ?
        HAL_TRACE_ERR("Error deprogramming the qos table for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

    return HAL_RET_OK;
}

static void
qos_class_pd_get_all_queues (pd_qos_class_t *qos_class_pd,
                             capri_queue_t iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS],
                             capri_queue_t oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS])
{
    uint32_t iq_idx = 0;
    uint32_t oq_idx = 0;
    uint32_t port;
    capri_queue_t *input_queue;
    capri_queue_t *output_queue;
    uint32_t iq_cnt = CAPRI_TM_MAX_IQS;
    uint32_t oq_cnt = CAPRI_TM_MAX_OQS;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        iq_idx = 0;
        oq_idx = 0;
        if (capri_tm_q_valid(qos_class_pd->uplink.iq)) {
            SDK_ASSERT(iq_idx < iq_cnt);
            input_queue = &iqs[port][iq_idx++];
            input_queue->valid = true;
            input_queue->queue = qos_class_pd->uplink.iq;
        }

        if (capri_tm_q_valid(qos_class_pd->dest_oq)) {
            SDK_ASSERT(oq_idx < oq_cnt);
            output_queue = &oqs[port][oq_idx++];
            output_queue->valid = true;
            output_queue->queue = qos_class_pd->dest_oq;
        }
    }
    for (port = TM_DMA_PORT_BEGIN; port <= TM_DMA_PORT_END; port++) {
        iq_idx = 0;
        oq_idx = 0;
        for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class_pd->txdma); i++) {
            if (capri_tm_q_valid(qos_class_pd->txdma[i].iq)) {
                SDK_ASSERT(iq_idx < iq_cnt);
                input_queue = &iqs[port][iq_idx++];
                input_queue->valid = true;
                input_queue->queue = qos_class_pd->txdma[i].iq;
            }
        }

        if (capri_tm_q_valid(qos_class_pd->dest_oq)) {
            SDK_ASSERT(oq_idx < oq_cnt);
            output_queue = &oqs[port][oq_idx++];
            output_queue->valid = true;
            output_queue->queue = qos_class_pd->dest_oq;
        }
    }

    // P4 IG
    port = TM_PORT_INGRESS;
    iq_idx = 0;
    oq_idx = 0;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class_pd->p4_ig_q); i++) {
        if (capri_tm_q_valid(qos_class_pd->p4_ig_q[i])) {
            SDK_ASSERT(iq_idx < iq_cnt);
            input_queue = &iqs[port][iq_idx++];
            input_queue->valid = true;
            input_queue->queue = qos_class_pd->p4_ig_q[i];

            SDK_ASSERT(oq_idx < oq_cnt);
            output_queue = &oqs[port][oq_idx++];
            output_queue->valid = true;
            output_queue->queue = qos_class_pd->p4_ig_q[i];
        }
    }

    // P4 EG
    port = TM_PORT_EGRESS;
    iq_idx = 0;
    oq_idx = 0;
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class_pd->p4_eg_q); i++) {
        if (capri_tm_q_valid(qos_class_pd->p4_eg_q[i])) {
            SDK_ASSERT(iq_idx < iq_cnt);
            input_queue = &iqs[port][iq_idx++];
            input_queue->valid = true;
            input_queue->queue = qos_class_pd->p4_eg_q[i];

            SDK_ASSERT(oq_idx < oq_cnt);
            output_queue = &oqs[port][oq_idx++];
            output_queue->valid = true;
            output_queue->queue = qos_class_pd->p4_eg_q[i];
        }
    }
}

static void
qos_class_pd_reset_stats (pd_qos_class_t *qos_class_pd)
{
    capri_queue_t iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS] = {};
    capri_queue_t oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS] = {};

    qos_class_pd_get_all_queues(qos_class_pd,
                                iqs, oqs);

    for (unsigned port = 0; port < SDK_ARRAY_SIZE(iqs); port++) {
        for (unsigned i = 0; i < SDK_ARRAY_SIZE(iqs[0]); i++) {
            if (iqs[port][i].valid) {
                capri_tm_reset_iq_stats(port, iqs[port][i].queue);
            }
        }
    }
}

// walk through all the user-defined no-drop classes and form 
// the bitmap of all the pfc-cos values
uint32_t
pd_qos_class_get_all_tc_pfc_cos_bitmap()
{
    qos_class_t *qos_class = NULL;
    qos_group_t qos_group;
    uint32_t    pfc_cos_bitmap = 0;

    for (qos_group = QOS_GROUP_USER_DEFINED_1; 
         qos_group <= QOS_GROUP_USER_DEFINED_6; 
         qos_group = qos_group_get_next_user_defined(qos_group)) {

        qos_class = find_qos_class_by_group(qos_group);

        if(!qos_class)
            continue;

        if(qos_class->no_drop) {
            pfc_cos_bitmap |= (1 << qos_class->pause.pfc_cos);
        }
    }

    HAL_TRACE_DEBUG("pfc_cos_bitmap for all TCs {}", pfc_cos_bitmap);

    return pfc_cos_bitmap;

}

// ----------------------------------------------------------------------------
// Qos-class set global pause type
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_set_global_pause_type (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sdk_ret = SDK_RET_OK;
    tm_port_t tm_port = TM_UPLINK_PORT_BEGIN;
    bool reset_all_xoff = false;
    bool set_all_xoff = false;
    bool reset_pfc_xoff = false;    // unused in this method
    bool set_pfc_xoff = false;      // used if pause-type = PFC & no-drop TCs present
    uint32_t pfc_cos_bitmap = 0x0;  // bitmap of PFC COS values for all TCs
    pd_qos_class_set_global_pause_type_args_t *args =
                            pd_func_args->pd_qos_class_set_global_pause_type;

    HAL_TRACE_DEBUG("setting global pause type {}", args->pause_type);
    switch (args->pause_type) {
    case hal::QOS_PAUSE_TYPE_LINK_LEVEL:
        set_all_xoff = true;
        break;

    default:
        reset_all_xoff = true;
        break;
    }

    if(reset_all_xoff == true) {
        // not link-level pause
        pfc_cos_bitmap = pd_qos_class_get_all_tc_pfc_cos_bitmap();
        if((pfc_cos_bitmap != 0) && (args->pause_type != hal::QOS_PAUSE_TYPE_NONE)) {
            // pause-type = PFC; other no-drop TCs present with pfc-cos
            set_pfc_xoff = true;
        }
    }

    for (tm_port = TM_UPLINK_PORT_BEGIN;
                        tm_port <= TM_UPLINK_PORT_END; tm_port++) {
        sdk_ret = capri_tm_set_uplink_mac_xoff(tm_port,
                                               reset_all_xoff,
                                               set_all_xoff,
                                               reset_pfc_xoff,
                                               set_pfc_xoff,
                                               pfc_cos_bitmap);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error setting global pause type {} "
                          "tm_port {} ret {}",
                          args->pause_type, tm_port, ret);
            break;
        }
    }
    return ret;
}

hal_ret_t
pd_qos_class_defaults_set (qos_class_t *qos_class)
{
    if (qos_class->mtu == 0) {
        qos_class->mtu = HAL_JUMBO_MTU;
    }
    if (qos_class->pause.xon_threshold == 0) {
        qos_class->pause.xon_threshold = CAPRI_TM_DEFAULT_XON_THRESHOLD;
    }
    if (qos_class->pause.xoff_threshold == 0) {
        qos_class->pause.xoff_threshold = CAPRI_TM_DEFAULT_XOFF_THRESHOLD;
    }
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Qos-class Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;;
    pd_qos_class_create_args_t *args = pd_func_args->pd_qos_class_create;
    pd_qos_class_t *pd_qos_class;

    HAL_TRACE_DEBUG("creating pd state for qos_class: {}",
                    args->qos_class->key);

    // Create qos_class PD
    pd_qos_class = qos_class_pd_alloc_init();
    if (pd_qos_class == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    pd_qos_class_defaults_set(args->qos_class);

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

    // Reset the stats
    qos_class_pd_reset_stats(pd_qos_class);

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
pd_qos_class_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_qos_class_update_args_t *args = pd_func_args->pd_qos_class_update;
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
        ret = qos_class_pd_update_uplink_iq_map_remove(
                                        args->dot1q_pcp_changed,
                                        args->dot1q_pcp_remove,
                                        args->ip_dscp_remove,
                                        SDK_ARRAY_SIZE(args->ip_dscp_remove));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error removing uplink iq map for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

    if (args->pfc_cos_changed) {
        ret = qos_class_pd_program_uplink_xoff(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming uplink oq xoff for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }

#if 0
    if (args->scheduler_changed) {
        ret = qos_class_pd_program_scheduler(pd_qos_class);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the p4 ports for "
                          "Qos-class {} ret {}",
                          qos_class->key, ret);
            return ret;
        }
    }
#endif

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
pd_qos_class_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret;
    pd_qos_class_delete_args_t *args = pd_func_args->pd_qos_class_delete;
    pd_qos_class_t *qos_class_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->qos_class != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->qos_class->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("deleting pd state for qos_class {}",
                    args->qos_class->key);
    qos_class_pd = (pd_qos_class_t *)args->qos_class->pd;

    // TODO: deprogram hw
    qos_class_pd_deprogram_hw(qos_class_pd);

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
pd_qos_class_make_clone (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_qos_class_make_clone_args_t *args = pd_func_args->pd_qos_class_make_clone;
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

static void
qos_class_pd_populate_status (pd_qos_class_t *qos_class_pd, QosClassStatusEpd *epd_status)
{
    capri_queue_t iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS] = {};
    capri_queue_t oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS] = {};

    qos_class_pd_get_all_queues(qos_class_pd,
                                iqs, oqs);

    for (unsigned port = 0; port < SDK_ARRAY_SIZE(iqs); port++) {
        auto port_status = epd_status->add_port_status();
        qos_class_pd_port_to_packet_buffer_port(port,
                                                port_status->mutable_packet_buffer_port());
        for (unsigned i = 0; i < SDK_ARRAY_SIZE(iqs[0]); i++) {
            if (iqs[port][i].valid) {
                port_status->add_input_queues(iqs[port][i].queue);
            }
        }
        for (unsigned i = 0; i < SDK_ARRAY_SIZE(oqs[0]); i++) {
            if (oqs[port][i].valid) {
                port_status->add_output_queues(oqs[port][i].queue);
            }
        }
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(qos_class_pd->txdma); i++) {
        if (capri_tm_q_valid(qos_class_pd->txdma[i].iq)) {
            epd_status->add_tx_traffic_class_coses(qos_class_pd->txdma[i].iq);
        }
    }
}

void
qos_class_queue_stats_to_proto_stats (qos::QosClassQueueStats *q_stats,
                                      sdk::platform::capri::capri_queue_stats_t *qos_queue_stats)
{
    for (unsigned i = 0; i < CAPRI_TM_MAX_IQS; i++) {
        if (qos_queue_stats->iq_stats[i].iq.valid) {
            auto input_stats = q_stats->add_input_queue_stats();
            auto iq_stats = &qos_queue_stats->iq_stats[i].stats;
            input_stats->set_input_queue_idx(qos_queue_stats->iq_stats[i].iq.queue);
            input_stats->mutable_oflow_fifo_stats()->set_good_pkts_in(iq_stats->oflow.good_pkts_in);
            input_stats->mutable_oflow_fifo_stats()->set_good_pkts_out(iq_stats->oflow.good_pkts_out);
            input_stats->mutable_oflow_fifo_stats()->set_errored_pkts_in(iq_stats->oflow.errored_pkts_in);
            input_stats->mutable_oflow_fifo_stats()->set_fifo_depth(iq_stats->oflow.fifo_depth);
            input_stats->mutable_oflow_fifo_stats()->set_max_fifo_depth(iq_stats->oflow.max_fifo_depth);
            input_stats->set_buffer_occupancy(iq_stats->buffer_occupancy);
            input_stats->set_peak_occupancy(iq_stats->peak_occupancy);
            input_stats->set_port_monitor(iq_stats->port_monitor);
        }
        if (qos_queue_stats->oq_stats[i].oq.valid) {
            auto output_stats = q_stats->add_output_queue_stats();
            auto oq_stats = &qos_queue_stats->oq_stats[i].stats;
            output_stats->set_output_queue_idx(qos_queue_stats->oq_stats[i].oq.queue);
            output_stats->set_queue_depth(oq_stats->queue_depth);
            output_stats->set_port_monitor(oq_stats->port_monitor);
        }
    }
}

static hal_ret_t
qos_class_pd_populate_stats (pd_qos_class_t *qos_class_pd, QosClassStats *stats)
{
    hal_ret_t                first_err_ret = HAL_RET_OK;
    hal_ret_t                ret = HAL_RET_OK;
    sdk_ret_t                sdk_ret = SDK_RET_OK;
    capri_queue_t            iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS] = {};
    capri_queue_t            oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS] = {};
    capri_queue_stats_t      queue_stats = {0};

    qos_class_pd_get_all_queues(qos_class_pd,
                                iqs, oqs);

    // Update the stats now
    pd_qos_class_periodic_stats_update(NULL);

    for (unsigned port = 0; port < SDK_ARRAY_SIZE(iqs); port++) {
        auto port_stats = stats->add_port_stats();
        qos_class_pd_port_to_packet_buffer_port(port,
                                                port_stats->mutable_packet_buffer_port());
        auto q_stats = port_stats->mutable_qos_queue_stats();
        sdk_ret = capri_populate_queue_stats(
                        port, iqs[port], oqs[port], &queue_stats);
        qos_class_queue_stats_to_proto_stats(q_stats, &queue_stats);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (first_err_ret == HAL_RET_OK) {
            first_err_ret = ret;
        }
    }
    return first_err_ret;
}

// ----------------------------------------------------------------------------
// pd qos_class get
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_qos_class_get_args_t *args = pd_func_args->pd_qos_class_get;
    qos_class_t         *qos_class = args->qos_class;
    pd_qos_class_t      *qos_class_pd = (pd_qos_class_t *)qos_class->pd;
    QosClassGetResponse *rsp = args->rsp;

    qos_class_pd_populate_status(qos_class_pd, rsp->mutable_status()->mutable_epd_status());
    qos_class_pd_populate_stats(qos_class_pd, rsp->mutable_stats());

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
pd_qos_class_restore (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret;
    pd_qos_class_restore_args_t *args = pd_func_args->pd_qos_class_restore;
    pd_qos_class_t *qos_class_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
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
pd_qos_class_mem_free (pd_func_args_t *pd_func_args)
{
    pd_qos_class_t        *pd_qos_class;
    pd_qos_class_mem_free_args_t *args = pd_func_args->pd_qos_class_mem_free;

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
pd_qos_class_get_qos_class_id (pd_func_args_t *pd_func_args)
{
    pd_qos_class_t *pd_qos_class;
    pd_qos_class_get_qos_class_id_args_t *q_args = pd_func_args->pd_qos_class_get_qos_class_id;
    pd_func_args_t pd_func_args1 = {0};
    tm_port_t dest_port = 0;
    uint32_t group = 0;
    pd_if_get_tm_oport_args_t args;

    qos_class_t *qos_class = q_args->qos_class;
    if_t *dest_if = q_args->dest_if;
    uint32_t *qos_class_id = q_args->qos_class_id;

    if (!qos_class) {
        return HAL_RET_QOS_CLASS_NOT_FOUND;
    }

    if (dest_if) {
        HAL_TRACE_VERBOSE("dest_if if_type {} if_id {}",
                          dest_if->if_type, dest_if->if_id);
        if (dest_if->if_id == 132) {
            HAL_TRACE_ERR("");
        }
        args.pi_if = dest_if;
        pd_func_args1.pd_if_get_tm_oport = &args;
        pd_if_get_tm_oport(&pd_func_args1);
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

    *qos_class_id = CAPRI_TM_INVALID_Q;
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
pd_qos_class_get_admin_cos (pd_func_args_t *pd_func_args)
{
    pd_qos_class_get_admin_cos_args_t *args = pd_func_args->pd_qos_class_get_admin_cos;
    args->cos = HAL_QOS_ADMIN_COS;
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// pd qos_class periodic_stats_update
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_periodic_stats_update (pd_func_args_t *pd_func_args)
{
    sdk_ret_t sdk_ret = capri_tm_periodic_stats_update();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

}    // namespace pd
}    // namespace hal
