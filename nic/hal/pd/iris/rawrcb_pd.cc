#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/rawrcb_pd.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/wring_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/rawrcb.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/gen/rawr_rxdma/include/rawr_rxdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/p4plus_pd_api.h"

namespace hal {
namespace pd {

void *
rawrcb_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_rawrcb_t *)entry)->hw_id);
}

uint32_t
rawrcb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(rawrcb_hw_id_t)) % ht_size;
}

bool
rawrcb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(rawrcb_hw_id_t *)key1 == *(rawrcb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_rawr_rx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "rawr_rx_stage0";

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret < 0) {
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Received offset for stage0 program: {:#x}", *offset);
    return HAL_RET_OK;
}

static hal_ret_t 
p4pd_add_or_del_rawr_rx_stage0_entry(pd_rawrcb_t* rawrcb_pd, bool del)
{
    common_p4plus_stage0_app_header_table_d     data = {0};
    rawrcb_t                                    *rawrcb;
    pd_wring_meta_t                             *pd_wring_meta;
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    wring_hw_id_t                               arq_base;

    // hardware index for this entry
    rawrcb_hw_id_t hwid = rawrcb_pd->hw_id + 
        (P4PD_RAWRCB_STAGE_ENTRY_OFFSET * P4PD_HWID_RAWR_RX_STAGE0);

    if(!del) {
        // get pc address
        if(p4pd_get_rawr_rx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("RAWRCB programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        rawrcb = rawrcb_pd->rawrcb;

        if (rawrcb->chain_rxq_base) {
            data.u.rawr_rx_start_d.chain_rxq_base = rawrcb->chain_rxq_base;
            data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr = rawrcb->chain_rxq_ring_indices_addr;
            data.u.rawr_rx_start_d.chain_rxq_ring_size_shift = rawrcb->chain_rxq_ring_size_shift;
            data.u.rawr_rx_start_d.chain_rxq_entry_size_shift = rawrcb->chain_rxq_entry_size_shift;
            data.u.rawr_rx_start_d.chain_rxq_ring_index_select = rawrcb->chain_rxq_ring_index_select;
        } else {

            /*
             * Provide reasonable defaults for above
             */
            pd_wring_meta = wring_pd_get_meta(types::WRING_TYPE_ARQRX);
            ret = wring_pd_get_base_addr(types::WRING_TYPE_ARQRX, 0, &arq_base);
            if((ret == HAL_RET_OK) && pd_wring_meta) {
                HAL_TRACE_DEBUG("RAWRCB {} arq_base: {:#x}",
                                rawrcb->cb_id, arq_base);
                data.u.rawr_rx_start_d.chain_rxq_base = arq_base;
                data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);
                data.u.rawr_rx_start_d.chain_rxq_ring_size_shift = log2(pd_wring_meta->num_slots);
                data.u.rawr_rx_start_d.chain_rxq_entry_size_shift = log2(pd_wring_meta->slot_size_in_bytes);
                data.u.rawr_rx_start_d.chain_rxq_ring_index_select = 0;
                HAL_TRACE_DEBUG("RAWRCB chain_rxq_ring_indices_addr: {:#x} chain_rxq_ring_size_shift: {} "
                                "chain_rxq_entry_size_shift: {} chain_rxq_ring_index_select: {}",
                                data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr,
                                data.u.rawr_rx_start_d.chain_rxq_ring_size_shift,
                                data.u.rawr_rx_start_d.chain_rxq_entry_size_shift,
                                data.u.rawr_rx_start_d.chain_rxq_ring_index_select);
            } else {
                HAL_TRACE_ERR("Failed to receive ARQRX info for RAWRCB");
            }
        }

        data.u.rawr_rx_start_d.desc_valid_bit_req = TRUE;
        if (rawrcb->desc_valid_bit_upd) {
            data.u.rawr_rx_start_d.desc_valid_bit_req = rawrcb->desc_valid_bit_req;
        }
    }
    HAL_TRACE_DEBUG("RAWRCB Programming stage0 at hw-id: 0x{0:x}", hwid); 
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create rx: stage0 entry for RAWRCB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_rawrcb_rxdma_entry(pd_rawrcb_t* rawrcb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_rawr_rx_stage0_entry(rawrcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

hal_ret_t 
p4pd_get_rawr_rx_stage0_entry(pd_rawrcb_t* rawrcb_pd)
{
    common_p4plus_stage0_app_header_table_d data = {0};
    rawrcb_t                                *rawrcb;

    // hardware index for this entry
    rawrcb_hw_id_t hwid = rawrcb_pd->hw_id + 
        (P4PD_RAWRCB_STAGE_ENTRY_OFFSET * P4PD_HWID_RAWR_RX_STAGE0);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for RAWRCB");
        return HAL_RET_HW_FAIL;
    }
    rawrcb = rawrcb_pd->rawrcb;
    rawrcb->chain_rxq_base = data.u.rawr_rx_start_d.chain_rxq_base;
    rawrcb->chain_rxq_ring_indices_addr = data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr;
    rawrcb->chain_rxq_ring_size_shift = data.u.rawr_rx_start_d.chain_rxq_ring_size_shift;
    rawrcb->chain_rxq_entry_size_shift = data.u.rawr_rx_start_d.chain_rxq_entry_size_shift;
    rawrcb->chain_rxq_ring_index_select = data.u.rawr_rx_start_d.chain_rxq_ring_index_select;

    rawrcb->chain_txq_base = data.u.rawr_rx_start_d.chain_txq_base;
    rawrcb->chain_txq_ring_indices_addr = data.u.rawr_rx_start_d.chain_txq_ring_indices_addr;
    rawrcb->chain_txq_ring_size_shift = data.u.rawr_rx_start_d.chain_txq_ring_size_shift;
    rawrcb->chain_txq_entry_size_shift = data.u.rawr_rx_start_d.chain_txq_entry_size_shift;
    rawrcb->chain_txq_ring_index_select = data.u.rawr_rx_start_d.chain_txq_ring_index_select;
    rawrcb->chain_txq_lif = data.u.rawr_rx_start_d.chain_txq_lif;
    rawrcb->chain_txq_qtype = data.u.rawr_rx_start_d.chain_txq_qtype;
    rawrcb->chain_txq_qid = data.u.rawr_rx_start_d.chain_txq_qid;
    rawrcb->chain_txq_doorbell_no_sched = data.u.rawr_rx_start_d.chain_txq_doorbell_no_sched;

    rawrcb->desc_valid_bit_upd = FALSE;
    rawrcb->desc_valid_bit_req = data.u.rawr_rx_start_d.desc_valid_bit_req;

    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_rawrcb_rxdma_entry(pd_rawrcb_t* rawrcb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;
    
    ret = p4pd_get_rawr_rx_stage0_entry(rawrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rawr_rx entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t 
p4pd_add_or_del_rawrcb_txdma_entry(pd_rawrcb_t* rawrcb_pd, bool del)
{
    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_rawrcb_txdma_entry(pd_rawrcb_t* rawrcb_pd)
{
    /* TODO */
    return HAL_RET_OK;
}

/**************************/

rawrcb_hw_id_t
pd_rawrcb_get_base_hw_index(pd_rawrcb_t* rawrcb_pd)
{
    HAL_ASSERT(NULL != rawrcb_pd);
    HAL_ASSERT(NULL != rawrcb_pd->rawrcb);
    
    // Get the base address of RAWR CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset. 
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_APP_REDIR, APP_REDIR_RAWR_QTYPE, 0);
    HAL_TRACE_DEBUG("RAWRCB received offset 0x{0:x}", offset);
    return offset + \
        (rawrcb_pd->rawrcb->cb_id * P4PD_HBM_RAWR_CB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_rawrcb_entry(pd_rawrcb_t* rawrcb_pd, bool del) 
{
    hal_ret_t                   ret = HAL_RET_OK;
 
    ret = p4pd_add_or_del_rawrcb_rxdma_entry(rawrcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }
   
    ret = p4pd_add_or_del_rawrcb_txdma_entry(rawrcb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }

err:
    return ret;
}

static
hal_ret_t
p4pd_get_rawrcb_entry(pd_rawrcb_t* rawrcb_pd) 
{
    hal_ret_t                   ret = HAL_RET_OK;
    
    ret = p4pd_get_rawrcb_rxdma_entry(rawrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rxdma entry for rawrcb");
        goto err;    
    }
   
    ret = p4pd_get_rawrcb_txdma_entry(rawrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for rawrcb");
        goto err;    
    }

err:
    /*TODO: cleanup */
    return ret;
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
pd_rawrcb_create (pd_rawrcb_args_t *args)
{
    hal_ret_t                ret;
    pd_rawrcb_s              *rawrcb_pd;

    HAL_TRACE_DEBUG("Creating pd state for RAWRCB.");

    // allocate PD rawrcb state
    rawrcb_pd = rawrcb_pd_alloc_init();
    if (rawrcb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    rawrcb_pd->rawrcb = args->rawrcb;
    // get hw-id for this RAWRCB
    rawrcb_pd->hw_id = pd_rawrcb_get_base_hw_index(rawrcb_pd);
    printf("RAWRCB{%u} Received hw-id: 0x%lx ",
           rawrcb_pd->rawrcb->cb_id, rawrcb_pd->hw_id);
    
    // program rawrcb
    ret = p4pd_add_or_del_rawrcb_entry(rawrcb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    // add to db
    ret = add_rawrcb_pd_to_db(rawrcb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->rawrcb->pd = rawrcb_pd;

    return HAL_RET_OK;

cleanup:

    if (rawrcb_pd) {
        rawrcb_pd_free(rawrcb_pd);
    }
    return ret;
}

hal_ret_t
pd_rawrcb_update (pd_rawrcb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawrcb_t*                rawrcb = args->rawrcb;
    pd_rawrcb_t*             rawrcb_pd = (pd_rawrcb_t*)rawrcb->pd;

    HAL_TRACE_DEBUG("RAWRCB pd update");
    
    // program rawrcb
    ret = p4pd_add_or_del_rawrcb_entry(rawrcb_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update rawrcb");
    }
    return ret;
}

hal_ret_t
pd_rawrcb_delete (pd_rawrcb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawrcb_t*                rawrcb = args->rawrcb;
    pd_rawrcb_t*             rawrcb_pd = (pd_rawrcb_t*)rawrcb->pd;

    HAL_TRACE_DEBUG("RAWRCB pd delete");
    
    // program rawrcb
    ret = p4pd_add_or_del_rawrcb_entry(rawrcb_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete rawrcb entry"); 
    }
    
    del_rawrcb_pd_from_db(rawrcb_pd);

    rawrcb_pd_free(rawrcb_pd);

    return ret;
}

hal_ret_t
pd_rawrcb_get (pd_rawrcb_args_t *args)
{
    hal_ret_t                ret;
    pd_rawrcb_t              rawrcb_pd;

    HAL_TRACE_DEBUG("RAWRCB pd get for id: {}", args->rawrcb->cb_id);

    // allocate PD rawrcb state
    rawrcb_pd_init(&rawrcb_pd);
    rawrcb_pd.rawrcb = args->rawrcb;
    
    // get hw-id for this RAWRCB
    rawrcb_pd.hw_id = pd_rawrcb_get_base_hw_index(&rawrcb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{0:x}", rawrcb_pd.hw_id);

    // get hw rawrcb entry
    ret = p4pd_get_rawrcb_entry(&rawrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", rawrcb_pd.rawrcb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
