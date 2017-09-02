#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <rdma_pd.hpp>
#include <pd.hpp>
#include <base.h>
#include "directmap.hpp"
#include <hal_state_pd.hpp>
//#include "if_pd_utils.hpp"

namespace hal {
namespace pd {

hal_ret_t
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_page_id,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries)
{
    hal_ret_t                    ret;
    DirectMap                    *dm;
    rx_stage0_rdma_params_table_actiondata data = { 0 };

    HAL_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_RDMA_PARAMS_TABLE);
    HAL_ASSERT(dm != NULL);

    data.actionid = RX_STAGE0_RDMA_PARAMS_TABLE_RX_STAGE0_LOAD_RDMA_PARAMS_ID;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.cqcb_base_addr_page_id = cqcb_base_addr_page_id;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.rx_stage0_rdma_params_table_action_u.rx_stage0_rdma_params_table_rx_stage0_load_rdma_params.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;

    ret = dm->insert_withid(&data, idx);
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
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get (uint32_t idx, rx_stage0_rdma_params_table_actiondata *data)
{
    hal_ret_t                    ret;
    DirectMap                    *dm;
    
    HAL_ASSERT(idx < MAX_LIFS);
    
    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_RDMA_PARAMS_TABLE);
    HAL_ASSERT(dm != NULL);
    
    ret = dm->retrieve(idx, data);
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
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_page_id,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries)
{
    hal_ret_t                    ret;
    DirectMap                    *dm;
    tx_stage0_rdma_params_table_actiondata data = { 0 };

    HAL_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_RDMA_PARAMS_TABLE);
    HAL_ASSERT(dm != NULL);

    data.actionid = RX_STAGE0_RDMA_PARAMS_TABLE_RX_STAGE0_LOAD_RDMA_PARAMS_ID;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.cqcb_base_addr_page_id = cqcb_base_addr_page_id;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.tx_stage0_rdma_params_table_action_u.tx_stage0_rdma_params_table_tx_stage0_load_rdma_params.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;

    ret = dm->insert_withid(&data, idx);
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
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get (uint32_t idx, tx_stage0_rdma_params_table_actiondata *data)
{
    hal_ret_t                    ret;
    DirectMap                    *dm;
    
    HAL_ASSERT(idx < MAX_LIFS);
    
    dm = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_RDMA_PARAMS_TABLE);
    HAL_ASSERT(dm != NULL);
    
    ret = dm->retrieve(idx, data);
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
