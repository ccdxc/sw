#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/rdma/rdma_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/base.hpp"
#include "lib/table/directmap/directmap.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
//#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

namespace hal {
namespace pd {

hal_ret_t
pd_rxdma_table_entry_add(pd_func_args_t *pd_func_args) {
    pd_rxdma_table_entry_add_args_t *args = pd_func_args->pd_rxdma_table_entry_add;
    return p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add(args->idx,
                                                                       args->rdma_en_qtype_mask,
                                                                       args->pt_base_addr_page_id,
                                                                       args->log_num_pt_entries,
                                                                       args->log_num_kt_entries,
                                                                       args->log_num_dcqcn_profiles,
                                                                       args->log_num_ah_entries,
                                                                       args->cqcb_base_addr_hi,
                                                                       args->sqcb_base_addr_hi,
                                                                       args->rqcb_base_addr_hi,
                                                                       args->log_num_cq_entries,
                                                                       args->log_num_sq_entries,
                                                                       args->log_num_rq_entries,
                                                                       args->prefetch_base_addr_page_id,
                                                                       args->log_prefetch_buf_size,
                                                                       args->sq_qtype,
                                                                       args->rq_qtype,
                                                                       args->aq_qtype);

}

hal_ret_t
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint8_t log_num_kt_entries,
                                                      uint8_t log_num_dcqcn_profiles,
                                                      uint8_t log_num_ah_entries,
                                                      uint32_t cqcb_base_addr_hi,
                                                      uint32_t sqcb_base_addr_hi,
                                                      uint32_t rqcb_base_addr_hi,
                                                      uint8_t log_num_cq_entries,
                                                      uint8_t log_num_sq_entries,
                                                      uint8_t log_num_rq_entries,
                                                      uint32_t prefetch_base_addr_page_id,
                                                      uint8_t log_prefetch_buf_size,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype,
                                                      uint8_t aq_qtype)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    rx_stage0_load_rdma_params_actiondata_t data = { 0 };

    SDK_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS);
    SDK_ASSERT(dm != NULL);

    data.action_id = RX_STAGE0_LOAD_RDMA_PARAMS_RX_STAGE0_LOAD_RDMA_PARAMS_ID;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_kt_entries = log_num_kt_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_dcqcn_profiles = log_num_dcqcn_profiles;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_ah_entries = log_num_ah_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.cqcb_base_addr_hi = cqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sqcb_base_addr_hi = sqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rqcb_base_addr_hi = rqcb_base_addr_hi;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_sq_entries = log_num_sq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_num_rq_entries = log_num_rq_entries;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.prefetch_base_addr_page_id = prefetch_base_addr_page_id;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.log_prefetch_buf_size = log_prefetch_buf_size;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.sq_qtype = sq_qtype;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.rq_qtype = rq_qtype;
    data.action_u.rx_stage0_load_rdma_params_rx_stage0_load_rdma_params.aq_qtype = aq_qtype;

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
p4pd_common_p4plus_rxdma_stage0_rdma_params_table_entry_get (uint32_t idx, rx_stage0_load_rdma_params_actiondata_t *data)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;

    SDK_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_rxdma_dm_table(P4_COMMON_RXDMA_ACTIONS_TBL_ID_RX_STAGE0_LOAD_RDMA_PARAMS);
    SDK_ASSERT(dm != NULL);

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
pd_txdma_table_entry_add(pd_func_args_t *pd_func_args) {
    pd_txdma_table_entry_add_args_t *args = pd_func_args->pd_txdma_table_entry_add;
    return p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add(args->idx,
                                                                       args->rdma_en_qtype_mask,
                                                                       args->pt_base_addr_page_id,
                                                                       args->ah_base_addr_page_id,
                                                                       args->log_num_pt_entries,
                                                                       args->log_num_kt_entries,
                                                                       args->log_num_dcqcn_profiles,
                                                                       args->log_num_ah_entries,
                                                                       args->cqcb_base_addr_hi,
                                                                       args->sqcb_base_addr_hi,
                                                                       args->rqcb_base_addr_hi,
                                                                       args->log_num_cq_entries,
                                                                       args->log_num_sq_entries,
                                                                       args->log_num_rq_entries,
                                                                       args->prefetch_base_addr_page_id,
                                                                       args->log_prefetch_buf_size,
                                                                       args->sq_qtype,
                                                                       args->rq_qtype,
                                                                       args->aq_qtype,
                                                                       args->barmap_base_addr,
                                                                       args->barmap_size,
                                                                       args->log_num_eq_entries);

}
hal_ret_t
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_add (uint32_t idx,
                                                      uint8_t rdma_en_qtype_mask,
                                                      uint32_t pt_base_addr_page_id,
                                                      uint32_t ah_base_addr_page_id,
                                                      uint8_t log_num_pt_entries,
                                                      uint8_t log_num_kt_entries,
                                                      uint8_t log_num_dcqcn_profiles,
                                                      uint8_t log_num_ah_entries,
                                                      uint32_t cqcb_base_addr_hi,
                                                      uint32_t sqcb_base_addr_hi,
                                                      uint32_t rqcb_base_addr_hi,
                                                      uint8_t log_num_cq_entries,
                                                      uint8_t log_num_sq_entries,
                                                      uint8_t log_num_rq_entries,
                                                      uint32_t prefetch_base_addr_page_id,
                                                      uint8_t log_prefetch_buf_size,
                                                      uint8_t sq_qtype,
                                                      uint8_t rq_qtype,
                                                      uint8_t aq_qtype,
                                                      uint32_t barmap_base_addr,
                                                      uint32_t barmap_size,
                                                      uint8_t log_num_eq_entries)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;
    tx_stage0_lif_params_table_actiondata_t data = { 0 };

    SDK_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE);
    SDK_ASSERT(dm != NULL);

    data.action_id = TX_STAGE0_LIF_PARAMS_TABLE_TX_STAGE0_LIF_RDMA_PARAMS_ID;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rdma_en_qtype_mask = rdma_en_qtype_mask;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.pt_base_addr_page_id = pt_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.ah_base_addr_page_id = ah_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_pt_entries = log_num_pt_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_kt_entries = log_num_kt_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_dcqcn_profiles = log_num_dcqcn_profiles;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_ah_entries = log_num_ah_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.cqcb_base_addr_hi = cqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sqcb_base_addr_hi = sqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rqcb_base_addr_hi = rqcb_base_addr_hi;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_cq_entries = log_num_cq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_sq_entries = log_num_sq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_rq_entries = log_num_rq_entries;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.prefetch_base_addr_page_id = prefetch_base_addr_page_id;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_prefetch_buf_size = log_prefetch_buf_size;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.sq_qtype = sq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.rq_qtype = rq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.aq_qtype = aq_qtype;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.barmap_base_addr = barmap_base_addr;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.barmap_size = barmap_size;
    data.action_u.tx_stage0_lif_params_table_tx_stage0_lif_rdma_params.log_num_eq_entries = log_num_eq_entries;

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
p4pd_common_p4plus_txdma_stage0_rdma_params_table_entry_get (uint32_t idx, tx_stage0_lif_params_table_actiondata_t *data)
{
    hal_ret_t                    ret;
    sdk_ret_t                    sdk_ret;
    directmap                    *dm;

    SDK_ASSERT(idx < MAX_LIFS);

    dm = g_hal_state_pd->p4plus_txdma_dm_table(P4_COMMON_TXDMA_ACTIONS_TBL_ID_TX_STAGE0_LIF_PARAMS_TABLE);
    SDK_ASSERT(dm != NULL);

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
