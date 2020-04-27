// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
#include "nic/sdk/asic/port.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/aclqos/qos_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/port.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/sdk/platform/elba/elba_tm_rw.hpp"
#include "nic/sdk/platform/elba/elba_tm_utils.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/plugins/cfg/aclqos/qos_api.hpp"

using namespace sdk::platform::elba;
using qos::QosClassStatusEpd;

/* TBD-ELBA-REBASE: fixup */
#define ELBA_TM_PORT_UPLINK_7   7
#define TM_PORT_UPLINK_7        ELBA_TM_PORT_UPLINK_7

namespace hal {
namespace pd {

//TODO: Move this to HAL slabs.
pd_qos_dscp_cos_map_t dscp_cos_txdma_iq_map;

uint32_t g_qos_iq_to_tc[CAPRI_TM_MAX_IQS];
uint32_t g_qos_oq_to_tc[CAPRI_TM_MAX_OQS];

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

static void
qos_class_populate_dscp_cos_txdma_iq_map(pd_qos_class_t *pd_qos_class, uint32_t dscp_or_pcp, bool is_no_drop)
{
    uint32_t    i = dscp_or_pcp;
    uint8_t     no_drop_index = 0;

    if ((i % 2) == 1) {
         dscp_cos_txdma_iq_map.txdma_iq[i / 2] &= 0xf0;
         dscp_cos_txdma_iq_map.txdma_iq[i / 2] |= pd_qos_class->txdma[0].iq;
    } else {
         dscp_cos_txdma_iq_map.txdma_iq[i / 2] &= 0x0f;
         dscp_cos_txdma_iq_map.txdma_iq[i / 2] |= (pd_qos_class->txdma[0].iq << 4);
    }
    if (is_no_drop) {
         if (dscp_cos_txdma_iq_map.no_drop1_txdma_iq == 0) {
             dscp_cos_txdma_iq_map.no_drop1_txdma_iq = pd_qos_class->txdma[1].iq;
             no_drop_index = 1;
         } else if (dscp_cos_txdma_iq_map.no_drop2_txdma_iq == 0) {
             dscp_cos_txdma_iq_map.no_drop2_txdma_iq = pd_qos_class->txdma[1].iq;
             no_drop_index = 2;
         } else {
             dscp_cos_txdma_iq_map.no_drop3_txdma_iq = pd_qos_class->txdma[1].iq;
             no_drop_index = 3;
         }

         if ((i % 4) == 0) {
             dscp_cos_txdma_iq_map.no_drop[i / 4] |= (no_drop_index << 6);
         } else if ((i % 4) == 1) {
             dscp_cos_txdma_iq_map.no_drop[i / 4] |= (no_drop_index << 4);
         } else if ((i % 4) == 2) {
             dscp_cos_txdma_iq_map.no_drop[i / 4] |= (no_drop_index << 2);
         } else {
             dscp_cos_txdma_iq_map.no_drop[i / 4] |= no_drop_index;
         }
    }

    HAL_TRACE_DEBUG("Programming DSCP-PCP to TxDMA IQ mapping "
                    "for is_dscp{}, dscp/pcp {}, no_drop {}, txdma-iq1 {}, txdma-iq2 {} "
                    "txdma_iq_map.txdma_iq1 {}, no_drop_index {}, txdma_iq_map.no_drop {}",
                     dscp_cos_txdma_iq_map.is_dscp, dscp_or_pcp, is_no_drop,
                     pd_qos_class->txdma[0].iq, pd_qos_class->txdma[1].iq,
                     dscp_cos_txdma_iq_map.txdma_iq[i / 2], no_drop_index,
                     dscp_cos_txdma_iq_map.no_drop[i / 4]);
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
                   dscp_cos_txdma_iq_map.is_dscp = true;
                   qos_class_populate_dscp_cos_txdma_iq_map(pd_qos_class, i, qos_class->no_drop);
               }
           }
       } else {
           for (i = 0; i < HAL_MAX_DOT1Q_PCP_VALS; i++) {
               if (i == qos_class->cmap.dot1q_pcp) {
                   dscp_cos_txdma_iq_map.is_dscp = false;
                   qos_class_populate_dscp_cos_txdma_iq_map(pd_qos_class, i, qos_class->no_drop);
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
    dscp_cos_map_hbm_base_addr =  (uint64_t)asicpd_get_mem_addr(ASIC_HBM_REG_QOS_DSCP_COS_MAP);

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

static void
qos_class_depopulate_dscp_cos_txdma_iq_map(pd_qos_class_t *pd_qos_class, uint32_t dscp_or_pcp, bool is_no_drop)
{
    uint32_t    i = dscp_or_pcp;
    uint8_t     no_drop_index = 0;

    if ((i % 2) == 1) {
         dscp_cos_txdma_iq_map.txdma_iq[i / 2] &= 0xf0;
    } else {
         dscp_cos_txdma_iq_map.txdma_iq[i / 2] &= 0x0f;
    }
    if (is_no_drop) {
         if ((i % 4) == 0) {
             no_drop_index = (dscp_cos_txdma_iq_map.no_drop[i / 4] & 0xc0);
             no_drop_index >>= 6;
             dscp_cos_txdma_iq_map.no_drop[i / 4] &= 0x3f;
         } else if ((i % 4) == 1) {
             no_drop_index = (dscp_cos_txdma_iq_map.no_drop[i / 4] & 0x30);
             no_drop_index >>= 4;
             dscp_cos_txdma_iq_map.no_drop[i / 4] &= 0xcf;
         } else if ((i % 4) == 2) {
             no_drop_index = (dscp_cos_txdma_iq_map.no_drop[i / 4] & 0xc);
             no_drop_index >>= 2;
             dscp_cos_txdma_iq_map.no_drop[i / 4] &= 0xf3;
         } else {
             no_drop_index = (dscp_cos_txdma_iq_map.no_drop[i / 4] & 0x3);
             dscp_cos_txdma_iq_map.no_drop[i / 4] &= 0xfc;
         }
         if (no_drop_index == 1) {
             dscp_cos_txdma_iq_map.no_drop1_txdma_iq = 0;
         } else if (no_drop_index == 2) {
             dscp_cos_txdma_iq_map.no_drop2_txdma_iq = 0;
         } else if (no_drop_index == 3) {
             dscp_cos_txdma_iq_map.no_drop3_txdma_iq = 0;
         }
    }

    HAL_TRACE_DEBUG("De-Programming DSCP-PCP to TxDMA IQ mapping "
                    "for is_dscp{}, dscp/pcp {}, no_drop {}, txdma-iq1 {}, txdma-iq2 {} "
                    "txdma_iq_map.txdma_iq1 {}, no_drop_index {}, txdma_iq_map.no_drop {}",
                     dscp_cos_txdma_iq_map.is_dscp, dscp_or_pcp, is_no_drop,
                     pd_qos_class->txdma[0].iq, pd_qos_class->txdma[1].iq,
                     dscp_cos_txdma_iq_map.txdma_iq[i / 2], no_drop_index,
                     dscp_cos_txdma_iq_map.no_drop[i / 4]);
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
                   qos_class_depopulate_dscp_cos_txdma_iq_map(pd_qos_class, i, qos_class->no_drop);
               }
           }
       } else {
           for (i = 0; i < HAL_MAX_DOT1Q_PCP_VALS; i++) {
               if (i == qos_class->cmap.dot1q_pcp) {
                   qos_class_depopulate_dscp_cos_txdma_iq_map(pd_qos_class, i, qos_class->no_drop);
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

    if (elba_tm_q_valid(pd_qos_class->uplink.iq)) {
        g_hal_state_pd->qos_uplink_iq_idxr()->free(pd_qos_class->uplink.iq);
    }
    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pd_qos_class->txdma); i++) {
        if (elba_tm_q_valid(pd_qos_class->txdma[i].iq)) {
            g_hal_state_pd->qos_txdma_iq_idxr()->free(pd_qos_class->txdma[i].iq);
        }
    }

    if (elba_tm_q_valid(pd_qos_class->dest_oq)) {
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
    sdk::platform::elba::tm_queue_node_params_t q_node_params = {};
    tm_q_t parent_node = 0;

    if (!elba_tm_q_valid(oq)) {
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
    sdk_ret = elba_tm_scheduler_map_update(port, TM_QUEUE_NODE_TYPE_LEVEL_1,
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
#if 0
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    tm_port_t             port;
    qos_class_t           *qos_class = pd_qos_class->pi_qos_class;
    tm_q_t                iq;
    tm_uplink_q_params_t  q_params = { 0 };

    iq = pd_qos_class->uplink.iq;
    if (!elba_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }

    q_params.iq = iq;
    q_params.mtu = qos_class->mtu;
    q_params.xoff_threshold = qos_class->pause.xoff_threshold;
    q_params.xon_threshold = qos_class->pause.xon_threshold;
    SDK_ASSERT(elba_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX]));
    q_params.p4_q = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX];

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        sdk_ret = elba_tm_uplink_iq_no_drop_update(
                                    port, iq, qos_class->no_drop);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the iq no_drop for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
        sdk_ret = elba_tm_uplink_q_params_update(port, &q_params);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the iq params for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
    }
    return HAL_RET_OK;
#else
    return HAL_RET_OK;
#endif
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

    iq = pd_qos_class->uplink.iq;
    if (!elba_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }
    has_pcp = cmap_type_pcp(qos_class->cmap.type);
    has_dscp = cmap_type_dscp(qos_class->cmap.type);

    if (has_pcp) {
        dot1q_pcp = qos_class->cmap.dot1q_pcp;
    } else {
        dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
    }
    for (port = TM_UPLINK_PORT_BEGIN; port < TM_UPLINK_PORT_END; port++) {
        sdk_ret = elba_tm_uplink_input_map_update(port, dot1q_pcp, iq);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming the uplink map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }
        if (has_dscp) {
            sdk_ret = elba_tm_uplink_input_dscp_map_update(port, dot1q_pcp,
                                                           qos_class->cmap.ip_dscp);
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
    uint32_t                   default_qos_class_dot1q_pcp = 0;
    tm_q_t                     default_qos_class_iq = 0;

    default_qos_class = find_qos_class_by_group(QOS_GROUP_DEFAULT);
    if (!default_qos_class) {
        HAL_TRACE_ERR("Default qos class is not created");
    } else {
        default_qos_class_dot1q_pcp = default_qos_class->cmap.dot1q_pcp;
        default_qos_class_iq = default_qos_class->pd->uplink.iq;
    }

    for (port = TM_UPLINK_PORT_BEGIN; port < TM_UPLINK_PORT_END; port++) {
        if (dot1q_remove) {
            sdk_ret = elba_tm_uplink_input_map_update(port, dot1q_pcp,
                                                      default_qos_class_iq);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming the uplink map "
                              "on port {} ret {}",
                              port, ret);
                return ret;
            }
        }
        sdk_ret = elba_tm_uplink_input_dscp_map_update(port,
                                                       default_qos_class_dot1q_pcp,
                                                       ip_dscp_vals);
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
qos_class_pd_deprogram_uplink_xoff (pd_qos_class_t *pd_qos_class)
{
    hal_ret_t             ret = HAL_RET_OK;
    sdk_ret_t             sdk_ret;
    tm_port_t             port;
    bool                  reset_pfc_xoff = false;    // reset current PFC xoff
    bool                  reset_all_xoff = false;    // reset all xoff
    bool                  set_pfc_xoff = false;      // set xoff for PFC
    bool                  set_all_xoff = false;      // set all xoff
    qos_class_t           *qos_class = pd_qos_class->pi_qos_class;
    uint32_t              xoff_cos_bitmap;

    bool                  has_pcp = false;
    uint32_t              dot1q_pcp = 0;
    bool                  update_default_class_pfc = false;
    uint32_t              xoff_value=0;

    if (!elba_tm_q_valid(pd_qos_class->dest_oq) ||
        (pd_qos_class->dest_oq_type != HAL_PD_QOS_OQ_COMMON)) {
        return HAL_RET_OK;
    }

    if (qos_class->no_drop == true) {
        if (qos_class->pause.pfc_enable == true) {
            reset_pfc_xoff = true;
        }
    }

    if ( (reset_pfc_xoff == true) && (qos_class->pause.pfc_cos == 0) ) {
        /*
         * By default, in PFC mode, PCP0/PFC-COS0 is mapped to default-class (TC0).
         * Since, user has mapped PCP0/PFC-COS0 to a different user-defined class,
         * default class was remapped to first free PFC-COS value.
         * Now that the user-defined class is being deleted, map default class
         * back to its original value - PCP0/PFC-COS0.
         */
        has_pcp = cmap_type_pcp(qos_class->cmap.type);
        if (has_pcp) {
            dot1q_pcp = qos_class->cmap.dot1q_pcp;
        } else {
            dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
        }

        if (dot1q_pcp == qos_class->pause.pfc_cos) {
            update_default_class_pfc = true;
            // get what is currently programmed
            sdk_ret = elba_tm_get_uplink_oq_xoff_map(0, // port
                                                     0, // oq
                                                     &xoff_value);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error getting uplink_oq_xoff_map for "
                              "port 0 oq 0 ret {}", ret);
            }

            HAL_TRACE_DEBUG("resetting PFC COS {} and set 0 for default class", xoff_value);
        }
    }

    HAL_TRACE_DEBUG("reset xoff2oq and mac_xoff mapping for dest_oq {} "
        "no_drop {} pfc_enable {} pfc_cos {} reset_pfc_xoff {}",
        pd_qos_class->dest_oq, qos_class->no_drop, qos_class->pause.pfc_enable,
        qos_class->pause.pfc_cos, reset_pfc_xoff);

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        // deprogrm xoff2oq mapping - 1-to-1 by default for oqs 2 through 7
        sdk_ret = elba_tm_uplink_oq_update(port,
                                           pd_qos_class->dest_oq,
                                           pd_qos_class->dest_oq);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming xoff2oq map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }

        if(reset_pfc_xoff) {
            xoff_cos_bitmap = 0;
            // get what is currently programmed
            sdk_ret = elba_tm_get_uplink_mac_xoff(port, &xoff_cos_bitmap);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error getting mac xoff for port {} ret {}",
                              port, ret);
            } else {
                HAL_TRACE_DEBUG("mac xoff bitmap {} port {}",
                                xoff_cos_bitmap, port);
                if((xoff_cos_bitmap == 0xFF) || (qos_class->pause.pfc_cos == 0)) {
                    // if link-pause is set or if pfc-cos is 0
                    // (intended for default-class); do not reset PFC-COS.
                    reset_pfc_xoff = false;
                }
            }
        }

        sdk_ret = elba_tm_set_uplink_mac_xoff(port,
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

        if(update_default_class_pfc) {
            sdk_ret = elba_tm_uplink_oq_update(port,
                                               0,  // dest_oq of 0 for default class
                                               0); // pfc-cos
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming xoff2oq map for "
                              "Qos-class {} on port {} ret {}",
                              qos_class->key, port, ret);
            }

            sdk_ret = elba_tm_set_uplink_mac_xoff(port,
                                                  reset_all_xoff,
                                                  set_all_xoff,
                                                  true,    // reset pfc-xoff
                                                  set_pfc_xoff,
                                                  (1 << xoff_value));
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming mac xoff for "
                              "Qos-class {} on port {} ret {}",
                              qos_class->key, port, ret);
            }
        }  // if update_default_class_pfc

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
    uint32_t              xoff_cos_bitmap;

    bool                  has_pcp = false;
    uint32_t              dot1q_pcp = 0;
    bool                  update_default_class_pfc = false;
    int                   xoff_val=0;
    uint32_t              xoff_val_pgm=0;
    uint32_t              xoff_cos_bitmap0;

    if (!elba_tm_q_valid(pd_qos_class->dest_oq) ||
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

    if ( set_pfc_xoff == true) {
        /*
         * By default, in PFC mode, PCP0/PFC-COS0 is mapped to default-class (TC0).
         * Now, if user has mapped PCP0/PFC-COS0 to a different user-defined class,
         * re-map the default class to first free PFC-COS value. Actually,
         * default-class maps to all others PCPs/PFC-COS alues, but since we cannot
         * programme a bitmap and need to pick only 1 value - choosing the first one.
         */
        if (qos_class->pause.pfc_cos == 0) {
            has_pcp = cmap_type_pcp(qos_class->cmap.type);
            if (has_pcp) {
                dot1q_pcp = qos_class->cmap.dot1q_pcp;
            } else {
                dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
            }

            if (dot1q_pcp == qos_class->pause.pfc_cos) {
                update_default_class_pfc = true;
            }
        } else {
            /*
             * oq0 would have been mapped to some then unused PFC-COS;
             * Now if that PFC-COS value is used by some other user-defined class,
             * remap oq0 to a new unused PFC-COS value
             */
            sdk_ret = elba_tm_get_uplink_oq_xoff_map(0, // port
                                                     0, // oq
                                                     &xoff_val_pgm);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error getting uplink_oq_xoff_map for "
                              "port 0 oq 0 ret {}", ret);
            }

            if(xoff_val_pgm == qos_class->pause.pfc_cos) {
                update_default_class_pfc = true;
            }
        }

        if (update_default_class_pfc) {
            // get what is currently programmed
            sdk_ret = elba_tm_get_uplink_mac_xoff(0,
                                                  &xoff_cos_bitmap0);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error getting mac xoff for port 0 ret {}", ret);
            }
            HAL_TRACE_DEBUG("mac xoff bitmap {} for port 0", xoff_cos_bitmap0);
            // get the first free xoff cos value
            for(xoff_val = 0; xoff_val < 8; xoff_val++) {
                if(!(xoff_cos_bitmap0 & (1 << xoff_val))) {
                    HAL_TRACE_DEBUG("setting PFC COS {} for default class", xoff_val);
                    break;
                }
            }  // for xoff_val
            if(xoff_val == 8) {
                HAL_TRACE_DEBUG("couldnt find free PFC COS value {} for default class", xoff_cos_bitmap0);
                xoff_val = 0;
            }
        }
    }

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        sdk_ret = elba_tm_uplink_oq_update(port, pd_qos_class->dest_oq,
                                           qos_class->pause.pfc_cos);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming xoff2oq map for "
                          "Qos-class {} on port {} ret {}",
                          qos_class->key, port, ret);
            return ret;
        }

        if(reset_pfc_xoff) {
            xoff_cos_bitmap = 0;
            // get what is currently programmed
            sdk_ret = elba_tm_get_uplink_mac_xoff(port, &xoff_cos_bitmap);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error getting mac xoff for port {} ret {}",
                              port, ret);
            } else {
                HAL_TRACE_DEBUG("mac xoff bitmap {} port {}",
                                xoff_cos_bitmap, port);
                if((xoff_cos_bitmap == 0xFF) || (qos_class->pause.pfc_cos == 0)) {
                    // if link-pause is set or if pfc-cos is 0
                    // (intended for default-class); do not reset PFC-COS.
                    reset_pfc_xoff = false;
                }
            }
        }

        sdk_ret = elba_tm_set_uplink_mac_xoff(port,
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

        if(update_default_class_pfc) {
            sdk_ret = elba_tm_uplink_oq_update(port,
                                               0,  // dest_oq of 0 for default class
                                               xoff_val); // pfc-cos
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming xoff2oq map for "
                              "Qos-class {} on port {} ret {}",
                              qos_class->key, port, ret);
            }
            sdk_ret = elba_tm_set_uplink_mac_xoff(port,
                                                  reset_all_xoff,
                                                  set_all_xoff,
                                                  reset_pfc_xoff,
                                                  set_pfc_xoff,
                                                  (1 << xoff_val));
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming mac xoff for port {} ret {}",
                              port, ret);
            }
        }  // if update_default_class_pfc
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
        if (!elba_tm_q_valid(pd_qos_class->p4_ig_q[i])) {
            continue;
        }
        qos_class_id = pd_qos_class->p4_ig_q[i];

        memset(&d, 0, sizeof(d));
        QOS_ACTION(egress_tm_oq) = pd_qos_class->p4_eg_q[i];
        SDK_ASSERT(elba_tm_q_valid(pd_qos_class->p4_eg_q[i]));
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

    if (!elba_tm_q_valid(oq)) {
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
        sdk_ret = elb_pb_sched_spq_pgm (0, 0, port, oq, &input_info);

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

    has_pcp = cmap_type_pcp(qos_class->cmap.type);
    has_dscp = cmap_type_dscp(qos_class->cmap.type);

    if (has_pcp) {
        dot1q_pcp = qos_class->cmap.dot1q_pcp;
    } else {
        dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
    }

    /*
     * Setting the PCP-to-Q mapping one-to-one may cause packets to get
     * classified to an IQ matching the packet's PCP, even though it was not
     * configured to do so. In case of congestion, PFC/Link pause behavior could be broken.
     * Setting it to 0 maps all PCPs to Q0. In PFC mode, in case Q0 is congested,
     * PFCs could be TXed out of all priorities.
     */
    // reset the PCP to default Q
    iq = QOS_QUEUE_DEFAULT;
    // reset PCP to IQ map to one-to-one
    //iq = dot1q_pcp;
    if (!elba_tm_q_valid(iq)) {
        return HAL_RET_OK;
    }
    for (port = TM_UPLINK_PORT_BEGIN; port < TM_UPLINK_PORT_END; port++) {
        sdk_ret = elba_tm_uplink_input_map_update(
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
            // program the default COS for dscp map
            sdk_ret = elba_tm_uplink_input_dscp_map_update(port,
                               QOS_COS_DEFAULT, qos_class->cmap.ip_dscp);
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
        if (!elba_tm_q_valid(pd_qos_class->p4_ig_q[i])) {
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

    ret = qos_class_pd_deprogram_uplink_xoff(pd_qos_class);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error deprogramming uplink oq xoff for "
                      "Qos-class {} ret {}",
                      qos_class->key, ret);
        return ret;
    }

#if 0
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
                             elba_queue_t iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS],
                             elba_queue_t oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS])
{
    uint32_t iq_idx = 0;
    uint32_t oq_idx = 0;
    uint32_t port;
    elba_queue_t *input_queue;
    elba_queue_t *output_queue;
    uint32_t iq_cnt = CAPRI_TM_MAX_IQS;
    uint32_t oq_cnt = CAPRI_TM_MAX_OQS;

    for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
        iq_idx = 0;
        oq_idx = 0;
        if (elba_tm_q_valid(qos_class_pd->uplink.iq)) {
            SDK_ASSERT(iq_idx < iq_cnt);
            input_queue = &iqs[port][iq_idx++];
            input_queue->valid = true;
            input_queue->queue = qos_class_pd->uplink.iq;
        }

        if (elba_tm_q_valid(qos_class_pd->dest_oq)) {
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
            if (elba_tm_q_valid(qos_class_pd->txdma[i].iq)) {
                SDK_ASSERT(iq_idx < iq_cnt);
                input_queue = &iqs[port][iq_idx++];
                input_queue->valid = true;
                input_queue->queue = qos_class_pd->txdma[i].iq;
            }
        }

        if (elba_tm_q_valid(qos_class_pd->dest_oq)) {
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
        if (elba_tm_q_valid(qos_class_pd->p4_ig_q[i])) {
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
        if (elba_tm_q_valid(qos_class_pd->p4_eg_q[i])) {
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
    elba_queue_t iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS] = {};
    elba_queue_t oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS] = {};

    qos_class_pd_get_all_queues(qos_class_pd,
                                iqs, oqs);

    for (unsigned port = 0; port < SDK_ARRAY_SIZE(iqs); port++) {
        for (unsigned i = 0; i < SDK_ARRAY_SIZE(iqs[0]); i++) {
            if (iqs[port][i].valid) {
                elba_tm_reset_iq_stats(port, iqs[port][i].queue);
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Qos-class init TC to IQ mapping
// ----------------------------------------------------------------------------
hal_ret_t
pd_qos_class_init_tc_to_iq_map (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    sdk_ret_t sdk_ret = SDK_RET_OK;
    tm_port_t tm_port = TM_UPLINK_PORT_BEGIN;
    tm_q_t    iq;
    uint32_t  dot1q_pcp;

    HAL_TRACE_DEBUG("Initing TC to IQ map 1-to-1");

    for(iq = QOS_UPLINK_IQ_START_INDEX; iq <= QOS_UPLINK_IQ_END_INDEX; iq ++) {

        dot1q_pcp = iq;     // map PCP to iq one-to-one by default

        for (tm_port = TM_UPLINK_PORT_BEGIN;
                            tm_port <= TM_UPLINK_PORT_END; tm_port++) {
            sdk_ret = elba_tm_uplink_input_map_update(tm_port,
                                                       dot1q_pcp,
                                                       iq);
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error initing TC to IQ map 1-to-1 for "
                              "tm_port {} ret {}",
                              tm_port, ret);
                break;
            }
        }
    }   // for iq

    return ret;
}

// walk through all the user-defined classes and get
// the iq and oq to TC mapping
void
pd_qos_class_get_iq_oq_to_tc_mapping(uint32_t *iq_to_tc, uint32_t *oq_to_tc)
{
    qos_class_t *qos_class = NULL;
    qos_group_t qos_group;

    for (qos_group = QOS_GROUP_USER_DEFINED_1;
         qos_group <= QOS_GROUP_USER_DEFINED_6;
         qos_group = qos_group_get_next_user_defined(qos_group)) {

        qos_class = find_qos_class_by_group(qos_group);

        if(!qos_class)
            continue;

        iq_to_tc[qos_class->pd->uplink.iq] = qos_group;
        oq_to_tc[qos_class->pd->dest_oq] = qos_group;
    }
}

// walk through all the user-defined no-drop classes and form
// the bitmap of all the pfc-cos values
uint32_t
pd_qos_class_get_all_tc_pfc_cos_bitmap()
{
    qos_class_t *qos_class = NULL;
    qos_group_t qos_group;
    uint32_t    pfc_cos_bitmap = 0x1;   // default class should be no-drop with PFC-COS 0
    hal_ret_t   ret = HAL_RET_OK;
    sdk_ret_t   sdk_ret = SDK_RET_OK;
    tm_port_t   port;
    uint32_t    xoff_val=0;
    uint32_t    dot1q_pcp = 0;
    bool        update_default_class_pfc = false;

    for (qos_group = QOS_GROUP_USER_DEFINED_1;
         qos_group <= QOS_GROUP_USER_DEFINED_6;
         qos_group = qos_group_get_next_user_defined(qos_group)) {

        qos_class = find_qos_class_by_group(qos_group);

        if(!qos_class)
            continue;

        if(qos_class->no_drop) {
            pfc_cos_bitmap |= (1 << qos_class->pause.pfc_cos);
        }

        if ( qos_class->no_drop && (qos_class->pause.pfc_cos == 0) ) {
            if (cmap_type_pcp(qos_class->cmap.type)) {
                dot1q_pcp = qos_class->cmap.dot1q_pcp;
            } else {
                dot1q_pcp = qos_class_group_get_dot1q_pcp(qos_class);
            }

            if (dot1q_pcp == qos_class->pause.pfc_cos) {
                // if user-defined class has PCP/PFC-COS 0, then update
                // default-class to first available PCP
                update_default_class_pfc = true;
            }
        }
    }

    if(update_default_class_pfc) {
        /*
         * class-default could have been set a different PFC-COS value,
         * read xoff2oq for oq0 and get the PFC-COS value
         */

        sdk_ret = elba_tm_get_uplink_oq_xoff_map(0, // port
                                                 0, // oq
                                                 &xoff_val);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error getting uplink_oq_xoff_map for "
                          "port 0 oq 0 ret {}", ret);
        }

        if(xoff_val == 0) {
            /*
             * default class not set to a different PCP/PFC-COS value yet
             * because xoff_val for default class is 0 and a user-defined
             * class seems to be having PFC-COS of 0
             */

            HAL_TRACE_DEBUG("getting first free value from pfc_cos_bitmap {}", pfc_cos_bitmap);

            // get the first free xoff cos value from pfc_cos_bitmap
            for(xoff_val = 0; xoff_val < 8; xoff_val++) {
                if(!(pfc_cos_bitmap & (1 << xoff_val))) {
                    break;
                }
            }  // for xoff_val
            if(xoff_val == 8) {
                HAL_TRACE_DEBUG("couldnt find free PFC COS value {} for default class", pfc_cos_bitmap);
                xoff_val = 0;
            }
        }
        pfc_cos_bitmap |= (1 << xoff_val);
        HAL_TRACE_DEBUG("setting PFC COS bit {} for default class", xoff_val);

        // program the xoff2oq map as well
        for (port = TM_UPLINK_PORT_BEGIN; port <= TM_UPLINK_PORT_END; port++) {
            sdk_ret = elba_tm_uplink_oq_update(port,
                                               0,  // dest_oq of 0 for default class
                                               xoff_val); // pfc-cos
            ret = hal_sdk_ret_to_hal_ret(sdk_ret);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Error programming xoff2oq map for "
                              "oq 0 on port {} ret {}",
                              port, ret);
            }
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

            HAL_TRACE_DEBUG("programming pfc_cos_bitmap to {}", pfc_cos_bitmap);
        }
    }

    for (tm_port = TM_UPLINK_PORT_BEGIN;
                        tm_port <= TM_UPLINK_PORT_END; tm_port++) {
        sdk_ret = elba_tm_set_uplink_mac_xoff(tm_port,
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


// ----------------------------------------------------------------------------
// SWM related
// ----------------------------------------------------------------------------

#define QOS_ACTION(_arg) d.action_u.qos_qos._arg
static hal_ret_t
qos_class_pd_program_qos_table_for_swm (uint32_t swm_p4_q, bool program)
{
    hal_ret_t      ret = HAL_RET_OK;
    sdk_ret_t      sdk_ret;
    directmap      *qos_tbl = NULL;
    qos_actiondata_t d;
    uint32_t       qos_class_id;

    qos_tbl = g_hal_state_pd->dm_table(P4TBL_ID_QOS);
    SDK_ASSERT_RETURN(qos_tbl != NULL, HAL_RET_ERR);

    qos_class_id = swm_p4_q;    // p4_ig_q

    memset(&d, 0, sizeof(d));

    if(program) {
        if(swm_p4_q == CAPRI_TM_P4_SWM_UC_QUEUE) {
            QOS_ACTION(egress_tm_oq) = CAPRI_TM_P4_SWM_UC_QUEUE_REPLACEMENT;    // p4_eg_q
        } else if (swm_p4_q == CAPRI_TM_P4_SWM_FLOOD_QUEUE) {
            QOS_ACTION(egress_tm_oq) = CAPRI_TM_P4_SWM_FLOOD_QUEUE_REPLACEMENT; // p4_eg_q
        } else {
            QOS_ACTION(egress_tm_oq) = swm_p4_q;    // p4_eg_q
        }

        // Non BMC UC traffic picks oq 0 at mgmt port
        if (swm_p4_q != CAPRI_TM_P4_SWM_FLOOD_QUEUE) {
            QOS_ACTION(dest_tm_oq) = (swm_p4_q - CAPRI_TM_P4_UPLINK_IQ_OFFSET);  // dest_oq: 5 or 6
        }
        SDK_ASSERT(elba_tm_q_valid(swm_p4_q));
    } else {
        // de-program - nothing to set
    }

    sdk_ret = qos_tbl->update(qos_class_id, &d);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("qos table write failure, qos-class-id {} p4_eg_q {} "
                      "ret {}",
                      qos_class_id, swm_p4_q, ret);
        return ret;
    }

    HAL_TRACE_DEBUG("qos table qos-class-id {} programmed egress_tm_oq {} "
                    "dest_oq {} cos_en {} cos {} dscp_en {} dscp {} ",
                    qos_class_id,
                    QOS_ACTION(egress_tm_oq),
                    QOS_ACTION(dest_tm_oq),
                    QOS_ACTION(cos_en),
                    QOS_ACTION(cos),
                    QOS_ACTION(dscp_en),
                    QOS_ACTION(dscp));

    return ret;
}
#undef QOS_ACTION

hal_ret_t
pd_qos_program_uplink_for_swm (uint32_t swm_uplink_port, uint64_t dmac, bool program)
{
    hal_ret_t   ret = HAL_RET_OK;
    sdk_ret_t   sdk_ret;

    uint32_t    etype = 0;
    uint32_t    cos = 0;
    int32_t     tc = 0;
    int32_t     tc_to_iq = 0;
    int32_t     uplink_iq = 0;
    int32_t     p4_oq = CAPRI_TM_P4_UPLINK_IQ_OFFSET;   // 24
    int32_t     default_p4_oq = CAPRI_TM_P4_UPLINK_IQ_OFFSET;   // 24

    bool        mgmt_port = ((swm_uplink_port == TM_PORT_NCSI) ? true : false);

    if (mgmt_port) {
        tc = QOS_SWM_CAM_NCSI_COS;
        uplink_iq = QOS_SWM_CAM_NCSI_COS;

        if (program) {
            etype = QOS_SWM_NCSI_ETHERTYPE;
            cos = QOS_SWM_CAM_NCSI_COS;
            tc_to_iq = QOS_SWM_CAM_NCSI_COS;
            p4_oq = CAPRI_TM_P4_SWM_NCSI_QUEUE;
            default_p4_oq = CAPRI_TM_P4_SWM_UC_QUEUE;
        }
    } else {
        tc = QOS_SWM_CAM_COS;
        uplink_iq = QOS_SWM_CAM_COS;

        if (program) {
            cos = QOS_SWM_CAM_COS;
            tc_to_iq = QOS_SWM_CAM_COS;
            p4_oq = CAPRI_TM_P4_SWM_UC_QUEUE;
            default_p4_oq = CAPRI_TM_P4_SWM_FLOOD_QUEUE;
        }
    }

    if (mgmt_port) {
        // program the ethertype to cam_type and set cam_enable to compare ethertype
        sdk_ret = elba_tm_uplink_set_cam_type(TM_PORT_NCSI,
                                               QOS_SWM_CAM_ENTRY, etype);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming etype {} for port {} ret {}",
                          etype, TM_PORT_NCSI, sdk_ret);
            return ret;
        }
    } else {
        // program the dmac to cam_da and set cam_enable to compare DA
        sdk_ret = elba_tm_uplink_set_cam_da(swm_uplink_port,
                                             QOS_SWM_CAM_ENTRY, dmac);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming dmac {} for port {} ret {}",
                          dmac, swm_uplink_port, sdk_ret);
            return ret;
        }
    }

    /*
     *      DMAC/ETHERTYPE (dmac/etype) =>  COS (cos)
     *      COS (tc)                    =>  IQ (tc_to_iq)
     *      IQ (uplink_iq)              =>  P4_OQ
     */

    // program cam_cos to drive SWM_CAM_COS/ SWM_CAM_NCSI_COS
    sdk_ret = elba_tm_uplink_set_cam_cos(swm_uplink_port,
                                         QOS_SWM_CAM_ENTRY, cos);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming cam_cos {} for port {} ret {}",
                      cos, swm_uplink_port, sdk_ret);
        return ret;
    }

    // program tc_to_q mapping based on SWM_CAM_COS/ SWM_CAM_NCSI_COS
    sdk_ret = elba_tm_uplink_input_map_update(swm_uplink_port,
                                               tc,          // cos
                                               tc_to_iq);   // iq
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming tc_to_q map for cos {} to iq {} "
                      "on port {} ret {}",
                      tc, tc_to_iq, swm_uplink_port, ret);
        return ret;
    }

    // program iq_to_p4_oq map for BMC UC queue/ NCSI queue
    sdk_ret = elba_tm_set_uplink_iq_to_p4_oq_map(swm_uplink_port,
                                                  uplink_iq,
                                                  p4_oq);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming uplink_iq_to_p4_oq_map for iq {} "
                      "p4_oq {} on port {} ret {}",
                      uplink_iq, p4_oq, swm_uplink_port, ret);
        return ret;
    }

    // set default RX queue for SWM uplink/ mgmt  port to
    // CAPRI_TM_P4_SWM_FLOOD_QUEUE/ CAPRI_TM_P4_SWM_UC_QUEUE,
    // map all uplink iqs other than QOS_SWM_CAM_COS/ QOS_SWM_CAM_NCSI_COS
    for (int iq = 0; iq < 8; iq ++) {

        if (iq == uplink_iq)
            continue;

        // TODO: look for user-defined classes and skip defaulting those queues

        // program iq_to_p4_oq map for BMC MC/BC queue
        sdk_ret = elba_tm_set_uplink_iq_to_p4_oq_map(swm_uplink_port,
                                                     iq,
                                                     default_p4_oq);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Error programming uplink_iq_to_p4_oq_map for iq {} "
                          "p4_oq {} on port {} ret {}",
                          iq, default_p4_oq, swm_uplink_port, ret);
            return ret;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_qos_reserve_and_program_swm_queues (uint32_t swm_p4_q, bool alloc)
{
    hal_ret_t       ret = HAL_RET_OK;
    indexer::status ret_idx = indexer::SUCCESS;
    uint32_t        swm_uplink_q = 0;
    const char      *str = (alloc ? "alloc" : "de-alloc");

    swm_uplink_q = swm_p4_q - CAPRI_TM_P4_UPLINK_IQ_OFFSET; // 5 or 6

    HAL_TRACE_DEBUG("{} of swm_uplink_q {} swm_p4_q {}",
                    str, swm_uplink_q, swm_p4_q);

    if (g_hal_state_pd->qos_uplink_iq_idxr()->is_index_allocated(swm_uplink_q)) {
        if(alloc) {
#if 0
        HAL_TRACE_ERR("swm_uplink_q {} already in use; "
                      "failing swm qos config for swm uplink port {}",
                      swm_uplink_q, args->swm_uplink_port);
        return HAL_RET_ENTRY_EXISTS;
#endif
            // TODO: to fig out why is it already allocated even w/o any config
            HAL_TRACE_DEBUG("swm_uplink_q {} already in use; "
                            "freeing it up for swm qos config",
                            swm_uplink_q);
        }

        ret_idx = g_hal_state_pd->qos_uplink_iq_idxr()->free(swm_uplink_q);
        if (ret_idx != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free swm_uplink_q {} ret_idx {}",
                          swm_uplink_q, ret_idx);
            return HAL_RET_ERR;
        }
    }

    if(alloc) {
        // alloc the queue
        ret_idx = g_hal_state_pd->qos_uplink_iq_idxr()->alloc_withid(swm_uplink_q);
        if (ret_idx != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to alloc swm_uplink_q {} ret_idx {}",
                          swm_uplink_q, ret_idx);
            return HAL_RET_NO_RESOURCE;
        }

        //TODO: alloc the queue at qos_txdma_iq_idxr as well; esp for NCSI queue
        //that ingresses at mgmt port and egresses PB at PXDMA port towards ARM.
    }

    // program/ reset the PQ_QOS_TBL
    ret = qos_class_pd_program_qos_table_for_swm(swm_p4_q, alloc);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program qos_table for swm_p4_q {} ret {}",
                      swm_p4_q, ret);
        return HAL_RET_ERR;
    }

    HAL_TRACE_DEBUG("{} done for SWM queues", str);

    return ret;
}

