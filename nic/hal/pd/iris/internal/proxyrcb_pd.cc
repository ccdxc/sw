#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "platform/capri/capri_common.hpp"
#include "nic/hal/pd/iris/internal/proxyrcb_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/proxyr_txdma/include/proxyr_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

/*
 * Number of CAPRI_QSTATE_HEADER_COMMON bytes that should stay constant,
 * i.e., not overwritten, during a CB update. Ideally this should come
 * from an offsetof(proxyr_tx_start_d) but that is not
 * available due to bit fields usage in proxyr_tx_start_d.
 */
#define PROXYRCB_QSTATE_HEADER_TOTAL_SIZE   \
    (CAPRI_QSTATE_HEADER_COMMON_SIZE +      \
     (HAL_NUM_PROXYRCB_RINGS_MAX * CAPRI_QSTATE_HEADER_RING_SINGLE_SIZE))


namespace hal {
namespace pd {

void *
proxyrcb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_proxyrcb_t *)entry)->hw_id);
}

uint32_t
proxyrcb_pd_hw_key_size ()
{
    return sizeof(proxyrcb_hw_id_t);
}

/********************************************
 * txdma
 * ******************************************/

hal_ret_t
p4pd_get_proxyr_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "proxyr_tx_stage0";

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
p4pd_clear_proxyr_stats_entry(pd_proxyrcb_t* proxyrcb_pd)
{
    proxyr_stats_err_stat_inc_d data = {0};

    // hardware index for this entry
    proxyrcb_hw_addr_t hw_addr = proxyrcb_pd->hw_addr +
                                 PROXYRCB_TABLE_STATS_OFFSET;

    if(!p4plus_hbm_write(hw_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write stats entry for PROXYRCB");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

static hal_ret_t
p4pd_proxyr_wring_eval(uint32_t qid,
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
p4pd_add_or_del_proxyr_tx_stage0_entry(pd_proxyrcb_t* proxyrcb_pd,
                                       bool del,
                                       bool qstate_header_overwrite)
{
    proxyr_tx_start_d     data = {0};
    uint8_t               *data_p = (uint8_t *)&data;
    proxyrcb_t            *proxyrcb;
    hal_ret_t             ret = HAL_RET_OK;
    uint64_t              pc_offset = 0;
    wring_hw_id_t         arq_base;
    wring_hw_id_t         txq_base;
    uint32_t              data_len = sizeof(data);
    proxyrcb_hw_addr_t    hw_addr = proxyrcb_pd->hw_addr;
    uint8_t               ring_size_shift;
    uint8_t               entry_size_shift;

    proxyrcb = proxyrcb_pd->proxyrcb;
    data.u.start_d.redir_span = proxyrcb->redir_span;
    data.u.start_d.my_txq_base = proxyrcb->my_txq_base;
    data.u.start_d.my_txq_ring_size_shift = proxyrcb->my_txq_ring_size_shift;
    data.u.start_d.my_txq_entry_size_shift = proxyrcb->my_txq_entry_size_shift;

    if (!proxyrcb->my_txq_base) {

        /*
         * Provide reasonable defaults for above
         */
        ret = p4pd_proxyr_wring_eval(proxyrcb->cb_id, types::WRING_TYPE_APP_REDIR_PROXYR,
                                     txq_base, ring_size_shift, entry_size_shift);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} wring info not found for WRING_TYPE_APP_REDIR_PROXYR",
                          __FUNCTION__);
            goto done;
        }

        data.u.start_d.my_txq_base = txq_base;
        data.u.start_d.my_txq_ring_size_shift  = ring_size_shift;
        data.u.start_d.my_txq_entry_size_shift = entry_size_shift;

        HAL_TRACE_DEBUG("PROXYCCB {} my_txq_base: {:#x}",
                        proxyrcb->cb_id, data.u.start_d.my_txq_base);
        HAL_TRACE_DEBUG("PROXYCCB my_txq_ring_size_shift: {} "
                        "my_txq_entry_size_shift: {}",
                        data.u.start_d.my_txq_ring_size_shift,
                        data.u.start_d.my_txq_entry_size_shift);
    }

    data.u.start_d.chain_rxq_base = proxyrcb->chain_rxq_base;
    data.u.start_d.chain_rxq_ring_indices_addr = proxyrcb->chain_rxq_ring_indices_addr;
    data.u.start_d.chain_rxq_ring_size_shift = proxyrcb->chain_rxq_ring_size_shift;
    data.u.start_d.chain_rxq_entry_size_shift = proxyrcb->chain_rxq_entry_size_shift;
    data.u.start_d.chain_rxq_ring_index_select = proxyrcb->chain_rxq_ring_index_select;

    if (!proxyrcb->chain_rxq_base) {

        /*
         * Provide reasonable defaults for above
         */
        ret = p4pd_proxyr_wring_eval(proxyrcb->chain_rxq_ring_index_select,
                                     types::WRING_TYPE_ARQRX,
                                     arq_base, ring_size_shift, entry_size_shift);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("{} wring info not found for WRING_TYPE_ARQRX",
                          __FUNCTION__);
            goto done;
        }

        HAL_TRACE_DEBUG("PROXYRCB {} ring: {} arq_base: {:#x}", proxyrcb->cb_id,
                        proxyrcb->chain_rxq_ring_index_select, arq_base);
        data.u.start_d.chain_rxq_base = arq_base;
        data.u.start_d.chain_rxq_ring_indices_addr =
               CAPRI_SEM_ARQ_INF_ADDR(proxyrcb->chain_rxq_ring_index_select);
        data.u.start_d.chain_rxq_ring_size_shift = ring_size_shift;
        data.u.start_d.chain_rxq_entry_size_shift = entry_size_shift;
        HAL_TRACE_DEBUG("PROXYRCB chain_rxq_ring_indices_addr: {:#x} "
                        "chain_rxq_ring_size_shift: {} "
                        "chain_rxq_entry_size_shift: {} "
                        "chain_rxq_ring_index_select: {}",
                        data.u.start_d.chain_rxq_ring_indices_addr,
                        data.u.start_d.chain_rxq_ring_size_shift,
                        data.u.start_d.chain_rxq_entry_size_shift,
                        data.u.start_d.chain_rxq_ring_index_select);
    }

    /*
     * desc_valid_bit_req defaults to true unless specifically
     * updated by the caller.
     */
    data.u.start_d.proxyrcb_flags = proxyrcb->proxyrcb_flags |
                                    APP_REDIR_DESC_VALID_BIT_REQ;
    if (proxyrcb->proxyrcb_flags & APP_REDIR_DESC_VALID_BIT_UPD) {
        data.u.start_d.proxyrcb_flags = proxyrcb->proxyrcb_flags;
    }

    /*
     * check to see if qstate area should be overwritten
     */
    if (qstate_header_overwrite) {
        if(p4pd_get_proxyr_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
            goto done;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("PROXYRCB programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.start_d.total = HAL_NUM_PROXYRCB_RINGS_MAX;

        /*
         * Note that similar to qstate, CB stats are cleared only once.
         */
        if (!del) {
            ret = p4pd_clear_proxyr_stats_entry(proxyrcb_pd);
        }

    } else {
        hw_addr  += PROXYRCB_QSTATE_HEADER_TOTAL_SIZE;
        data_p   += PROXYRCB_QSTATE_HEADER_TOTAL_SIZE;
        data_len -= PROXYRCB_QSTATE_HEADER_TOTAL_SIZE;
    }

    /*
     * Deactivate on request or in error case
     */
    data.u.start_d.proxyrcb_deactivate = PROXYRCB_DEACTIVATE;
    if (!del && (ret == HAL_RET_OK)) {
        data.u.start_d.proxyrcb_deactivate = (uint8_t)~PROXYRCB_DEACTIVATE;
    }

    HAL_TRACE_DEBUG("PROXYRCB Programming stage0 at hw_addr: {:#x}", hw_addr);
    if (!p4plus_hbm_write(hw_addr, data_p, data_len,
            P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create tx: stage0 entry for PROXYRCB");
        ret = HAL_RET_HW_FAIL;
    }

done:
    return ret;
}

static hal_ret_t
p4pd_add_or_del_proxyr_flow_key_entry(pd_proxyrcb_t* proxyrcb_pd,
                                      bool del)
{
    proxyr_flow_key_d     data = {0};
    proxyrcb_t            *proxyrcb;
    hal_ret_t             ret = HAL_RET_OK;

    // hardware index for this entry
    proxyrcb_hw_addr_t hw_addr = proxyrcb_pd->hw_addr +
                                 PROXYRCB_TABLE_FLOW_KEY_OFFSET;
    /*
     * Caller must have invoked pd_proxyrcb_get() to get current
     * programmed values for the delete case. We keep all values
     * intact and only modify the sentinel values.
     */
    data.u.flow_key_post_read_d.proxyrcb_activate = (uint8_t)~PROXYRCB_ACTIVATE;
    if (!del) {
        proxyrcb = proxyrcb_pd->proxyrcb;

        /*
         * Always copy the entire address regardless of AF
         */
        memcpy(&data.u.flow_key_post_read_d.ip_sa, &proxyrcb->ip_sa,
               sizeof(data.u.flow_key_post_read_d.ip_sa));
        memcpy(&data.u.flow_key_post_read_d.ip_da, &proxyrcb->ip_da,
               sizeof(data.u.flow_key_post_read_d.ip_da));
        data.u.flow_key_post_read_d.sport    = proxyrcb->sport;
        data.u.flow_key_post_read_d.dport    = proxyrcb->dport;
        data.u.flow_key_post_read_d.vrf      = proxyrcb->vrf;
        data.u.flow_key_post_read_d.af       = proxyrcb->af;
        data.u.flow_key_post_read_d.ip_proto = proxyrcb->ip_proto;

        data.u.flow_key_post_read_d.proxyrcb_activate = PROXYRCB_ACTIVATE;
    }

    HAL_TRACE_DEBUG("PROXYRCB Programming flow_key at hw_addr: {:#x}", hw_addr);
    if (!p4plus_hbm_write(hw_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create flow_key entry for PROXYRCB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_proxyrcb_txdma_entry(pd_proxyrcb_t* proxyrcb_pd,
                                     bool del,
                                     bool qstate_header_overwrite)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_proxyr_tx_stage0_entry(proxyrcb_pd, del,
                                                 qstate_header_overwrite);
    if (ret == HAL_RET_OK) {
        ret = p4pd_add_or_del_proxyr_flow_key_entry(proxyrcb_pd, del);
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
p4pd_get_proxyr_tx_stage0_entry(pd_proxyrcb_t* proxyrcb_pd)
{
    proxyr_tx_start_d   data = {0};
    proxyrcb_t          *proxyrcb;

    // hardware index for this entry
    proxyrcb_hw_addr_t hw_addr = proxyrcb_pd->hw_addr;

    if (sdk::asic::asic_mem_read(hw_addr, (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for PROXYRCB");
        return HAL_RET_HW_FAIL;
    }
    proxyrcb = proxyrcb_pd->proxyrcb;
    proxyrcb->redir_span = data.u.start_d.redir_span;
    proxyrcb->proxyrcb_flags = data.u.start_d.proxyrcb_flags;
    proxyrcb->my_txq_base = data.u.start_d.my_txq_base;
    proxyrcb->my_txq_ring_size_shift = data.u.start_d.my_txq_ring_size_shift;
    proxyrcb->my_txq_entry_size_shift = data.u.start_d.my_txq_entry_size_shift;
    proxyrcb->chain_rxq_base = data.u.start_d.chain_rxq_base;
    proxyrcb->chain_rxq_ring_indices_addr = data.u.start_d.chain_rxq_ring_indices_addr;
    proxyrcb->chain_rxq_ring_size_shift = data.u.start_d.chain_rxq_ring_size_shift;
    proxyrcb->chain_rxq_entry_size_shift = data.u.start_d.chain_rxq_entry_size_shift;
    proxyrcb->chain_rxq_ring_index_select = data.u.start_d.chain_rxq_ring_index_select;

    proxyrcb->pi = data.u.start_d.pi_0;
    proxyrcb->ci = data.u.start_d.ci_0;

    HAL_TRACE_DEBUG("yyy: proxyrcb pi/ci for qid {} is {}/{}",
            proxyrcb->cb_id, proxyrcb->pi, proxyrcb->ci);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_proxyr_flow_key_entry(pd_proxyrcb_t* proxyrcb_pd)
{
    proxyr_flow_key_d   data = {0};
    proxyrcb_t          *proxyrcb;

    // hardware index for this entry
    proxyrcb_hw_addr_t hw_addr = proxyrcb_pd->hw_addr +
                                 PROXYRCB_TABLE_FLOW_KEY_OFFSET;

    if (sdk::asic::asic_mem_read(hw_addr, (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for PROXYRCB");
        return HAL_RET_HW_FAIL;
    }

    /*
     * Always copy the entire address regardless of AF
     */
    proxyrcb = proxyrcb_pd->proxyrcb;
    memcpy(&proxyrcb->ip_sa, &data.u.flow_key_post_read_d.ip_sa,
           sizeof(proxyrcb->ip_sa));
    memcpy(&proxyrcb->ip_da, &data.u.flow_key_post_read_d.ip_da,
           sizeof(proxyrcb->ip_da));
    proxyrcb->sport    = data.u.flow_key_post_read_d.sport;
    proxyrcb->dport    = data.u.flow_key_post_read_d.dport;
    proxyrcb->vrf      = data.u.flow_key_post_read_d.vrf;
    proxyrcb->af       = data.u.flow_key_post_read_d.af;
    proxyrcb->ip_proto = data.u.flow_key_post_read_d.ip_proto;

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_proxyr_stats_entry(pd_proxyrcb_t* proxyrcb_pd)
{
    proxyr_stats_err_stat_inc_d data;
    proxyrcb_t                  *proxyrcb;

    // hardware index for this entry
    proxyrcb_hw_addr_t hw_addr = proxyrcb_pd->hw_addr +
                                 PROXYRCB_TABLE_STATS_OFFSET;

    if(sdk::asic::asic_mem_read(hw_addr, (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get stats entry for PROXYRCB");
        return HAL_RET_HW_FAIL;
    }
    proxyrcb = proxyrcb_pd->proxyrcb;
    proxyrcb->stat_pkts_redir = ntohll(data.stat_pkts_redir);
    proxyrcb->stat_pkts_discard = ntohll(data.stat_pkts_discard);
    proxyrcb->stat_cb_not_ready = ntohl(data.stat_cb_not_ready);
    proxyrcb->stat_null_ring_indices_addr = ntohl(data.stat_null_ring_indices_addr);
    proxyrcb->stat_aol_err = ntohl(data.stat_aol_err);
    proxyrcb->stat_rxq_full = ntohl(data.stat_rxq_full);
    proxyrcb->stat_txq_empty = ntohl(data.stat_txq_empty);
    proxyrcb->stat_sem_alloc_full = ntohl(data.stat_sem_alloc_full);
    proxyrcb->stat_sem_free_full = ntohl(data.stat_sem_free_full);

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_proxyrcb_txdma_entry(pd_proxyrcb_t* proxyrcb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_proxyr_tx_stage0_entry(proxyrcb_pd);
    if (ret == HAL_RET_OK) {
        ret = p4pd_get_proxyr_flow_key_entry(proxyrcb_pd);
    }
    if (ret == HAL_RET_OK) {
        ret = p4pd_get_proxyr_stats_entry(proxyrcb_pd);
    }
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get proxyr_rx entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/**************************/

proxyrcb_hw_addr_t
pd_proxyrcb_get_base_hw_addr(pd_proxyrcb_t* proxyrcb_pd)
{
    SDK_ASSERT(NULL != proxyrcb_pd);

    // Get the base address of PROXYR CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    // uint64_t offset = lif_manager()->GetLIFQStateAddr(SERVICE_LIF_APP_REDIR,
    uint64_t offset = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_APP_REDIR,
                                                         APP_REDIR_PROXYR_QTYPE, 0);
    HAL_TRACE_DEBUG("PROXYRCB received offset {:#x}", offset);
    return offset + \
           (proxyrcb_pd->hw_id * P4PD_HBM_PROXYRCB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_proxyrcb_entry(pd_proxyrcb_t* proxyrcb_pd,
                               bool del,
                               bool qstate_header_overwrite)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_proxyrcb_txdma_entry(proxyrcb_pd, del,
                                               qstate_header_overwrite);
    if(ret != HAL_RET_OK) {
        goto err;
    }
err:
    return ret;
}

static hal_ret_t
p4pd_get_proxyrcb_entry(pd_proxyrcb_t* proxyrcb_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_proxyrcb_txdma_entry(proxyrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for proxyrcb");
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
pd_proxyrcb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_proxyrcb_create_args_t *args = pd_func_args->pd_proxyrcb_create;
    pd_proxyrcb_s           *proxyrcb_pd;
    proxyrcb_hw_id_t        hw_id = args->proxyrcb->cb_id;
    bool                    qstate_header_overwrite = false;

    HAL_TRACE_DEBUG("PROXYRCB pd create for id: {}", hw_id);

    proxyrcb_pd = find_proxyrcb_by_hwid(hw_id);
    if (!proxyrcb_pd) {

        // allocate PD proxyrcb state
        qstate_header_overwrite = true;
        proxyrcb_pd = proxyrcb_pd_alloc_init(hw_id);
        if (proxyrcb_pd == NULL) {
            ret = HAL_RET_OOM;
            goto cleanup;
        }
    }

    proxyrcb_pd->proxyrcb = args->proxyrcb;
    proxyrcb_pd->hw_addr = pd_proxyrcb_get_base_hw_addr(proxyrcb_pd);
    printf("PROXYRCB{%u} Received hw_addr: 0x%lx ",
           hw_id, proxyrcb_pd->hw_addr);

    // program proxyrcb
    ret = p4pd_add_or_del_proxyrcb_entry(proxyrcb_pd, false,
                                       qstate_header_overwrite);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    // add to db
    ret = add_proxyrcb_pd_to_db(proxyrcb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->proxyrcb->pd = proxyrcb_pd;

    return HAL_RET_OK;

cleanup:

    if (proxyrcb_pd) {
        proxyrcb_pd_free(proxyrcb_pd);
    }
    return ret;
}

/*
 * Deactivate a PROXYRCB: fetch current HW programmed values
 * and deactivate the CB by changing only the sentinel values
 * proxyrcb_deactivate and proxyrcb_activate.
 */
static hal_ret_t
pd_proxyrcb_deactivate (pd_proxyrcb_update_args_t *args)
{
    hal_ret_t           ret;
    proxyrcb_t          curr_proxyrcb;
    pd_proxyrcb_t       curr_proxyrcb_pd;
    pd_proxyrcb_get_args_t  curr_args;
    pd_func_args_t      pd_func_args = {0};
    proxyrcb_t*         proxyrcb = args->proxyrcb;

    pd_proxyrcb_get_args_init(&curr_args);
    curr_args.proxyrcb = &curr_proxyrcb;
    curr_proxyrcb.cb_id = proxyrcb->cb_id;

    proxyrcb_pd_init(&curr_proxyrcb_pd, proxyrcb->cb_id);
    curr_proxyrcb_pd.proxyrcb = &curr_proxyrcb;

    curr_proxyrcb_pd.hw_addr = pd_proxyrcb_get_base_hw_addr(&curr_proxyrcb_pd);
    HAL_TRACE_DEBUG("PROXYRCB pd deactivate for id: {} hw_addr {:#x}",
                    proxyrcb->cb_id,  curr_proxyrcb_pd.hw_addr);
    pd_func_args.pd_proxyrcb_get = &curr_args;
    ret = pd_proxyrcb_get(&pd_func_args);
    if (ret == HAL_RET_OK) {
        ret = p4pd_add_or_del_proxyrcb_entry(&curr_proxyrcb_pd, true, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to deactivate proxyrcb entry");
        }
    }

    return ret;
}

hal_ret_t
pd_proxyrcb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_proxyrcb_update_args_t *args = pd_func_args->pd_proxyrcb_update;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    proxyrcb_t      *proxyrcb = args->proxyrcb;
    pd_proxyrcb_t   *proxyrcb_pd = (pd_proxyrcb_t*)proxyrcb->pd;

    HAL_TRACE_DEBUG("PROXYRCB pd update for id: {}", proxyrcb_pd->hw_id);

    /*
     * First, deactivate the current proxyrcb before programming
     * new values.
     */
    ret = pd_proxyrcb_deactivate(args);
    if (ret == HAL_RET_OK) {

        /*
         * program proxyrcb
         * until TCP/TLS moves away from maintaining our PI in its CB,
         * we have to clear our PI/CI on update here
         */
        ret = p4pd_add_or_del_proxyrcb_entry(proxyrcb_pd, false, true);
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update proxyrcb");
    }
    return ret;
}

hal_ret_t
#if 0
pd_proxyrcb_delete (pd_proxyrcb_args_t *args,
                    bool retain_in_db)
#endif
pd_proxyrcb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret;
    pd_proxyrcb_delete_args_t *del_args = pd_func_args->pd_proxyrcb_delete;
    pd_proxyrcb_args_t *args = del_args->r_args;
    bool retain_in_db = del_args->retain_in_db;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    proxyrcb_t      *proxyrcb = args->proxyrcb;
    pd_proxyrcb_t   *proxyrcb_pd = (pd_proxyrcb_t*)proxyrcb->pd;

    HAL_TRACE_DEBUG("PROXYRCB pd delete for id: {}", proxyrcb_pd->hw_id);

    ret = p4pd_add_or_del_proxyrcb_entry(proxyrcb_pd, true, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete proxyrcb entry");
    }

    if (!retain_in_db) {
        del_proxyrcb_pd_from_db(proxyrcb_pd);
        proxyrcb_pd_free(proxyrcb_pd);
    }
    return ret;
}

hal_ret_t
pd_proxyrcb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_proxyrcb_get_args_t *args = pd_func_args->pd_proxyrcb_get;
    pd_proxyrcb_t           proxyrcb_pd;
    proxyrcb_hw_id_t        hw_id = args->proxyrcb->cb_id;

    HAL_TRACE_DEBUG("PROXYRCB pd get for id: {}", hw_id);

    // allocate PD proxyrcb state
    proxyrcb_pd_init(&proxyrcb_pd, hw_id);
    proxyrcb_pd.proxyrcb = args->proxyrcb;

    proxyrcb_pd.hw_addr = pd_proxyrcb_get_base_hw_addr(&proxyrcb_pd);
    HAL_TRACE_DEBUG("Received hw_addr {:#x}", proxyrcb_pd.hw_addr);

    // get hw proxyrcb entry
    ret = p4pd_get_proxyrcb_entry(&proxyrcb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: {:#x}", hw_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
