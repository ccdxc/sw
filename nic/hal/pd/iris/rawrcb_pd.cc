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

/*
 * Number of CAPRI_QSTATE_HEADER_COMMON bytes that should stay constant,
 * i.e., not overwritten, during a CB update. Ideally this should come
 * from an offsetof(common_p4plus_stage0_app_header_table_d) but that is not
 * available due to bit fields usage in common_p4plus_stage0_app_header_table_d.
 */
#define RAWRCB_QSTATE_HEADER_TOTAL_SIZE     \
    (CAPRI_QSTATE_HEADER_COMMON_SIZE +      \
     (HAL_NUM_RAWRCB_RINGS_MAX * CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE))
     

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
p4pd_rawr_wring_eval(uint32_t qid,
                     types::WRingType wring_type,
                     wring_hw_id_t& wring_base,
                     uint8_t& ring_size_shift,
                     uint8_t& entry_size_shift)
{
    pd_wring_meta_t     *pd_wring_meta;
    hal_ret_t           ret;

    pd_wring_meta = wring_pd_get_meta(wring_type);
    ret = wring_pd_get_base_addr(wring_type, qid, &wring_base);
    if (!pd_wring_meta) {
        ret = HAL_RET_WRING_NOT_FOUND;
    }
    if (ret == HAL_RET_OK) {
        ring_size_shift  = log2(pd_wring_meta->num_slots);
        entry_size_shift = log2(pd_wring_meta->slot_size_in_bytes);
    }

    return ret;
}