hal_ret_t
pd_qos_swm_queue_init (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;

    pd_qos_swm_queue_init_args_t *args =
                            pd_func_args->pd_qos_swm_queue_init;

    HAL_TRACE_DEBUG("SWM queue init for uplink {}", args->swm_uplink_port);

    if (args->swm_uplink_port > TM_PORT_UPLINK_7) {
        HAL_TRACE_ERR("unsupported port number for SWM {}",
                      args->swm_uplink_port);
        return HAL_RET_INVALID_ARG;
    }

    /*
     * Reserving Queue #30 at P4 ingr, #16 at P4 egr ports for SWM UC traffic.
     * Reserving Queue #29 at P4 ingr/egr ports for SWM NCSI protocol traffic.
     *
     * To be able to do that, respective uplink queues need to be reserved,
     * hence allocating queues 6 and 5 (at uplinks) so that they remain
     * unavailable for user-defined classes.
     *
     * Queue #31 is used at P4 ingr, #17 at P4 egr for SWM flood traffic, but
     * since this queue is not associated with uplink-queues - they are unused
     * and hence need not be reserved in the indexer.
     */

    // reserve BMC UC queue for SWM
    ret = pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_UC_QUEUE, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to reserve/program queues for swm_p4_q {} ret {}",
                      CAPRI_TM_P4_SWM_UC_QUEUE, ret);
        return ret;
    }

    // reserve NCSI queue for SWM
    ret = pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_NCSI_QUEUE, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to reserve/program queues for swm_p4_q {} ret {}",
                      CAPRI_TM_P4_SWM_NCSI_QUEUE, ret);
        //TODO: should this be freed as it failed or use whatever has been
        // allocated successfully? That would make debugging a nightmare..
        if(pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_UC_QUEUE,
                                                        false) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} "
                          "ret {}",
                          CAPRI_TM_P4_SWM_UC_QUEUE, ret);
        }
        return ret;
    }

    // nothing to reserve, just program BMC Flood queue for SWM
    ret = qos_class_pd_program_qos_table_for_swm(CAPRI_TM_P4_SWM_FLOOD_QUEUE,
                                                 true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program queues for swm_p4_q {} ret {}",
                      CAPRI_TM_P4_SWM_FLOOD_QUEUE, ret);
        //TODO: should this be freed as it failed or use whatever has been
        // allocated successfully? That would make debugging a nightmare..
        if(pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_UC_QUEUE,
                                                        false) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} "
                          "ret {}",
                          CAPRI_TM_P4_SWM_UC_QUEUE, ret);
        }
        if(pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_NCSI_QUEUE,
                                                        false) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} "
                          "ret {}",
                          CAPRI_TM_P4_SWM_NCSI_QUEUE, ret);
        }
        return ret;
    }

    HAL_TRACE_DEBUG("Done allocating SWM related queues!");

    ret = pd_qos_program_uplink_for_swm(args->swm_uplink_port, args->dmac, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program HW ret {}", ret);
        //TODO: should this be freed as it failed or use whatever has been
        // allocated successfully? That would make debugging a nightmare..
        if(pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_UC_QUEUE,
                                                        false) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} "
                          "ret {}",
                          CAPRI_TM_P4_SWM_UC_QUEUE, ret);
        }
        if(pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_NCSI_QUEUE,
                                                        false) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} "
                          "ret {}",
                          CAPRI_TM_P4_SWM_NCSI_QUEUE, ret);
        }
        return ret;
    }

    ret = pd_qos_program_uplink_for_swm(TM_PORT_NCSI, 0, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program HW ret {}", ret);
    }

    HAL_TRACE_DEBUG("Done programming the uplinks to pick the allocated queues "
                    "for SWM traffic!");

    return HAL_RET_OK;
}

