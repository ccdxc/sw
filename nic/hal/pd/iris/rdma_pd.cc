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
p4pd_common_p4plus_rxdma_stage0_lif_table1_entry_add (uint32_t idx,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint32_t cqcb_base_addr_page_id,
                                                      uint8_t log_num_cq_entries,
                                                      uint32_t prefetch_pool_base_addr_page_id,
                                                      uint8_t log_num_prefetch_pool_entries)
{
    hal_ret_t                    ret;
    DirectMap                    *dm;
    common_p4plus_stage0_lif_table1_actiondata data = { 0 };

    HAL_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_LIF_TABLE1);
    HAL_ASSERT(dm != NULL);

    data.actionid = COMMON_P4PLUS_STAGE0_LIF_TABLE1_COMMON_P4PLUS_STAGE0_LIF_TABLE1_ID;
    data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.pt_base_addr_page_id = pt_base_addr_page_id;
    data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_pt_entries = log_num_pt_entries;
    data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.cqcb_base_addr_page_id = cqcb_base_addr_page_id;
    data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_cq_entries = log_num_cq_entries;
    data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.prefetch_pool_base_addr_page_id = prefetch_pool_base_addr_page_id;
    data.common_p4plus_stage0_lif_table1_action_u.common_p4plus_stage0_lif_table1_common_p4plus_stage0_lif_table1.log_num_prefetch_pool_entries = log_num_prefetch_pool_entries;

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
p4pd_common_p4plus_rxdma_stage0_lif_table1_entry_get (uint32_t idx, common_p4plus_stage0_lif_table1_actiondata *data)
{
    hal_ret_t                    ret;
    DirectMap                    *dm;
    
    HAL_ASSERT(idx < MAX_LIFS);
    
    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_LIF_TABLE1);
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
p4pd_common_p4plus_txdma_stage0_lif_table1_entry_get (uint32_t idx, common_p4plus_stage0_lif_table1_actiondata *data)
{
#if 0
    hal_ret_t                    ret;
    DirectMap                    *dm;

    HAL_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_COMMON_P4PLUS_STAGE0_LIF_TABLE1);
    HAL_ASSERT(dm != NULL);

    ret = dm->retrieve(idx, data);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("stage0 rdma LIF table entry get failure for txdma, idx : {}, err : {}",
                      idx, ret);
        return ret;       
    }       
#endif
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
