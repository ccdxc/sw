#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/rawccb_pd.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/wring_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/gen/rawc_txdma/include/rawc_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/p4plus_pd_api.h"

namespace hal {
namespace pd {

void *
rawccb_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_rawccb_t *)entry)->hw_id);
}

uint32_t
rawccb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(rawccb_hw_id_t)) % ht_size;
}

bool
rawccb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(rawccb_hw_id_t *)key1 == *(rawccb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t
p4pd_get_rawc_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rawc_tx_stage0";

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
p4pd_add_or_del_rawc_tx_stage0_entry(pd_rawccb_t* rawccb_pd, bool del)
{
    rawc_tx_start_d     data = {0};
    rawccb_t            *rawccb;
    pd_wring_meta_t     *pd_wring_meta;
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t            pc_offset = 0;
    wring_hw_id_t       my_txq_base;

    // hardware index for this entry
    rawccb_hw_id_t hwid = rawccb_pd->hw_id;

    /*
     * Caller must have invoked pd_rawccb_get() to get current
     * programmed values for the delete case. We keep all values
     * intact and only modify the sentinel values.
     */
    data.u.start_d.rawccb_deactivated = true;
    data.u.start_d.rawccb_activated = false;
    if (!del) {

        // get pc address
        if(p4pd_get_rawc_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("RAWCCB programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.start_d.total = 1;
        rawccb = rawccb_pd->rawccb;

        if (rawccb->my_txq_base) {
            data.u.start_d.my_txq_base = rawccb->my_txq_base;
            data.u.start_d.my_txq_ring_size_shift = rawccb->my_txq_ring_size_shift;
            data.u.start_d.my_txq_entry_size_shift = rawccb->my_txq_entry_size_shift;
        } else {

            /*
             * Provide reasonable defaults for above
             */
            pd_wring_meta = wring_pd_get_meta(types::WRING_TYPE_APP_REDIR_RAWC);
            ret = wring_pd_get_base_addr(types::WRING_TYPE_APP_REDIR_RAWC,
                                         rawccb->cb_id, &my_txq_base);
            if((ret == HAL_RET_OK) && pd_wring_meta) {
                HAL_TRACE_DEBUG("RAWCCB {} my_txq_base: {:#x}",
                                rawccb->cb_id, my_txq_base);
                data.u.start_d.my_txq_base = my_txq_base;
                data.u.start_d.my_txq_ring_size_shift =
                               log2(pd_wring_meta->num_slots);
                data.u.start_d.my_txq_entry_size_shift =
                               log2(pd_wring_meta->slot_size_in_bytes);
                HAL_TRACE_DEBUG("RAWCCB my_txq_ring_size_shift: {} "
                                "my_txq_entry_size_shift: {}",
                                data.u.start_d.my_txq_ring_size_shift,
                                data.u.start_d.my_txq_entry_size_shift);
            } else {
                HAL_TRACE_ERR("Failed to receive WRING_TYPE_APP_REDIR_RAWC");
                ret = HAL_RET_WRING_NOT_FOUND;
            }
        }

        data.u.start_d.chain_txq_base = rawccb->chain_txq_base;
        data.u.start_d.chain_txq_ring_indices_addr = rawccb->chain_txq_ring_indices_addr;
        data.u.start_d.chain_txq_ring_size_shift = rawccb->chain_txq_ring_size_shift;
        data.u.start_d.chain_txq_entry_size_shift = rawccb->chain_txq_entry_size_shift;
        data.u.start_d.chain_txq_lif = rawccb->chain_txq_lif;
        data.u.start_d.chain_txq_qtype = rawccb->chain_txq_qtype;
        data.u.start_d.chain_txq_qid = rawccb->chain_txq_qid;
        data.u.start_d.chain_txq_ring = rawccb->chain_txq_ring;
        data.u.start_d.rawccb_flags = rawccb->rawccb_flags;

        if (ret == HAL_RET_OK) {
            data.u.start_d.rawccb_deactivated = false;
            data.u.start_d.rawccb_activated = true;
        }
    }

    HAL_TRACE_DEBUG("RAWCCB Programming stage0 at hw-id: 0x{0:x}", hwid); 
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: stage0 entry for RAWCCB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t 
p4pd_get_rawccb_tx_stage0_entry(pd_rawccb_t* rawccb_pd)
{
    rawc_tx_start_d     data = {0};
    rawccb_t            *rawccb;

    // hardware index for this entry
    rawccb_hw_id_t hwid = rawccb_pd->hw_id;

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: stage0 entry for RAWCCB");
        return HAL_RET_HW_FAIL;
    }
    rawccb = rawccb_pd->rawccb;
    rawccb->rawccb_deactivated = data.u.start_d.rawccb_deactivated;
    rawccb->rawccb_flags = data.u.start_d.rawccb_flags;
    rawccb->my_txq_base = data.u.start_d.my_txq_base;
    rawccb->my_txq_ring_size_shift = data.u.start_d.my_txq_ring_size_shift;
    rawccb->my_txq_entry_size_shift = data.u.start_d.my_txq_entry_size_shift;

    rawccb->chain_txq_base = data.u.start_d.chain_txq_base;
    rawccb->chain_txq_ring_indices_addr = data.u.start_d.chain_txq_ring_indices_addr;
    rawccb->chain_txq_ring_size_shift = data.u.start_d.chain_txq_ring_size_shift;
    rawccb->chain_txq_entry_size_shift = data.u.start_d.chain_txq_entry_size_shift;
    rawccb->chain_txq_lif = data.u.start_d.chain_txq_lif;
    rawccb->chain_txq_qtype = data.u.start_d.chain_txq_qtype;
    rawccb->chain_txq_qid = data.u.start_d.chain_txq_qid;
    rawccb->chain_txq_ring = data.u.start_d.chain_txq_ring;
    rawccb->rawccb_activated = data.u.start_d.rawccb_activated;

    rawccb->pi = data.u.start_d.pi_0;
    rawccb->ci = data.u.start_d.ci_0;

    return HAL_RET_OK;
}

hal_ret_t 
p4pd_add_or_del_rawccb_txdma_entry(pd_rawccb_t* rawccb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_rawc_tx_stage0_entry(rawccb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

hal_ret_t 
p4pd_get_rawccb_txdma_entry(pd_rawccb_t* rawccb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;
    
    ret = p4pd_get_rawccb_tx_stage0_entry(rawccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rawc_tx entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/**************************/

rawccb_hw_id_t
pd_rawccb_get_base_hw_index(pd_rawccb_t* rawccb_pd)
{
    HAL_ASSERT(NULL != rawccb_pd);
    HAL_ASSERT(NULL != rawccb_pd->rawccb);
    
    // Get the base address of RAWC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset. 
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_APP_REDIR,
                                                      APP_REDIR_RAWC_QTYPE, 0);
    HAL_TRACE_DEBUG("RAWCCB received offset 0x{0:x}", offset);
    return offset + \
        (rawccb_pd->rawccb->cb_id * P4PD_HBM_RAWC_CB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_rawccb_entry(pd_rawccb_t* rawccb_pd, bool del) 
{
    hal_ret_t                   ret = HAL_RET_OK;
 
    ret = p4pd_add_or_del_rawccb_txdma_entry(rawccb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }

err:
    return ret;
}

static
hal_ret_t
p4pd_get_rawccb_entry(pd_rawccb_t* rawccb_pd) 
{
    hal_ret_t                   ret = HAL_RET_OK;
    
    ret = p4pd_get_rawccb_txdma_entry(rawccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for rawccb");
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
pd_rawccb_create (pd_rawccb_args_t *args)
{
    hal_ret_t                ret;
    pd_rawccb_s              *rawccb_pd;

    HAL_TRACE_DEBUG("Creating pd state for RAWCCB.");

    // allocate PD rawccb state
    rawccb_pd = rawccb_pd_alloc_init();
    if (rawccb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    rawccb_pd->rawccb = args->rawccb;
    // get hw-id for this RAWCCB
    rawccb_pd->hw_id = pd_rawccb_get_base_hw_index(rawccb_pd);
    printf("RAWCCB{%u} Received hw-id: 0x%lx ",
           rawccb_pd->rawccb->cb_id, rawccb_pd->hw_id);
    
    // program rawccb
    ret = p4pd_add_or_del_rawccb_entry(rawccb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    // add to db
    ret = add_rawccb_pd_to_db(rawccb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->rawccb->pd = rawccb_pd;

    return HAL_RET_OK;

cleanup:

    if (rawccb_pd) {
        rawccb_pd_free(rawccb_pd);
    }
    return ret;
}

static hal_ret_t
pd_rawccb_deactivate (pd_rawccb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawccb_t            old_rawccb;
    pd_rawccb_t         old_rawccb_pd;
    pd_rawccb_args_t    old_args;
    rawccb_t*           rawccb = args->rawccb;
    pd_rawccb_t*        rawccb_pd = (pd_rawccb_t*)rawccb->pd;

    memset(&old_args, 0, sizeof(old_args));
    old_rawccb.cb_id = rawccb->cb_id;
    old_args.rawccb = &old_rawccb;

    rawccb_pd_init(&old_rawccb_pd);
    old_rawccb_pd.rawccb = &old_rawccb;
    old_rawccb_pd.hw_id = rawccb_pd->hw_id;

    HAL_TRACE_DEBUG("RAWCCB pd deactivate");
    
    // fetch current programmed values
    ret = pd_rawccb_get(&old_args);
    if (ret == HAL_RET_OK) {
    
        // program rawccb
        ret = p4pd_add_or_del_rawccb_entry(&old_rawccb_pd, true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to deactivate rawccb entry"); 
        }
    }
    
    return ret;
}

hal_ret_t
pd_rawccb_update (pd_rawccb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawccb_t*                rawccb = args->rawccb;
    pd_rawccb_t*             rawccb_pd = (pd_rawccb_t*)rawccb->pd;

    HAL_TRACE_DEBUG("RAWCCB pd update");
    
    /*
     * First, deactivate the current rawccb
     */
    ret = pd_rawccb_deactivate(args);
    if (ret == HAL_RET_OK) {

        // program rawccb
        ret = p4pd_add_or_del_rawccb_entry(rawccb_pd, false);
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update rawccb");
    }
    return ret;
}

hal_ret_t
pd_rawccb_delete (pd_rawccb_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    rawccb_t*                rawccb = args->rawccb;
    pd_rawccb_t*             rawccb_pd = (pd_rawccb_t*)rawccb->pd;

    HAL_TRACE_DEBUG("RAWCCB pd delete");
    
    ret = pd_rawccb_deactivate(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete rawccb entry"); 
    }
    
    del_rawccb_pd_from_db(rawccb_pd);

    rawccb_pd_free(rawccb_pd);

    return ret;
}

hal_ret_t
pd_rawccb_get (pd_rawccb_args_t *args)
{
    hal_ret_t                ret;
    pd_rawccb_t              rawccb_pd;

    HAL_TRACE_DEBUG("RAWCCB pd get for id: {}", args->rawccb->cb_id);

    // allocate PD rawccb state
    rawccb_pd_init(&rawccb_pd);
    rawccb_pd.rawccb = args->rawccb;
    
    // get hw-id for this RAWCCB
    rawccb_pd.hw_id = pd_rawccb_get_base_hw_index(&rawccb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{0:x}", rawccb_pd.hw_id);

    // get hw rawccb entry
    ret = p4pd_get_rawccb_entry(&rawccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", rawccb_pd.rawccb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