hal_ret_t
pd_qos_swm_queue_deinit (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;

    pd_qos_swm_queue_deinit_args_t *args =
                            pd_func_args->pd_qos_swm_queue_deinit;

    HAL_TRACE_DEBUG("SWM queue deinit for uplink {}", args->swm_uplink_port);

    if (args->swm_uplink_port > TM_PORT_UPLINK_7) {
        HAL_TRACE_ERR("unsupported port number for SWM {}",
                      args->swm_uplink_port);
        return HAL_RET_INVALID_ARG;
    }

    // free BMC UC queue for SWM
    ret = pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_UC_QUEUE,
                                                false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} ret {}",
                      CAPRI_TM_P4_SWM_UC_QUEUE, ret);
    }

    // free NCSI queue for SWM
    ret = pd_qos_reserve_and_program_swm_queues(CAPRI_TM_P4_SWM_NCSI_QUEUE,
                                                false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to free/de-program queues for swm_p4_q {} ret {}",
                      CAPRI_TM_P4_SWM_NCSI_QUEUE, ret);
    }

    // deprogram BMC Flood queue for SWM
    ret = qos_class_pd_program_qos_table_for_swm(CAPRI_TM_P4_SWM_FLOOD_QUEUE,
                                                 false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to deprogram queues for swm_p4_q {} ret {}",
                      CAPRI_TM_P4_SWM_FLOOD_QUEUE, ret);
    }

    HAL_TRACE_DEBUG("Done de-allocating SWM related queues!");

    // reset programming on uplink
    ret = pd_qos_program_uplink_for_swm(args->swm_uplink_port, 0, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to deprogram HW ret {}", ret);
    }

    // reset programming on mgmt port
    ret = pd_qos_program_uplink_for_swm(TM_PORT_NCSI, 0, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program HW ret {}", ret);
    }

    HAL_TRACE_DEBUG("Done de-programming HW for SWM config!");

    return HAL_RET_OK;
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

    HAL_TRACE_DEBUG("created pd state for qos_class: {}, iq {}, oq {}",
                    args->qos_class->key, pd_qos_class->uplink.iq, pd_qos_class->dest_oq);

    // Update the iq and oq to TC mapping for user-defined classes
    if (qos_group_is_user_defined(args->qos_class->key.qos_group)) {
        g_qos_iq_to_tc[pd_qos_class->uplink.iq] = args->qos_class->key.qos_group;
        g_qos_oq_to_tc[pd_qos_class->dest_oq] = args->qos_class->key.qos_group;
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
    uint8_t iq, oq;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->qos_class != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->qos_class->pd != NULL), HAL_RET_INVALID_ARG);
    qos_class_pd = (pd_qos_class_t *)args->qos_class->pd;

    iq = qos_class_pd->uplink.iq;
    oq = qos_class_pd->dest_oq;
    HAL_TRACE_DEBUG("deleting pd state for qos_class {}, iq {}, oq {}",
                    args->qos_class->key, qos_class_pd->uplink.iq, qos_class_pd->dest_oq);

    // TODO: deprogram hw
    qos_class_pd_deprogram_hw(qos_class_pd);

    // free up the resource and memory
    ret = qos_class_pd_cleanup(qos_class_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd qos_class cleanup Qos-class {}, ret {}",
                      args->qos_class->key, ret);
        goto err;
    }

    // Update the iq and oq to TC mapping
    if (qos_group_is_user_defined(args->qos_class->key.qos_group)) {
        g_qos_iq_to_tc[iq] = 0;
        g_qos_oq_to_tc[oq] = 0;
    }

