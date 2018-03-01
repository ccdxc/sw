#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/hal/pd/asicpd/asic_pd_scheduler.hpp"
#include "sdk/utils.hpp"

namespace hal {
namespace pd {

hal_ret_t
asicpd_scheduler_tx_pd_alloc (asicpd_scheduler_lif_params_t *lif)
{
    hal_ret_t     ret = HAL_RET_OK;
    uint32_t      alloc_offset = INVALID_INDEXER_INDEX;
    uint32_t      alloc_units = 0;
    capri_txs_sched_lif_params_t params;

    if (lif->tx_sched_table_offset != INVALID_INDEXER_INDEX) {
        HAL_TRACE_DEBUG("Scheduler resource already allocated for"
                        ":lif_id:{},at offset:{} and num-of-entries {}",
                        lif->lif_id,
                        lif->tx_sched_num_table_entries,
                        lif->tx_sched_table_offset);
        goto end;
    }
    
    // Allocate txs scheduler resource for this lif.
    // Sched table can hold 8K queues per index and mandates new index for each cos.
    params.total_qcount = lif->total_qcount;
    ret = capri_txs_scheduler_tx_alloc(&params, &alloc_offset, &alloc_units);
    if (ret != HAL_RET_OK) {    
        HAL_TRACE_ERR("lif_id:{},failed to alloc txs sched res",
                       lif->lif_id);
        lif->tx_sched_table_offset = INVALID_INDEXER_INDEX;
        lif->tx_sched_num_table_entries = 0;
        ret = HAL_RET_NO_RESOURCE;
        goto end; 
    }
    lif->tx_sched_table_offset      = alloc_offset;
    lif->tx_sched_num_table_entries = alloc_units;
    HAL_TRACE_DEBUG("lif_id: {},allocated :{} scheduler resource at offset:{}",
                    lif->lif_id,
                    lif->tx_sched_num_table_entries,
                    lif->tx_sched_table_offset);
end:
    return ret;
}

hal_ret_t
asicpd_scheduler_tx_pd_dealloc (asicpd_scheduler_lif_params_t *lif) 
{
    hal_ret_t      ret = HAL_RET_OK;

    if (!lif) {
        goto end;
    }
    //Free the scheduler table indices.
    ret = capri_txs_scheduler_tx_dealloc(lif->tx_sched_table_offset,
                                         lif->tx_sched_num_table_entries);
    if (ret != HAL_RET_OK) {
        goto end;
    }
    lif->tx_sched_table_offset      = INVALID_INDEXER_INDEX;
    lif->tx_sched_num_table_entries = 0;
end:
    return ret;
}

hal_ret_t
asicpd_scheduler_tx_pd_program_hw (asicpd_scheduler_lif_params_t *lif) 
{
    hal_ret_t                    ret = HAL_RET_OK;
    uint32_t                     num_cos_values = 0;
    capri_txs_sched_lif_params_t txs_hw_params = {0};

    if (!lif) {
        goto end;
    }
    num_cos_values = sdk::lib::set_bits_count(lif->cos_bmp);
    if (num_cos_values && lif->tx_sched_num_table_entries) {
        txs_hw_params.sched_table_offset  = lif->tx_sched_table_offset;
        txs_hw_params.num_entries_per_cos = (lif->tx_sched_num_table_entries / num_cos_values);
        txs_hw_params.cos_bmp             = lif->cos_bmp;
        ret = capri_txs_scheduler_lif_params_update(lif->hw_lif_id, &txs_hw_params);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif_id:{},failed to program tx sched lif params in hw",
                           lif->lif_id);
        }
    }
end:
    return ret;
}

hal_ret_t
asicpd_scheduler_tx_pd_deprogram_hw (asicpd_scheduler_lif_params_t *lif) 
{   
    hal_ret_t                    ret = HAL_RET_OK;
    capri_txs_sched_lif_params_t txs_hw_params = {0};
    
    if (!lif) {
        goto end;
    }
    // Pass txs params for delete case.               
    txs_hw_params.num_entries_per_cos = 0;
    txs_hw_params.cos_bmp             = 0x0;
    ret = capri_txs_scheduler_lif_params_update(lif->hw_lif_id, &txs_hw_params);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("lif_id:{},failed to program tx sched lif params in hw",
                       lif->lif_id);
    }
end:
    return ret;     
}

hal_ret_t
asicpd_policer_tx_pd_program_hw (asicpd_scheduler_lif_params_t *lif) 
{
    hal_ret_t                         ret = HAL_RET_OK;
    uint32_t                          num_cos_values = 0;
    capri_txs_policer_lif_params_t    txs_hw_params = {0};

    if (!lif) {
        goto end;
    }
    num_cos_values = sdk::lib::set_bits_count(lif->cos_bmp);
    if (num_cos_values && lif->tx_sched_num_table_entries) {
        txs_hw_params.sched_table_start_offset  = lif->tx_sched_table_offset;
        txs_hw_params.sched_table_end_offset = (lif->tx_sched_table_offset + lif->tx_sched_num_table_entries - 1);

        ret = capri_txs_policer_lif_params_update(lif->hw_lif_id, &txs_hw_params);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("lif_id:{},failed to program tx policer lif params in hw",
                           lif->lif_id);
        }
    }
end:
    return ret;
}

}    // namespace pd
}    // namespace hal

