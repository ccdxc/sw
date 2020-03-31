#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "platform/capri/capri_common.hpp"
#include "nic/hal/pd/iris/internal/rawccb_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/rawc_txdma/include/rawc_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

/*
 * Number of CAPRI_QSTATE_HEADER_COMMON bytes that should stay constant,
 * i.e., not overwritten, during a CB update. Ideally this should come
 * from an offsetof(rawc_tx_start_d) but that is not available due to
 * bit fields usage in rawc_tx_start_d.
 */
#define RAWCCB_QSTATE_HEADER_TOTAL_SIZE     \
    (CAPRI_QSTATE_HEADER_COMMON_SIZE +      \
     (HAL_NUM_RAWCCB_RINGS_MAX * CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE))


namespace hal {
namespace pd {

void *
rawccb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_rawccb_t *)entry)->hw_id);
}

uint32_t
rawccb_pd_hw_key_size ()
{
    return sizeof(rawccb_hw_id_t);
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t
p4pd_get_rawc_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "rawc_tx_stage0";

    int ret = sdk::p4::p4_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret != 0) {
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Received offset for stage0 program: {:#x}", *offset);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_clear_rawc_stats_entry(pd_rawccb_t* rawccb_pd)
{
    s7_tbl2_metrics0_commit_d   data = {0};

    // hardware index for this entry
    rawccb_hw_addr_t hw_addr = rawccb_pd->hw_addr +
                               RAWCCB_TABLE_METRICS0_OFFSET;

    if(!p4plus_hbm_write(hw_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write stats entry for RAWCCB");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_rawc_wring_eval(uint32_t qid,
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
p4pd_add_or_del_rawc_tx_stage0_entry(pd_rawccb_t* rawccb_pd,
                                     bool del,
                                     bool qstate_header_overwrite)
{
    s0_tbl_d            data = {0};
    uint8_t             *data_p = (uint8_t *)&data;
    rawccb_t            *rawccb;
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t            pc_offset = 0;
    wring_hw_id_t       my_txq_base;
    uint32_t            data_len = sizeof(data);
    rawccb_hw_addr_t    hw_addr = rawccb_pd->hw_addr;
    uint8_t             ring_size_shift;
    uint8_t             entry_size_shift;

    rawccb = rawccb_pd->rawccb;
    data.u.rawc_tx_start_d.my_txq_base = rawccb->my_txq_base;
    data.u.rawc_tx_start_d.my_txq_ring_size_shift = rawccb->my_txq_ring_size_shift;
    data.u.rawc_tx_start_d.my_txq_entry_size_shift = rawccb->my_txq_entry_size_shift;

    if (!rawccb->my_txq_base) {

        /*
         * Provide reasonable defaults for above
         */
        ret = p4pd_rawc_wring_eval(rawccb->cb_id, types::WRING_TYPE_APP_REDIR_RAWC,
                                   my_txq_base, ring_size_shift, entry_size_shift);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} wring info not found for WRING_TYPE_APP_REDIR_RAWC",
                          __FUNCTION__);
            goto done;
        }

        data.u.rawc_tx_start_d.my_txq_base = my_txq_base;
        data.u.rawc_tx_start_d.my_txq_ring_size_shift  = ring_size_shift;
        data.u.rawc_tx_start_d.my_txq_entry_size_shift = entry_size_shift;

        HAL_TRACE_DEBUG("RAWCCB {} my_txq_base: {:#x}",
                        rawccb->cb_id, my_txq_base);
        HAL_TRACE_DEBUG("RAWCCB my_txq_ring_size_shift: {} "
                        "my_txq_entry_size_shift: {}",
                        data.u.rawc_tx_start_d.my_txq_ring_size_shift,
                        data.u.rawc_tx_start_d.my_txq_entry_size_shift);
    }

    data.u.rawc_tx_start_d.chain_txq_base = rawccb->chain_txq_base;
    data.u.rawc_tx_start_d.chain_txq_ring_indices_addr = rawccb->chain_txq_ring_indices_addr;
    data.u.rawc_tx_start_d.chain_txq_ring_size_shift = rawccb->chain_txq_ring_size_shift;
    data.u.rawc_tx_start_d.chain_txq_entry_size_shift = rawccb->chain_txq_entry_size_shift;
    data.u.rawc_tx_start_d.chain_txq_lif = rawccb->chain_txq_lif;
    data.u.rawc_tx_start_d.chain_txq_qtype = rawccb->chain_txq_qtype;
    data.u.rawc_tx_start_d.chain_txq_qid = rawccb->chain_txq_qid;
    data.u.rawc_tx_start_d.chain_txq_ring = rawccb->chain_txq_ring;
    data.u.rawc_tx_start_d.rawccb_flags = rawccb->rawccb_flags;
    data.u.rawc_tx_start_d.cpu_id = rawccb->cpu_id;

    /*
     * check to see if qstate area should be overwritten
     */
    if (qstate_header_overwrite) {
        if(p4pd_get_rawc_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
            goto done;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("RAWCCB programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.rawc_tx_start_d.total = HAL_NUM_RAWCCB_RINGS_MAX;

        /*
         * Note that similar to qstate, CB stats are cleared only once.
         */
        if (!del) {
            ret = p4pd_clear_rawc_stats_entry(rawccb_pd);
        }

    } else {
        hw_addr  += RAWCCB_QSTATE_HEADER_TOTAL_SIZE;
        data_p   += RAWCCB_QSTATE_HEADER_TOTAL_SIZE;
        data_len -= RAWCCB_QSTATE_HEADER_TOTAL_SIZE;
    }

    /*
     * Deactivate on request or in error case
     */
    data.u.rawc_tx_start_d.rawccb_deactivate = RAWCCB_DEACTIVATE;
    data.u.rawc_tx_start_d.rawccb_activate = (uint8_t)~RAWCCB_ACTIVATE;
    if (!del && (ret == HAL_RET_OK)) {
        data.u.rawc_tx_start_d.rawccb_deactivate = (uint8_t)~RAWCCB_DEACTIVATE;
        data.u.rawc_tx_start_d.rawccb_activate = RAWCCB_ACTIVATE;
    }

    HAL_TRACE_DEBUG("RAWCCB Programming stage0 at hw_addr: {:#x}", hw_addr);
    if (!p4plus_hbm_write(hw_addr, data_p, data_len,
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: stage0 entry for RAWCCB");
        ret = HAL_RET_HW_FAIL;
    }

done:
    return ret;
}

static hal_ret_t
p4pd_add_or_del_rawc_tx_stage1_entry(pd_rawccb_t* rawccb_pd,
                                     bool del)
{
    s1_tbl1_d           data = {0};
    rawccb_t            *rawccb;
    hal_ret_t           ret = HAL_RET_OK;
    rawccb_hw_addr_t    hw_addr;
    wring_hw_id_t       ascq_base;

    rawccb = rawccb_pd->rawccb;
    if (!del) {
        data.u.cb_extra_read_d.ascq_base = rawccb->ascq_base;;
        data.u.cb_extra_read_d.ascq_sem_inf_addr = rawccb->ascq_sem_inf_addr;
        if (!data.u.cb_extra_read_d.ascq_base) {

            /*
             * Provide reasonable defaults for above
             */
            ret = wring_pd_get_base_addr(types::WRING_TYPE_ASCQ,
                                         rawccb->cpu_id, &ascq_base);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("{} base_addr not found for WRING_TYPE_ASCQ for cpu_id {}",
                              __FUNCTION__, rawccb->cpu_id);
                goto done;
            }
            data.u.cb_extra_read_d.ascq_base = ascq_base;
            data.u.cb_extra_read_d.ascq_sem_inf_addr =
                                   CAPRI_SEM_ASCQ_INF_ADDR(rawccb->cpu_id);
        }
    }

    HAL_TRACE_DEBUG("RAWCCB {} ascq_base: {:#x} ascq_sem_inf_addr: {:#x}",
                    rawccb->cb_id, data.u.cb_extra_read_d.ascq_base,
                    data.u.cb_extra_read_d.ascq_sem_inf_addr);

    hw_addr = rawccb_pd->hw_addr + RAWCCB_TABLE_EXTRA_OFFSET;
    HAL_TRACE_DEBUG("RAWCCB Programming stage1 at hw_addr: {:#x}", hw_addr);
    if (!p4plus_hbm_write(hw_addr, (uint8_t *)&data, sizeof(data),
                          P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: stage1 entry for RAWCCB");
        ret = HAL_RET_HW_FAIL;
    }

done:
    return ret;
}

hal_ret_t
p4pd_get_rawccb_tx_stage0_entry(pd_rawccb_t* rawccb_pd)
{
    s0_tbl_d    data = {0};
    rawccb_t    *rawccb;

    // hardware index for this entry
    rawccb_hw_addr_t    hw_addr = rawccb_pd->hw_addr;

    if(sdk::asic::asic_mem_read(hw_addr,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: stage0 entry for RAWCCB");
        return HAL_RET_HW_FAIL;
    }
    rawccb = rawccb_pd->rawccb;
    rawccb->rawccb_flags = data.u.rawc_tx_start_d.rawccb_flags;
    rawccb->my_txq_base = data.u.rawc_tx_start_d.my_txq_base;
    rawccb->my_txq_ring_size_shift = data.u.rawc_tx_start_d.my_txq_ring_size_shift;
    rawccb->my_txq_entry_size_shift = data.u.rawc_tx_start_d.my_txq_entry_size_shift;

    rawccb->chain_txq_base = data.u.rawc_tx_start_d.chain_txq_base;
    rawccb->chain_txq_ring_indices_addr = data.u.rawc_tx_start_d.chain_txq_ring_indices_addr;
    rawccb->chain_txq_ring_size_shift = data.u.rawc_tx_start_d.chain_txq_ring_size_shift;
    rawccb->chain_txq_entry_size_shift = data.u.rawc_tx_start_d.chain_txq_entry_size_shift;
    rawccb->chain_txq_lif = data.u.rawc_tx_start_d.chain_txq_lif;
    rawccb->chain_txq_qtype = data.u.rawc_tx_start_d.chain_txq_qtype;
    rawccb->chain_txq_qid = data.u.rawc_tx_start_d.chain_txq_qid;
    rawccb->chain_txq_ring = data.u.rawc_tx_start_d.chain_txq_ring;
    rawccb->cpu_id = data.u.rawc_tx_start_d.cpu_id;

    rawccb->pi = data.u.rawc_tx_start_d.pi_0;
    rawccb->ci = data.u.rawc_tx_start_d.ci_0;

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_rawccb_tx_stage1_entry(pd_rawccb_t* rawccb_pd)
{
    s1_tbl1_d   data = {0};
    rawccb_t    *rawccb;

    // hardware index for this entry
    rawccb_hw_addr_t    hw_addr = rawccb_pd->hw_addr + RAWCCB_TABLE_EXTRA_OFFSET;

    if(sdk::asic::asic_mem_read(hw_addr,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: stage1 entry for RAWCCB");
        return HAL_RET_HW_FAIL;
    }
    rawccb = rawccb_pd->rawccb;
    rawccb->ascq_base = data.u.cb_extra_read_d.ascq_base;
    rawccb->ascq_sem_inf_addr = data.u.cb_extra_read_d.ascq_sem_inf_addr;

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_rawc_stats_entry(pd_rawccb_t* rawccb_pd)
{
    s7_tbl2_metrics0_commit_d   data;
    rawccb_t                    *rawccb;

    // hardware index for this entry
    rawccb_hw_addr_t hw_addr = rawccb_pd->hw_addr +
                               RAWCCB_TABLE_METRICS0_OFFSET;

    if(sdk::asic::asic_mem_read(hw_addr, (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get stats entry for RAWCCB");
        return HAL_RET_HW_FAIL;
    }
    rawccb = rawccb_pd->rawccb;
    rawccb->chain_pkts = data.chain_pkts;
    rawccb->cb_not_ready_discards = data.cb_not_ready_discards;
    rawccb->qstate_cfg_discards = data.qstate_cfg_discards;
    rawccb->aol_error_discards = data.aol_error_discards;
    rawccb->my_txq_empty_discards = data.my_txq_empty_discards;
    rawccb->txq_full_discards = data.txq_full_discards;
    rawccb->pkt_free_errors = data.pkt_free_errors;

    return HAL_RET_OK;
}

hal_ret_t
p4pd_add_or_del_rawccb_txdma_entry(pd_rawccb_t* rawccb_pd,
                                   bool del,
                                   bool qstate_header_overwrite)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_rawc_tx_stage0_entry(rawccb_pd, del,
                                               qstate_header_overwrite);
    if (ret == HAL_RET_OK) {
        ret = p4pd_add_or_del_rawc_tx_stage1_entry(rawccb_pd, del);
    }
    if (ret != HAL_RET_OK) {
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
    if (ret == HAL_RET_OK) {
        ret = p4pd_get_rawccb_tx_stage1_entry(rawccb_pd);
    }
    if (ret == HAL_RET_OK) {
        ret = p4pd_get_rawc_stats_entry(rawccb_pd);
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rawc_tx entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/**************************/

rawccb_hw_addr_t
pd_rawccb_get_base_hw_addr(pd_rawccb_t* rawccb_pd)
{
    SDK_ASSERT(NULL != rawccb_pd);

    // Get the base address of RAWC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    // uint64_t offset = lif_manager()->GetLIFQStateAddr(SERVICE_LIF_APP_REDIR,
    uint64_t offset = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_APP_REDIR,
                                                         APP_REDIR_RAWC_QTYPE, 0);
    HAL_TRACE_DEBUG("RAWCCB received offset {:#x}", offset);
    return offset + \
        (rawccb_pd->hw_id * P4PD_HBM_RAWCCB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_rawccb_entry(pd_rawccb_t* rawccb_pd,
                             bool del,
                             bool qstate_header_overwrite)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_rawccb_txdma_entry(rawccb_pd, del,
                                             qstate_header_overwrite);
    if(ret != HAL_RET_OK) {
        goto err;
    }

err:
    return ret;
}

static hal_ret_t
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
pd_rawccb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_rawccb_create_args_t *args = pd_func_args->pd_rawccb_create;
    pd_rawccb_s             *rawccb_pd;
    rawccb_hw_id_t          hw_id = args->rawccb->cb_id;
    bool                    qstate_header_overwrite = false;

    HAL_TRACE_DEBUG("RAWCCB pd create for id: {}", hw_id);

    rawccb_pd = find_rawccb_by_hwid(hw_id);
    if (!rawccb_pd) {

        // allocate PD rawccb state
        qstate_header_overwrite = true;
        rawccb_pd = rawccb_pd_alloc_init(hw_id);
        if (rawccb_pd == NULL) {
            ret = HAL_RET_OOM;
            goto cleanup;
        }
    }

    rawccb_pd->rawccb = args->rawccb;
    rawccb_pd->hw_addr = pd_rawccb_get_base_hw_addr(rawccb_pd);
    printf("RAWCCB{%u} Received hw_addr: 0x%lx ",
           hw_id, rawccb_pd->hw_addr);

    // program rawccb
    ret = p4pd_add_or_del_rawccb_entry(rawccb_pd, false,
                                       qstate_header_overwrite);
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
pd_rawccb_deactivate (pd_rawccb_update_args_t *args)
{
    hal_ret_t           ret;
    rawccb_t            curr_rawccb;
    pd_rawccb_t         curr_rawccb_pd;
    pd_rawccb_get_args_t    curr_args;
    rawccb_t*           rawccb = args->rawccb;
    pd_func_args_t      pd_func_args = {0};

    pd_rawccb_get_args_init(&curr_args);
    curr_args.rawccb = &curr_rawccb;
    curr_rawccb.cb_id = rawccb->cb_id;

    rawccb_pd_init(&curr_rawccb_pd, rawccb->cb_id);
    curr_rawccb_pd.rawccb = &curr_rawccb;

    curr_rawccb_pd.hw_addr = pd_rawccb_get_base_hw_addr(&curr_rawccb_pd);
    HAL_TRACE_DEBUG("RAWCCB pd deactivate for id: {} hw_addr {:#x}",
                    rawccb->cb_id,  curr_rawccb_pd.hw_addr);
    pd_func_args.pd_rawccb_get = &curr_args;
    ret = pd_rawccb_get(&pd_func_args);
    if (ret == HAL_RET_OK) {
        ret = p4pd_add_or_del_rawccb_entry(&curr_rawccb_pd, true, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to deactivate rawccb entry");
        }
    }

    return ret;
}

hal_ret_t
pd_rawccb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_rawccb_update_args_t *args = pd_func_args->pd_rawccb_update;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    rawccb_t        *rawccb = args->rawccb;
    pd_rawccb_t     *rawccb_pd = (pd_rawccb_t*)rawccb->pd;

    HAL_TRACE_DEBUG("RAWCCB pd update for id: {}", rawccb_pd->hw_id);

    /*
     * First, deactivate the current rawccb
     */
    ret = pd_rawccb_deactivate(args);
    if (ret == HAL_RET_OK) {

        // program rawccb
        ret = p4pd_add_or_del_rawccb_entry(rawccb_pd, false, false);
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update rawccb");
    }
    return ret;
}

hal_ret_t
#if 0
pd_rawccb_delete (pd_rawccb_args_t *args,
                  bool retain_in_db)
#endif
pd_rawccb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_rawccb_delete_args_t *del_args = pd_func_args->pd_rawccb_delete;
    pd_rawccb_args_t *args = del_args->r_args;
    bool retain_in_db = del_args->retain_in_db;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    rawccb_t        *rawccb = args->rawccb;
    pd_rawccb_t     *rawccb_pd = (pd_rawccb_t*)rawccb->pd;

    HAL_TRACE_DEBUG("RAWCCB pd delete for id: {}", rawccb_pd->hw_id);

    ret = p4pd_add_or_del_rawccb_entry(rawccb_pd, true, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete rawccb entry");
    }

    if (!retain_in_db) {
        del_rawccb_pd_from_db(rawccb_pd);
        rawccb_pd_free(rawccb_pd);
    }

    return ret;
}

hal_ret_t
pd_rawccb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_rawccb_get_args_t *args = pd_func_args->pd_rawccb_get;
    pd_rawccb_t     rawccb_pd;
    rawccb_hw_id_t  hw_id = args->rawccb->cb_id;

    HAL_TRACE_DEBUG("RAWCCB pd get for id: {}", hw_id);

    // allocate PD rawccb state
    rawccb_pd_init(&rawccb_pd, hw_id);
    rawccb_pd.rawccb = args->rawccb;

    rawccb_pd.hw_addr = pd_rawccb_get_base_hw_addr(&rawccb_pd);
    HAL_TRACE_DEBUG("Received hw_addr {:#x}", rawccb_pd.hw_addr);

    // get hw rawccb entry
    ret = p4pd_get_rawccb_entry(&rawccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: {:#x}", hw_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