err:
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
    elba_queue_t iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS] = {};
    elba_queue_t oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS] = {};

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
        if (elba_tm_q_valid(qos_class_pd->txdma[i].iq)) {
            epd_status->add_tx_traffic_class_coses(qos_class_pd->txdma[i].iq);
        }
    }
}

void
qos_class_queue_stats_to_proto_stats (qos::QosClassQueueStats *q_stats,
                                      elba_queue_stats_t *qos_queue_stats)
{
    /* Removing this call to avoid crashes due to ht lookup.
     * Instead, get the iq and oq map from the global tables. */

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
            input_stats->set_qos_group_idx(g_qos_iq_to_tc[i]); // Set the Qos Group (TC) index for this iq
        }
        if (qos_queue_stats->oq_stats[i].oq.valid) {
            auto output_stats = q_stats->add_output_queue_stats();
            auto oq_stats = &qos_queue_stats->oq_stats[i].stats;
            output_stats->set_output_queue_idx(qos_queue_stats->oq_stats[i].oq.queue);
            output_stats->set_queue_depth(oq_stats->queue_depth);
            output_stats->set_port_monitor(oq_stats->port_monitor);
            output_stats->set_qos_group_idx(g_qos_oq_to_tc[i]); // Set the Qos Group (TC) index for this oq
        }
    }
}

