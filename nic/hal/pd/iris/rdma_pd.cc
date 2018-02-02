#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/rdma_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/base.h"
#include "sdk/directmap.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
//#include "if_pd_utils.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_rxdma_table_entry_add(pd_rxdma_table_entry_add_args_t *args) {
    return p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add(args->idx,
                                                                       args->rdma_en_qtype_mask,
                                                                       args->pt_base_addr_page_id,
                                                                       args->log_num_pt_entries,
                                                                       args->cqcb_base_addr_page_id,
                                                                       args->log_num_cq_entries,
                                                                       args->prefetch_pool_base_addr_page_id,
                                                                       args->log_num_prefetch_pool_entries,
                                                                       args->sq_qtype,
                                                                       args->rq_qtype);

}

hal_ret_t
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_page_id,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype)
{
    hal_ret_t                    ret;       
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    rx_stage0_load_rdma_params_actiondata data = { 0 };

    HAL_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS);
    HAL_ASSERT(dm != NULL);

    data.actionid = RX_STAGE0_LOAD_RDMA_PARAMS_RX_STAGE0_LOAD_RDMA_PARAMS_ID;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.cqcb_base_addr_page_id = cqcb_base_addr_page_id;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sq_qtype = sq_qtype;
    data.rx_stage0_load_rdma_params_action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rq_qtype = rq_qtype;

    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for rxdma, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("stage0 rdma LIF table entry add successful for rxdma, idx : {}, err : {}",
                  idx, ret);
    return HAL_RET_OK;
}


hal_ret_t
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get (uint32_t idx, rx_stage0_load_rdma_params_actiondata *data)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    
    HAL_ASSERT(idx < MAX_LIFS);
    
    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS);
    HAL_ASSERT(dm != NULL);
    
    sdk_ret = dm->retrieve(idx, data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for rxdma, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("stage0 rdma LIF table entry get successful for rxdma, idx : {}, err : {}",
                  idx, ret);
    return HAL_RET_OK;
}



hal_ret_t
pd_txdma_table_entry_add(pd_txdma_table_entry_add_args_t *args) {
    return p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add(args->idx,
                                                                       args->rdma_en_qtype_mask,
                                                                       args->pt_base_addr_page_id,
                                                                       args->log_num_pt_entries,
                                                                       args->cqcb_base_addr_page_id,
                                                                       args->log_num_cq_entries,
                                                                       args->prefetch_pool_base_addr_page_id,
                                                                       args->log_num_prefetch_pool_entries,
                                                                       args->sq_qtype,
                                                                       args->rq_qtype);

}
hal_ret_t
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_page_id,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    tx_stage0_lif_params_table_actiondata data = { 0 };

    HAL_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE);
    HAL_ASSERT(dm != NULL);

    data.actionid = TX_STAGE0_LIF_PARAMS_TABLE_TX_STAGE0_LIF_RDMA_PARAMS_ID;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.cqcb_base_addr_page_id = cqcb_base_addr_page_id;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sq_qtype = sq_qtype;
    data.tx_stage0_lif_params_table_action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rq_qtype = rq_qtype;

    sdk_ret = dm->insert_withid(&data, idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table write failure for txdma, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("stage0 rdma LIF table entry add successful for txdma, idx : {}, err : {}",
                  idx, ret);
    return HAL_RET_OK;
}


hal_ret_t
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get (uint32_t idx, tx_stage0_lif_params_table_actiondata *data)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    
    HAL_ASSERT(idx < MAX_LIFS);
    
    dm = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE);
    HAL_ASSERT(dm != NULL);
    
    sdk_ret = dm->retrieve(idx, data);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for txdma, idx : {}, err : {}",
                      idx, ret);
        return ret;
    }
    HAL_TRACE_DEBUG("stage0 rdma LIF table entry get successful for txdma, idx : {}, err : {}",
                  idx, ret);
    return HAL_RET_OK;
}



}    // namespace pd
}    // namespace hal
