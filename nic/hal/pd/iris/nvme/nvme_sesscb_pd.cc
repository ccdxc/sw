//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "platform/capri/capri_txs_scheduler.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/hal.hpp"
#include "platform/capri/capri_common.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nvme_sesscb_pd.hpp"
#include "nvme_global_pd.hpp"
#include "nvme_pd.hpp"
#include "p4pd_nvme_api.h"
#include "nic/sdk/nvme/nvme_common.h"

namespace hal {
namespace pd {

extern pd_nvme_global_t *g_pd_nvme_global;

void *
nvme_sesscb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_nvme_sesscb_t *)entry)->hw_id);
}

bool
nvme_sesscb_pd_compare_hw_key_func (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(nvme_sesscb_hw_id_t *)key1 == *(nvme_sesscb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

uint32_t
nvme_sesscb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(nvme_sesscb_hw_id_t)) % ht_size;
}

uint64_t nvme_sesscb_pd_sesq_prod_ci_addr_get(uint32_t g_sess_id)
{
    uint64_t addr;

    addr = g_pd_nvme_global->txsessprodcb_base + g_sess_id * sizeof(nvme_txsessprodcb_t);
    //addr += offsetof(nvme_txsessprodcb_t, tcp_q_ci);
    addr += 28; //XXX

    return addr;
}

static uint64_t
get_nvme_tx_sessprodcb_addr (uint32_t g_sess_id)
{
    SDK_ASSERT(g_sess_id < g_pd_nvme_global->nvme_global->max_sess);
    return (g_pd_nvme_global->txsessprodcb_base + g_sess_id * sizeof(nvme_txsessprodcb_t));
}

hal_ret_t
get_program_offset (char *progname, char *labelname, uint64_t *offset)
{
    int ret = sdk::p4::p4_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret != 0) {
        return HAL_RET_HW_FAIL;
    }
    *offset >>= MPU_PC_ADDR_SHIFT;
    return HAL_RET_OK;
}


/********************************************
 * APIs
 *******************************************/

