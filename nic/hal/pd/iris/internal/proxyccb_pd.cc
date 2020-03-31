#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/proxyccb_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/proxyc_txdma/include/proxyc_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

/*
 * Number of CAPRI_QSTATE_HEADER_COMMON bytes that should stay constant,
 * i.e., not overwritten, during a CB update. Ideally this should come
 * from an offsetof(proxyc_tx_start_d) but that is not available due to
 * bit fields usage in proxyc_tx_start_d.
 */
#define PROXYCCB_QSTATE_HEADER_TOTAL_SIZE   \
    (CAPRI_QSTATE_HEADER_COMMON_SIZE +      \
     (HAL_NUM_PROXYCCB_RINGS_MAX * CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE))


namespace hal {
namespace pd {

void *
proxyccb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_proxyccb_t *)entry)->hw_id);
}

uint32_t
proxyccb_pd_hw_key_size ()
{
    return sizeof(proxyccb_hw_id_t);
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t
p4pd_get_proxyc_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "proxyc_tx_stage0";

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
p4pd_clear_proxyc_stats_entry(pd_proxyccb_t* proxyccb_pd)
{
    proxyc_stats_err_stat_inc_d   data = {0};

    // hardware index for this entry
    proxyccb_hw_addr_t hw_addr = proxyccb_pd->hw_addr +
                                 PROXYCCB_TABLE_STATS_OFFSET;

    if(!p4plus_hbm_write(hw_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write stats entry for PROXYCCB");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_proxyc_wring_eval(uint32_t qid,
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
p4pd_add_or_del_proxyc_tx_stage0_entry(pd_proxyccb_t* proxyccb_pd,
                                       bool del,
                                       bool qstate_header_overwrite)
{
    proxyc_tx_start_d   data = {0};
    uint8_t             *data_p = (uint8_t *)&data;
    proxyccb_t          *proxyccb = proxyccb_pd->proxyccb;
    hal_ret_t           ret = HAL_RET_OK;
    uint64_t            pc_offset = 0;
    wring_hw_id_t       txq_base;
    uint32_t            data_len = sizeof(data);
    proxyccb_hw_addr_t  hw_addr = proxyccb_pd->hw_addr;
    uint16_t            lif;
    uint8_t             ring_size_shift;
    uint8_t             entry_size_shift;

    proxyccb = proxyccb_pd->proxyccb;
    data.u.start_d.redir_span = proxyccb->redir_span;
    data.u.start_d.my_txq_base = proxyccb->my_txq_base;
    data.u.start_d.my_txq_ring_size_shift = proxyccb->my_txq_ring_size_shift;
    data.u.start_d.my_txq_entry_size_shift = proxyccb->my_txq_entry_size_shift;

    if (!proxyccb->my_txq_base) {

        /*
         * Provide reasonable defaults for above
         */
        ret = p4pd_proxyc_wring_eval(proxyccb->cb_id, types::WRING_TYPE_APP_REDIR_PROXYC,
                                     txq_base, ring_size_shift, entry_size_shift);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} wring info not found for WRING_TYPE_APP_REDIR_PROXYC",
                          __FUNCTION__);
            goto done;
        }

        data.u.start_d.my_txq_base = txq_base;
        data.u.start_d.my_txq_ring_size_shift  = ring_size_shift;
        data.u.start_d.my_txq_entry_size_shift = entry_size_shift;

        HAL_TRACE_DEBUG("PROXYCCB {} my_txq_base: {:#x}",
                        proxyccb->cb_id, data.u.start_d.my_txq_base);
        HAL_TRACE_DEBUG("PROXYCCB my_txq_ring_size_shift: {} "
                        "my_txq_entry_size_shift: {}",
                        data.u.start_d.my_txq_ring_size_shift,
                        data.u.start_d.my_txq_entry_size_shift);
    }

    /*
     * Build out chain_txq info based on LIF provided in proxyccb->chain_txq_lif
     */
    data.u.start_d.chain_txq_base = proxyccb->chain_txq_base;
    data.u.start_d.chain_txq_ring_indices_addr = proxyccb->chain_txq_ring_indices_addr;
    data.u.start_d.chain_txq_lif = proxyccb->chain_txq_lif;
    data.u.start_d.chain_txq_qtype = proxyccb->chain_txq_qtype;
    data.u.start_d.chain_txq_qid = proxyccb->chain_txq_qid;
    data.u.start_d.chain_txq_ring = proxyccb->chain_txq_ring;

    if (!proxyccb->chain_txq_base) {
        lif = proxyccb->chain_txq_lif;
        assert((lif == SERVICE_LIF_TCP_PROXY) || (lif == SERVICE_LIF_TLS_PROXY));
        ret = p4pd_proxyc_wring_eval(proxyccb->chain_txq_qid,
                                     lif == SERVICE_LIF_TCP_PROXY ?
                                     types::WRING_TYPE_SESQ : types::WRING_TYPE_SERQ,
                                     txq_base, ring_size_shift, entry_size_shift);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} wring info not found for lif {}", __FUNCTION__,
                          proxyccb->chain_txq_lif);
            goto done;
        }

        data.u.start_d.chain_txq_base = txq_base;
        data.u.start_d.chain_txq_ring_indices_addr =
             // lif_manager()->GetLIFQStateAddr(lif, 0, proxyccb->chain_txq_qid) +
             lif_manager()->get_lif_qstate_addr(lif, 0, proxyccb->chain_txq_qid) +
             (CAPRI_QSTATE_HEADER_COMMON_SIZE +
              (proxyccb->chain_txq_ring * CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE));
        data.u.start_d.chain_txq_ring_size_shift  = ring_size_shift;
        data.u.start_d.chain_txq_entry_size_shift = entry_size_shift;
        HAL_TRACE_DEBUG("{} chain_txq_base: {:#x} ring_size_shift {} "
                        "entry_size_shift {}", __FUNCTION__,
                        txq_base, data.u.start_d.chain_txq_ring_size_shift,
                        data.u.start_d.chain_txq_entry_size_shift);
    }

    data.u.start_d.proxyccb_flags = proxyccb->proxyccb_flags;

    /*
     * check to see if qstate area should be overwritten
     */
    if (qstate_header_overwrite) {
        if(p4pd_get_proxyc_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
            goto done;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("PROXYCCB programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.start_d.total = HAL_NUM_PROXYCCB_RINGS_MAX;

        /*
         * Note that similar to qstate, CB stats are cleared only once.
         */
        if (!del) {
            ret = p4pd_clear_proxyc_stats_entry(proxyccb_pd);
        }

    } else {
        hw_addr  += PROXYCCB_QSTATE_HEADER_TOTAL_SIZE;
        data_p   += PROXYCCB_QSTATE_HEADER_TOTAL_SIZE;
        data_len -= PROXYCCB_QSTATE_HEADER_TOTAL_SIZE;
    }

    /*
     * Deactivate on request or in error case
     */
    data.u.start_d.proxyccb_deactivate = PROXYCCB_DEACTIVATE;
    data.u.start_d.proxyccb_activate = (uint8_t)~PROXYCCB_ACTIVATE;
    if (!del && (ret == HAL_RET_OK)) {
        data.u.start_d.proxyccb_deactivate = (uint8_t)~PROXYCCB_DEACTIVATE;
        data.u.start_d.proxyccb_activate = PROXYCCB_ACTIVATE;
    }

    HAL_TRACE_DEBUG("PROXYCCB Programming stage0 at hw_addr: {:#x}", hw_addr);
    if(!p4plus_hbm_write(hw_addr, data_p, data_len,
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: stage0 entry for PROXYCCB");
        ret = HAL_RET_HW_FAIL;
    }

done:
    return ret;
}

hal_ret_t
p4pd_get_proxyccb_tx_stage0_entry(pd_proxyccb_t* proxyccb_pd)
{
    proxyc_tx_start_d   data = {0};
    proxyccb_t          *proxyccb;

    // hardware index for this entry
    proxyccb_hw_addr_t    hw_addr = proxyccb_pd->hw_addr;

    if(sdk::asic::asic_mem_read(hw_addr,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: stage0 entry for PROXYCCB");
        return HAL_RET_HW_FAIL;
    }
    proxyccb = proxyccb_pd->proxyccb;
    proxyccb->redir_span = data.u.start_d.redir_span;
    proxyccb->proxyccb_flags = data.u.start_d.proxyccb_flags;
    proxyccb->my_txq_base = data.u.start_d.my_txq_base;
    proxyccb->my_txq_ring_size_shift = data.u.start_d.my_txq_ring_size_shift;
    proxyccb->my_txq_entry_size_shift = data.u.start_d.my_txq_entry_size_shift;

    proxyccb->chain_txq_base = data.u.start_d.chain_txq_base;
    proxyccb->chain_txq_ring_indices_addr = data.u.start_d.chain_txq_ring_indices_addr;
    proxyccb->chain_txq_ring_size_shift = data.u.start_d.chain_txq_ring_size_shift;
    proxyccb->chain_txq_entry_size_shift = data.u.start_d.chain_txq_entry_size_shift;
    proxyccb->chain_txq_lif = data.u.start_d.chain_txq_lif;
    proxyccb->chain_txq_qtype = data.u.start_d.chain_txq_qtype;
    proxyccb->chain_txq_qid = data.u.start_d.chain_txq_qid;
    proxyccb->chain_txq_ring = data.u.start_d.chain_txq_ring;

    proxyccb->pi = data.u.start_d.pi_0;
    proxyccb->ci = data.u.start_d.ci_0;

    HAL_TRACE_DEBUG("xxx: proxyccb pi/ci for qid {} is {}/{}",
            proxyccb->cb_id, proxyccb->pi, proxyccb->ci);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_proxyc_stats_entry(pd_proxyccb_t* proxyccb_pd)
{
    proxyc_stats_err_stat_inc_d   data;
    proxyccb_t                    *proxyccb;

    // hardware index for this entry
    proxyccb_hw_addr_t hw_addr = proxyccb_pd->hw_addr +
                                 PROXYCCB_TABLE_STATS_OFFSET;

    if(sdk::asic::asic_mem_read(hw_addr, (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get stats entry for PROXYCCB");
        return HAL_RET_HW_FAIL;
    }
    proxyccb = proxyccb_pd->proxyccb;
    proxyccb->stat_pkts_chain = ntohll(data.stat_pkts_chain);
    proxyccb->stat_pkts_discard = ntohll(data.stat_pkts_discard);
    proxyccb->stat_cb_not_ready = ntohl(data.stat_cb_not_ready);
    proxyccb->stat_my_txq_empty = ntohl(data.stat_my_txq_empty);
    proxyccb->stat_aol_err = ntohl(data.stat_aol_err);
    proxyccb->stat_txq_full = ntohl(data.stat_txq_full);
    proxyccb->stat_desc_sem_free_full = ntohl(data.stat_desc_sem_free_full);
    proxyccb->stat_page_sem_free_full = ntohl(data.stat_page_sem_free_full);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_add_or_del_proxyccb_txdma_entry(pd_proxyccb_t* proxyccb_pd,
                                     bool del,
                                     bool qstate_header_overwrite)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_proxyc_tx_stage0_entry(proxyccb_pd, del,
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
p4pd_get_proxyccb_txdma_entry(pd_proxyccb_t* proxyccb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_proxyccb_tx_stage0_entry(proxyccb_pd);
    if (ret == HAL_RET_OK) {
        ret = p4pd_get_proxyc_stats_entry(proxyccb_pd);
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get proxyc_tx entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/**************************/

proxyccb_hw_addr_t
pd_proxyccb_get_base_hw_addr(pd_proxyccb_t* proxyccb_pd)
{
    SDK_ASSERT(NULL != proxyccb_pd);

    // Get the base address of PROXYC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    // uint64_t offset = lif_manager()->GetLIFQStateAddr(SERVICE_LIF_APP_REDIR,
    uint64_t offset = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_APP_REDIR,
                                                         APP_REDIR_PROXYC_QTYPE, 0);
    HAL_TRACE_DEBUG("PROXYCCB received offset {:#x}", offset);
    return offset + \
        (proxyccb_pd->hw_id * P4PD_HBM_PROXYCCB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_proxyccb_entry(pd_proxyccb_t* proxyccb_pd,
                               bool del,
                               bool qstate_header_overwrite)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_proxyccb_txdma_entry(proxyccb_pd, del,
                                               qstate_header_overwrite);
    if(ret != HAL_RET_OK) {
        goto err;
    }

err:
    return ret;
}

static hal_ret_t
p4pd_get_proxyccb_entry(pd_proxyccb_t* proxyccb_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_proxyccb_txdma_entry(proxyccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for proxyccb");
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
pd_proxyccb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_proxyccb_create_args_t *args = pd_func_args->pd_proxyccb_create;
    pd_proxyccb_s           *proxyccb_pd;
    proxyccb_hw_id_t        hw_id = args->proxyccb->cb_id;
    bool                    qstate_header_overwrite = false;

    HAL_TRACE_DEBUG("PROXYCCB pd create for id: {}", hw_id);

    proxyccb_pd = find_proxyccb_by_hwid(hw_id);
    if (!proxyccb_pd) {

        // allocate PD proxyccb state
        qstate_header_overwrite = true;
        proxyccb_pd = proxyccb_pd_alloc_init(hw_id);
        if (proxyccb_pd == NULL) {
            ret = HAL_RET_OOM;
            goto cleanup;
        }
    }

    proxyccb_pd->proxyccb = args->proxyccb;
    proxyccb_pd->hw_addr = pd_proxyccb_get_base_hw_addr(proxyccb_pd);
    printf("PROXYCCB{%u} Received hw_addr: 0x%lx ",
           hw_id, proxyccb_pd->hw_addr);

    // program proxyccb
    ret = p4pd_add_or_del_proxyccb_entry(proxyccb_pd, false,
                                       qstate_header_overwrite);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    // add to db
    ret = add_proxyccb_pd_to_db(proxyccb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->proxyccb->pd = proxyccb_pd;

    return HAL_RET_OK;

cleanup:

    if (proxyccb_pd) {
        proxyccb_pd_free(proxyccb_pd);
    }
    return ret;
}

static hal_ret_t
pd_proxyccb_deactivate (pd_proxyccb_update_args_t *args)
{
    hal_ret_t           ret;
    proxyccb_t          curr_proxyccb;
    pd_proxyccb_t       curr_proxyccb_pd;
    pd_proxyccb_get_args_t  curr_args;
    pd_func_args_t      pd_func_args = {0};
    proxyccb_t*         proxyccb = args->proxyccb;

    pd_proxyccb_get_args_init(&curr_args);
    curr_args.proxyccb = &curr_proxyccb;
    curr_proxyccb.cb_id = proxyccb->cb_id;

    proxyccb_pd_init(&curr_proxyccb_pd, proxyccb->cb_id);
    curr_proxyccb_pd.proxyccb = &curr_proxyccb;

    curr_proxyccb_pd.hw_addr = pd_proxyccb_get_base_hw_addr(&curr_proxyccb_pd);
    HAL_TRACE_DEBUG("PROXYCCB pd deactivate for id: {} hw_addr {:#x}",
                    proxyccb->cb_id,  curr_proxyccb_pd.hw_addr);
    pd_func_args.pd_proxyccb_get = &curr_args;
    ret = pd_proxyccb_get(&pd_func_args);
    if (ret == HAL_RET_OK) {
        ret = p4pd_add_or_del_proxyccb_entry(&curr_proxyccb_pd, true, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to deactivate proxyccb entry");
        }
    }

    return ret;
}

hal_ret_t
pd_proxyccb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_proxyccb_update_args_t *args = pd_func_args->pd_proxyccb_update;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    proxyccb_t     *proxyccb = args->proxyccb;
    pd_proxyccb_t  *proxyccb_pd = (pd_proxyccb_t*)proxyccb->pd;

    HAL_TRACE_DEBUG("PROXYCCB pd update for id: {}", proxyccb_pd->hw_id);

    /*
     * First, deactivate the current proxyccb
     */
    ret = pd_proxyccb_deactivate(args);
    if (ret == HAL_RET_OK) {

        /*
         * program proxyccb
         * until TCP/TLS moves away from maintaining our PI in its CB,
         * we have to clear our PI/CI on update here
         */
        ret = p4pd_add_or_del_proxyccb_entry(proxyccb_pd, false, true);
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update proxyccb");
    }
    return ret;
}

hal_ret_t
#if 0
pd_proxyccb_delete (pd_proxyccb_args_t *args,
                    bool retain_in_db)
#endif
pd_proxyccb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_proxyccb_delete_args_t *del_args = pd_func_args->pd_proxyccb_delete;
    pd_proxyccb_args_t *args = del_args->r_args;
    bool retain_in_db = del_args->retain_in_db;


    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    proxyccb_t      *proxyccb = args->proxyccb;
    pd_proxyccb_t   *proxyccb_pd = (pd_proxyccb_t*)proxyccb->pd;

    HAL_TRACE_DEBUG("PROXYCCB pd delete for id: {}", proxyccb_pd->hw_id);

    ret = p4pd_add_or_del_proxyccb_entry(proxyccb_pd, true, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete proxyccb entry");
    }

    if (!retain_in_db) {
        del_proxyccb_pd_from_db(proxyccb_pd);
        proxyccb_pd_free(proxyccb_pd);
    }

    return ret;
}

hal_ret_t
pd_proxyccb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_proxyccb_get_args_t *args = pd_func_args->pd_proxyccb_get;
    pd_proxyccb_t           proxyccb_pd;
    proxyccb_hw_id_t        hw_id = args->proxyccb->cb_id;

    HAL_TRACE_DEBUG("PROXYCCB pd get for id: {}", hw_id);

    // allocate PD proxyccb state
    proxyccb_pd_init(&proxyccb_pd, hw_id);
    proxyccb_pd.proxyccb = args->proxyccb;

    proxyccb_pd.hw_addr = pd_proxyccb_get_base_hw_addr(&proxyccb_pd);
    HAL_TRACE_DEBUG("Received hw_addr {:#x}", proxyccb_pd.hw_addr);

    // get hw proxyccb entry
    ret = p4pd_get_proxyccb_entry(&proxyccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: {:#x}", hw_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