static hal_ret_t
qos_class_pd_populate_stats (pd_qos_class_t *qos_class_pd, QosClassStats *stats)
{
    hal_ret_t                first_err_ret = HAL_RET_OK;
    hal_ret_t                ret = HAL_RET_OK;
    sdk_ret_t                sdk_ret = SDK_RET_OK;
    elba_queue_t            iqs[TM_NUM_PORTS][CAPRI_TM_MAX_IQS] = {};
    elba_queue_t            oqs[TM_NUM_PORTS][CAPRI_TM_MAX_OQS] = {};
    elba_queue_stats_t      queue_stats = {0};

    qos_class_pd_get_all_queues(qos_class_pd,
                                iqs, oqs);

    // Update the stats now
    pd_qos_class_periodic_stats_update(NULL);

    for (unsigned port = 0; port < SDK_ARRAY_SIZE(iqs); port++) {
        auto port_stats = stats->add_port_stats();
        qos_class_pd_port_to_packet_buffer_port(port,
                                                port_stats->mutable_packet_buffer_port());
        auto q_stats = port_stats->mutable_qos_queue_stats();
        sdk_ret = elba_populate_queue_stats(
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
    if (elba_tm_port_is_uplink_port(dest_port)) {
        group = dest_port % 2;
        if (elba_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0 + group])) {
            *qos_class_id = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_TX_UPLINK_GROUP_0 + group];
        }
    } else if (elba_tm_port_is_dma_port(dest_port)) {
        if (elba_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX])) {
            *qos_class_id = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_RX];
        }
    }

    if (!elba_tm_q_valid(*qos_class_id) &&
        elba_tm_q_valid(pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_COMMON])) {
        *qos_class_id = pd_qos_class->p4_ig_q[HAL_PD_QOS_IQ_COMMON];
    }

    if (!elba_tm_q_valid(*qos_class_id)) {
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
    sdk_ret_t sdk_ret = elba_tm_periodic_stats_update();
    return hal_sdk_ret_to_hal_ret(sdk_ret);
}

}    // namespace pd
}    // namespace hal