hal_ret_t
p4pd_add_or_del_txsessprodcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    s5_t0_nvme_req_tx_sessprodcb_process_bitfield_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t g_sess_id, lif_sess_id, sesq_qid;
    uint64_t tx_sess_xtsq_base;
    uint64_t tx_sess_dgstq_base;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;
    sesq_qid = nvme_sesscb_pd->nvme_sesscb->sesq_qid;

    data_addr = g_pd_nvme_global->txsessprodcb_base + g_sess_id * sizeof(nvme_txsessprodcb_t);

    // Get Tx Sess XTSQ base address
    tx_sess_xtsq_base = g_pd_nvme_global->tx_sess_xtsq_base + g_sess_id * NVME_TX_SESS_XTSQ_SIZE;

    data.xts_q_base_addr = tx_sess_xtsq_base;
    data.xts_q_pi = 0;
    data.xts_q_ci = 0;
    data.log_num_xts_q_entries = log2(NVME_TX_SESS_XTSQ_DEPTH);
    data.xts_qid = lif_sess_id;
    SDK_ASSERT((NVME_TX_SESS_XTSQ_DEPTH & (NVME_TX_SESS_XTSQ_DEPTH - 1)) == 0);

    // Get SESQ address of the TCP flow
    wring_hw_id_t  sesq_base;
    ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                 sesq_qid,
                                 &sesq_base);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive sesq base for tcp qid: {}",
                      sesq_qid);
    }

    uint32_t sesq_size;
    ret = wring_pd_get_num_entries(types::WRING_TYPE_SESQ,
                                   &sesq_size);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive sesq num_entries for tcp qid: {}",
                      sesq_qid);
    }

    HAL_TRACE_DEBUG("Sesq id: {:#x} Sesq base: {:#x}, size: {}",
                    sesq_qid, sesq_base, sesq_size);

    // Get Tx Sess DGSTQ base address
    tx_sess_dgstq_base = g_pd_nvme_global->tx_sess_dgstq_base + g_sess_id * NVME_TX_SESS_DGSTQ_SIZE;
    data.dgst_q_base_addr = tx_sess_dgstq_base;
    data.dgst_q_pi = 0;
    data.dgst_q_ci = 0;
    data.log_num_dgst_q_entries = log2(NVME_TX_SESS_DGSTQ_DEPTH);
    data.dgst_qid = lif_sess_id;
    data.tcp_q_base_addr = sesq_base;
    data.tcp_q_pi = 0;
    data.tcp_q_ci = 0;
    data.log_num_tcp_q_entries = log2(sesq_size);
    SDK_ASSERT((NVME_TX_SESS_DGSTQ_DEPTH & (NVME_TX_SESS_DGSTQ_DEPTH - 1)) == 0);

    HAL_TRACE_DEBUG("Programming txsessprodcb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write txsessprodcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_sessxtstxcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    s0_t0_nvme_sessprexts_tx_cb_process_bitfield_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id;
    uint64_t tx_sess_xtsq_base;
    uint64_t offset;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_TX_SESS_XTSQ, lif_sess_id);
    SDK_ASSERT(data_addr != 0);

    data.total = MAX_SESSXTSTX_RINGS;
    data.host = MAX_SESSXTSTX_HOST_RINGS;

    get_program_offset((char *)"txdma_stage0.bin",
                       (char *)"nvme_tx_sessxts_stage0",
                       &offset);
    data.rsvd = offset; //pc

    HAL_TRACE_DEBUG("sess_num: {}, tx sessxts pc: {:#x}", g_sess_id, offset);

    tx_sess_xtsq_base = g_pd_nvme_global->tx_sess_xtsq_base + g_sess_id * NVME_TX_SESS_XTSQ_SIZE;

    data.base_addr = tx_sess_xtsq_base;
    data.log_num_entries = log2(NVME_TX_SESS_XTSQ_DEPTH);

    data.session_id = g_sess_id;

    HAL_TRACE_DEBUG("Programming sessxtstxcb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write sessxtstxcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_sessdgsttxcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    s0_t0_nvme_sesspredgst_tx_cb_process_bitfield_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id;
    uint64_t tx_sess_dgstq_base;
    uint64_t offset;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_TX_SESS_DGSTQ, lif_sess_id);
    SDK_ASSERT(data_addr != 0);

    data.total = MAX_SESSDGSTTX_RINGS;
    data.host = MAX_SESSDGSTTX_HOST_RINGS;

    get_program_offset((char *)"txdma_stage0.bin",
                       (char *)"nvme_tx_sessdgst_stage0",
                       &offset);
    data.rsvd = offset; //pc

    HAL_TRACE_DEBUG("sess_num: {}, tx sessdgst pc: {:#x}", g_sess_id, offset);

    tx_sess_dgstq_base = g_pd_nvme_global->tx_sess_dgstq_base + g_sess_id * NVME_TX_SESS_DGSTQ_SIZE;

    data.base_addr = tx_sess_dgstq_base;
    data.log_num_entries = log2(NVME_TX_SESS_DGSTQ_DEPTH);

    data.session_id = g_sess_id;

    HAL_TRACE_DEBUG("Programming sessdgsttxcb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write sessdgsttxcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}


hal_ret_t
p4pd_add_or_del_nvme_sesscb (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t               ret;

    ret = p4pd_add_or_del_txsessprodcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program txsessprodcb for session-id: {}, ret: {:x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_sessxtstxcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program sessxtstxcb for session-id: {}, ret: {:x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_sessdgsttxcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program sessdgsttxcb for session-id: {}, ret: {:x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    return ret;
}

hal_ret_t
pd_nvme_sesscb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_nvme_sesscb_create_args_t *args = pd_func_args->pd_nvme_sesscb_create;
    pd_nvme_sesscb_s              *nvme_sesscb_pd;

    HAL_TRACE_DEBUG("Creating pd state for NVME Session CB.");

    // allocate PD nvme_sesscb state
    nvme_sesscb_pd = nvme_sesscb_pd_alloc_init();
    if (nvme_sesscb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    nvme_sesscb_pd->nvme_sesscb = args->nvme_sesscb;
    nvme_sesscb_pd->hw_id = get_nvme_tx_sessprodcb_addr(args->nvme_sesscb->g_sess_id);

    HAL_TRACE_DEBUG("Creating NVME Session CB at addr: 0x{:x} qid: {}",
            nvme_sesscb_pd->hw_id, nvme_sesscb_pd->nvme_sesscb->cb_id);

    // program nvme_sesscb
    ret = p4pd_add_or_del_nvme_sesscb(nvme_sesscb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Programming done");

    // add to db
    ret = add_nvme_sesscb_pd_to_db(nvme_sesscb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->nvme_sesscb->pd = nvme_sesscb_pd;

    return HAL_RET_OK;

cleanup:

    if (nvme_sesscb_pd) {
        nvme_sesscb_pd_free(nvme_sesscb_pd);
    }
    return ret;
}


} //namespace pd
} //namespace hal
