#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"
#include "nic/hal/pd/gft/lif_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/gft/scheduler_pd.hpp"
#include "nic/hal/pd/capri/capri_txs_scheduler.hpp"
#include "nic/utils/bitmap/bitmap.hpp"

namespace hal {
namespace pd {

//helper function to count number of set bits.
static uint32_t num_set_bits(uint16_t n)
{
    uint32_t count = 0;
    while (n)
    {
      n &= (n-1) ;
      count++;
    }
    return count;
}

hal_ret_t scheduler_tx_pd_alloc (pd_lif_t *lif_pd) 
{

    hal_ret_t     ret = HAL_RET_OK;
    lif_t         *pi_lif;
    uint32_t      alloc_offset = INVALID_INDEXER_INDEX;
    uint32_t      alloc_units = 0, total_qcount = 0;

    if (lif_pd->tx_sched_table_offset != INVALID_INDEXER_INDEX) {
        HAL_TRACE_DEBUG("pd-lif:{} Scheduler resource already allocated for"
                        ":lif_id:{},at offset:{} and num-of-entries {}",
                        __FUNCTION__,
                        lif_get_lif_id((lif_t *)lif_pd->pi_lif),
                        lif_pd->tx_sched_num_table_entries,
                        lif_pd->tx_sched_table_offset);
        goto end;
    }
    
    pi_lif = (lif_t *)lif_pd->pi_lif;

    // Allocate txs scheduler resource for this lif.
    // Sched table can hold 8K queues per index and mandates new index for each cos.
    total_qcount = lif_get_total_qcount(lif_pd->hw_lif_id);
    alloc_units  =  (total_qcount / TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY);
    alloc_units += ((total_qcount % TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY) ? 1 : 0);
    alloc_units *=   num_set_bits(pi_lif->qos_info.cos_bmp);

    if (alloc_units > 0) {    
        //Allocate consecutive alloc_unit num of entries in sched table.
        alloc_offset = g_hal_state_pd->txs_scheduler_map_idxr()->Alloc(alloc_units);

        if (alloc_offset < 0) {
            HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to alloc txs sched res",
                      __FUNCTION__, lif_get_lif_id(pi_lif));
            lif_pd->tx_sched_table_offset = INVALID_INDEXER_INDEX;
            lif_pd->tx_sched_num_table_entries = 0;
            ret = HAL_RET_NO_RESOURCE;
            goto end; 
        }
    }

    lif_pd->tx_sched_table_offset      = alloc_offset;
    lif_pd->tx_sched_num_table_entries = alloc_units;

    HAL_TRACE_DEBUG("pd-lif:{}:lif_id:{},allocated :{} scheduler resource at offset:{}",
                    __FUNCTION__,
                    lif_get_lif_id((lif_t *)lif_pd->pi_lif),
                    lif_pd->tx_sched_num_table_entries,
                    lif_pd->tx_sched_table_offset);
end:
    return ret;
}

hal_ret_t scheduler_tx_pd_dealloc (pd_lif_t *lif_pd) 
{
    hal_ret_t      ret = HAL_RET_OK;
    uint32_t       alloc_units, alloc_offset;

    if (!lif_pd) {
        // Nothing to do
        goto end;
    }

    alloc_offset  =     lif_pd->tx_sched_table_offset;
    alloc_units   =     lif_pd->tx_sched_num_table_entries;

    //Free the scheduler table indices.
    g_hal_state_pd->txs_scheduler_map_idxr()->Free(alloc_offset, alloc_units);
    lif_pd->tx_sched_table_offset      = INVALID_INDEXER_INDEX;
    lif_pd->tx_sched_num_table_entries = 0;

end:
    return ret;
}

hal_ret_t scheduler_tx_pd_program_hw (pd_lif_t *lif_pd) 
{
    hal_ret_t              ret = HAL_RET_OK;
    lif_t                  *pi_lif;
    txs_sched_lif_params_t txs_hw_params = {0};
    uint32_t               num_cos_values = 0;

    if (!lif_pd) {
        // Nothing to do
        goto end;
    }

    pi_lif = (lif_t *)lif_pd->pi_lif;
    num_cos_values = num_set_bits(pi_lif->qos_info.cos_bmp);

    if (num_cos_values && lif_pd->tx_sched_num_table_entries) {
        txs_hw_params.sched_table_offset  = lif_pd->tx_sched_table_offset;
        txs_hw_params.num_entries_per_cos = (lif_pd->tx_sched_num_table_entries / num_cos_values);
        txs_hw_params.cos_bmp             = pi_lif->qos_info.cos_bmp;

        ret = capri_txs_scheduler_lif_params_update(lif_pd->hw_lif_id, &txs_hw_params);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to program tx sched lif params in hw",
                              __FUNCTION__, lif_get_lif_id(pi_lif));
        }
    } 
end:
    return ret;
}

hal_ret_t scheduler_tx_pd_deprogram_hw (pd_lif_t *lif_pd)
{   
    hal_ret_t              ret = HAL_RET_OK;
    lif_t                  *pi_lif;
    txs_sched_lif_params_t txs_hw_params = {0};
    
    if (!lif_pd) {
        // Nothing to do
        goto end;
    }
    
    pi_lif = (lif_t *)lif_pd->pi_lif;

    // Reset cos_bmp in PI lif.
    pi_lif->qos_info.cos_bmp = 0x0;
    
    // Pass txs params for delete case.               
    txs_hw_params.num_entries_per_cos = 0;
    txs_hw_params.cos_bmp             = 0x0;
                
    ret = capri_txs_scheduler_lif_params_update(lif_pd->hw_lif_id, &txs_hw_params);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-lif:{}:lif_id:{},failed to program tx sched lif params in hw",
                              __FUNCTION__, lif_get_lif_id(pi_lif));
    }
end:
    return ret;     
}                    
}    // namespace pd
}    // namespace hal