static hal_ret_t 
p4pd_add_or_del_rawr_rx_stage0_entry(pd_rawrcb_t* rawrcb_pd,
                                     bool del,
                                     bool qstate_header_overwrite)
{
    common_p4plus_stage0_app_header_table_d     data = {0};
    uint8_t                                     *data_p = (uint8_t *)&data;
    rawrcb_t                                    *rawrcb;
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    wring_hw_id_t                               arq_base;
    uint32_t                                    data_len = sizeof(data);
    rawrcb_hw_addr_t                            hw_addr = rawrcb_pd->hw_addr;
    uint8_t                                     ring_size_shift;
    uint8_t                                     entry_size_shift;

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to an RxQ, or
     *   2) Redirect to a P4+ TxQ
     */
    rawrcb = rawrcb_pd->rawrcb;
    if (rawrcb->chain_txq_base) {
        assert(!rawrcb->chain_rxq_base);
        data.u.rawr_rx_start_d.chain_txq_base = rawrcb->chain_txq_base;
        data.u.rawr_rx_start_d.chain_txq_ring_indices_addr = rawrcb->chain_txq_ring_indices_addr;
        data.u.rawr_rx_start_d.chain_txq_ring_size_shift = rawrcb->chain_txq_ring_size_shift;
        data.u.rawr_rx_start_d.chain_txq_entry_size_shift = rawrcb->chain_txq_entry_size_shift;
        data.u.rawr_rx_start_d.chain_txq_ring_index_select = rawrcb->chain_txq_ring_index_select;
        data.u.rawr_rx_start_d.chain_txq_lif = rawrcb->chain_txq_lif;
        data.u.rawr_rx_start_d.chain_txq_qtype = rawrcb->chain_txq_qtype;
        data.u.rawr_rx_start_d.chain_txq_qid = rawrcb->chain_txq_qid;

    } else {
        data.u.rawr_rx_start_d.chain_rxq_base = rawrcb->chain_rxq_base;
        data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr = rawrcb->chain_rxq_ring_indices_addr;
        data.u.rawr_rx_start_d.chain_rxq_ring_size_shift = rawrcb->chain_rxq_ring_size_shift;
        data.u.rawr_rx_start_d.chain_rxq_entry_size_shift = rawrcb->chain_rxq_entry_size_shift;
        data.u.rawr_rx_start_d.chain_rxq_ring_index_select = rawrcb->chain_rxq_ring_index_select;
        if (!rawrcb->chain_rxq_base) {

            /*
             * Provide reasonable defaults for above
             */
            ret = p4pd_rawr_wring_eval(rawrcb->chain_rxq_ring_index_select,
                                       types::WRING_TYPE_ARQRX,
                                       arq_base, ring_size_shift, entry_size_shift);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("{} wring info not found for WRING_TYPE_ARQRX",
                              __FUNCTION__);
                goto done;
            }

            data.u.rawr_rx_start_d.chain_rxq_base = arq_base;
            data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr =
                                   get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);
            data.u.rawr_rx_start_d.chain_rxq_ring_size_shift  = ring_size_shift;
            data.u.rawr_rx_start_d.chain_rxq_entry_size_shift = entry_size_shift;

            HAL_TRACE_DEBUG("RAWRCB {} ring: {} arq_base: {:#x}", rawrcb->cb_id,
                            rawrcb->chain_rxq_ring_index_select, arq_base);
            HAL_TRACE_DEBUG("RAWRCB chain_rxq_ring_indices_addr: {:#x} "
                            "chain_rxq_ring_size_shift: {} "
                            "chain_rxq_entry_size_shift: {} "
                            "chain_rxq_ring_index_select: {}",
                            data.u.rawr_rx_start_d.chain_rxq_ring_indices_addr,
                            data.u.rawr_rx_start_d.chain_rxq_ring_size_shift,
                            data.u.rawr_rx_start_d.chain_rxq_entry_size_shift,
                            data.u.rawr_rx_start_d.chain_rxq_ring_index_select);
        }
    }

    /*
     * desc_valid_bit_req defaults to true unless specifically
     * updated by the caller.
     */
    data.u.rawr_rx_start_d.rawrcb_flags = rawrcb->rawrcb_flags |
                                          APP_REDIR_DESC_VALID_BIT_REQ;
    if (rawrcb->rawrcb_flags & APP_REDIR_DESC_VALID_BIT_UPD) {
        data.u.rawr_rx_start_d.rawrcb_flags = rawrcb->rawrcb_flags;
    }

    /*
     * check to see if qstate area should be overwritten
     */
    if (qstate_header_overwrite) {
        if(p4pd_get_rawr_rx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
            goto done;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("RAWRCB programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.rawr_rx_start_d.total = HAL_NUM_RAWRCB_RINGS_MAX;

    } else {
        hw_addr  += RAWRCB_QSTATE_HEADER_TOTAL_SIZE;
        data_p   += RAWRCB_QSTATE_HEADER_TOTAL_SIZE;
        data_len -= RAWRCB_QSTATE_HEADER_TOTAL_SIZE;
    }

    /*
     * Deactivate on request or in error case
     */
    data.u.rawr_rx_start_d.rawrcb_deactivate = RAWRCB_DEACTIVATE;
    data.u.rawr_rx_start_d.rawrcb_activate = (uint8_t)~RAWRCB_ACTIVATE;
    if (!del && (ret == HAL_RET_OK)) {
        data.u.rawr_rx_start_d.rawrcb_deactivate = (uint8_t)~RAWRCB_DEACTIVATE;
        data.u.rawr_rx_start_d.rawrcb_activate = RAWRCB_ACTIVATE;
    }

    HAL_TRACE_DEBUG("RAWRCB Programming stage0 at hw_addr: 0x{0:x}", hw_addr); 
    if (!p4plus_hbm_write(hw_addr, data_p, data_len)){
        HAL_TRACE_ERR("Failed to create rx: stage0 entry for RAWRCB");
        ret = HAL_RET_HW_FAIL;
    }

done:
    return ret;
}

hal_ret_t 
p4pd_add_or_del_rawrcb_rxdma_entry(pd_rawrcb_t* rawrcb_pd,
                                   bool del,
                                   bool qstate_header_overwrite)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_rawr_rx_stage0_entry(rawrcb_pd, del,
                                               qstate_header_overwrite);
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
    rawrcb_hw_addr_t hw_addr = rawrcb_pd->hw_addr;

    if(!p4plus_hbm_read(hw_addr, (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for RAWRCB");
        return HAL_RET_HW_FAIL;
    }
    rawrcb = rawrcb_pd->rawrcb;
    rawrcb->rawrcb_flags = data.u.rawr_rx_start_d.rawrcb_flags;
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

/**************************/

rawrcb_hw_addr_t
pd_rawrcb_get_base_hw_addr(pd_rawrcb_t* rawrcb_pd)
{
    HAL_ASSERT(NULL != rawrcb_pd);
    
    // Get the base address of RAWR CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset. 
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_APP_REDIR,
                                                      APP_REDIR_RAWR_QTYPE, 0);
    HAL_TRACE_DEBUG("RAWRCB received offset 0x{0:x}", offset);
    return offset + \
        (rawrcb_pd->hw_id * P4PD_HBM_RAWRCB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_rawrcb_entry(pd_rawrcb_t* rawrcb_pd,
                             bool del,
                             bool qstate_header_overwrite)
{
    hal_ret_t   ret = HAL_RET_OK;
 
    ret = p4pd_add_or_del_rawrcb_rxdma_entry(rawrcb_pd, del,
                                             qstate_header_overwrite);
    if(ret != HAL_RET_OK) {
        goto err;    
    }
err:
    return ret;
}

static hal_ret_t
p4pd_get_rawrcb_entry(pd_rawrcb_t* rawrcb_pd) 
{
    hal_ret_t                   ret = HAL_RET_OK;
    
    ret = p4pd_get_rawrcb_rxdma_entry(rawrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rxdma entry for rawrcb");
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
    hal_ret_t               ret;
    pd_rawrcb_s             *rawrcb_pd;
    rawrcb_hw_id_t          hw_id = args->rawrcb->cb_id;
    bool                    qstate_header_overwrite = false;

    HAL_TRACE_DEBUG("RAWRCB pd create for id: {}", hw_id);

    rawrcb_pd = find_rawrcb_by_hwid(hw_id);
    if (!rawrcb_pd) {

        // allocate PD rawrcb state
        qstate_header_overwrite = true;
        rawrcb_pd = rawrcb_pd_alloc_init(hw_id);
        if (rawrcb_pd == NULL) {
            ret = HAL_RET_OOM;
            goto cleanup;    
        }
    }

    rawrcb_pd->rawrcb = args->rawrcb;
    rawrcb_pd->hw_addr = pd_rawrcb_get_base_hw_addr(rawrcb_pd);
    printf("RAWRCB{%u} Received hw_addr: 0x%lx ",
           hw_id, rawrcb_pd->hw_addr);
    
    // program rawrcb
    ret = p4pd_add_or_del_rawrcb_entry(rawrcb_pd, false,
                                       qstate_header_overwrite);
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

/*
 * Deactivate a RAWRCB: fetch current HW programmed values
 * and deactivate the CB by changing only the sentinel values
 * rawrcb_deactivate and rawrcb_activate.
 */
static hal_ret_t
pd_rawrcb_deactivate (pd_rawrcb_args_t *args)
{
    hal_ret_t           ret;
    rawrcb_t            curr_rawrcb;
    pd_rawrcb_t         curr_rawrcb_pd;
    pd_rawrcb_args_t    curr_args;
    rawrcb_t*           rawrcb = args->rawrcb;

    pd_rawrcb_args_init(&curr_args);
    curr_args.rawrcb = &curr_rawrcb;
    curr_rawrcb.cb_id = rawrcb->cb_id;

    rawrcb_pd_init(&curr_rawrcb_pd, rawrcb->cb_id);
    curr_rawrcb_pd.rawrcb = &curr_rawrcb;

    HAL_TRACE_DEBUG("RAWRCB pd deactivate for id: {}", rawrcb->cb_id);
    
    ret = pd_rawrcb_get(&curr_args);
    if (ret == HAL_RET_OK) {
        ret = p4pd_add_or_del_rawrcb_entry(&curr_rawrcb_pd, true, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to deactivate rawrcb entry"); 
        }
    }
    
    return ret;
}

hal_ret_t
pd_rawrcb_update (pd_rawrcb_args_t *args)
{
    hal_ret_t       ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawrcb_t       *rawrcb = args->rawrcb;
    pd_rawrcb_t    *rawrcb_pd = (pd_rawrcb_t*)rawrcb->pd;

    HAL_TRACE_DEBUG("RAWRCB pd update for id: {}", rawrcb_pd->hw_id);
    
    /*
     * First, deactivate the current rawrcb
     */
    ret = pd_rawrcb_deactivate(args);
    if (ret == HAL_RET_OK) {

        // program rawrcb
        ret = p4pd_add_or_del_rawrcb_entry(rawrcb_pd, false, true);
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update rawrcb");
    }
    return ret;
}

hal_ret_t
pd_rawrcb_delete (pd_rawrcb_args_t *args,
                  bool retain_in_db)
{
    hal_ret_t       ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawrcb_t        *rawrcb = args->rawrcb;
    pd_rawrcb_t     *rawrcb_pd = (pd_rawrcb_t*)rawrcb->pd;

    HAL_TRACE_DEBUG("RAWRCB pd delete for id: {}", rawrcb_pd->hw_id);
    
    ret = p4pd_add_or_del_rawrcb_entry(rawrcb_pd, true, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete rawrcb entry"); 
    }
    
    if (!retain_in_db) {
        del_rawrcb_pd_from_db(rawrcb_pd);
        rawrcb_pd_free(rawrcb_pd);
    }
    return ret;
}

hal_ret_t
pd_rawrcb_get (pd_rawrcb_args_t *args)
{
    hal_ret_t       ret;
    pd_rawrcb_t     rawrcb_pd;
    rawrcb_hw_id_t  hw_id = args->rawrcb->cb_id;

    HAL_TRACE_DEBUG("RAWRCB pd get for id: {}", hw_id);

    // allocate PD rawrcb state
    rawrcb_pd_init(&rawrcb_pd, hw_id);
    rawrcb_pd.rawrcb = args->rawrcb;
    
    rawrcb_pd.hw_addr = pd_rawrcb_get_base_hw_addr(&rawrcb_pd);
    HAL_TRACE_DEBUG("Received hw_addr 0x{0:x}", rawrcb_pd.hw_addr);

    // get hw rawrcb entry
    ret = p4pd_get_rawrcb_entry(&rawrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", hw_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
