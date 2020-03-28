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
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/hal.hpp"
#include "nvme_sesscb_pd.hpp"
#include "nvme_ns_pd.hpp"
#include "nvme_global_pd.hpp"
#include "nvme_pd.hpp"
#include "p4pd_nvme_api.h"
#include "nic/sdk/nvme/nvme_common.h"
#include "nic/hal/pd/iris/internal/tcpcb_pd.hpp"

namespace hal {
namespace pd {

extern pd_nvme_global_t *g_pd_nvme_global;

enum nvme_sess_qsubtype {
    NVME_SESS_XTS_TX = 0,
    NVME_SESS_DGST_TX = 1,
    NVME_SESS_XTS_RX = 2,
    NVME_SESS_DGST_RX = 3,
    NVME_SESS_RQ = 4,
    NVME_SESS_RF = 5
};
void *
nvme_sesscb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_nvme_sesscb_t *)entry)->hw_id);
}

uint32_t
nvme_sesscb_pd_hw_key_size ()
{
    return sizeof(nvme_sesscb_hw_id_t);
}

uint32_t nvme_sesscb_pd_sesq_prod_ci_offset_get()
{
    uint32_t offset;

    //assumption: tcp_q_ci is at offset 62 of the struct nvme_txsessprodcb_t
    //(i.e; last 2 bytes field of 64 bytes cb)

    offset = sizeof(nvme_txsessprodcb_t) - 2;

    return offset;
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
p4pd_add_or_del_txsessprodcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del, NvmeSessResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_txsessprodcb_t data = { 0 };
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
    if (rsp != NULL) {
        rsp->set_txsessprodcb_addr(data_addr);
    }

    // Get Tx Sess XTSQ base address
    tx_sess_xtsq_base = g_pd_nvme_global->tx_sess_xtsq_base + g_sess_id * NVME_TX_SESS_XTSQ_SIZE;
    if (rsp != NULL) {
        rsp->set_tx_xtsq_base(tx_sess_xtsq_base);
        rsp->set_tx_xtsq_num_entries(NVME_TX_SESS_XTSQ_DEPTH);
    }

    data.xts_q_base_addr = tx_sess_xtsq_base;
    data.xts_q_pi = 0;
    data.xts_q_ci = 0;
    data.log_num_xts_q_entries = log2(NVME_TX_SESS_XTSQ_DEPTH);
    data.xts_qid = lif_sess_id;
    SDK_ASSERT((NVME_TX_SESS_XTSQ_DEPTH & (NVME_TX_SESS_XTSQ_DEPTH - 1)) == 0);

    // Get Tx Sess DGSTQ base address
    tx_sess_dgstq_base = g_pd_nvme_global->tx_sess_dgstq_base + g_sess_id * NVME_TX_SESS_DGSTQ_SIZE;
    if (rsp != NULL) {
        rsp->set_tx_dgstq_base(tx_sess_dgstq_base);
        rsp->set_tx_dgstq_num_entries(NVME_TX_SESS_DGSTQ_DEPTH);
    }

    data.dgst_q_base_addr = tx_sess_dgstq_base;
    data.dgst_q_pi = 0;
    data.dgst_q_ci = 0;
    data.log_num_dgst_q_entries = log2(NVME_TX_SESS_DGSTQ_DEPTH);
    data.dgst_qid = lif_sess_id;

    HAL_TRACE_DEBUG("lif: {} g_sess_id: {} lif_sess_id: {} ns_sess_id: {}",
                    nvme_sesscb_pd->nvme_sesscb->lif,
                    g_sess_id, lif_sess_id, nvme_sesscb_pd->nvme_sesscb->ns_sess_id);

    HAL_TRACE_DEBUG("xts_qid: {} xts_q_base_addr: {:#x} dgst_qid: {} dgst_q_base_addr: {:#x}",
                    data.xts_qid, data.xts_q_base_addr, data.dgst_qid, data.dgst_q_base_addr);

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

    if (rsp != NULL) {
        rsp->set_tx_sesq_base(sesq_base);
        rsp->set_tx_sesq_num_entries(sesq_size);
    }

    data.tcp_q_base_addr = sesq_base;
    data.tcp_q_pi = 0;
    data.tcp_q_ci = 0;
    data.log_num_tcp_q_entries = log2(sesq_size);
    SDK_ASSERT((NVME_TX_SESS_DGSTQ_DEPTH & (NVME_TX_SESS_DGSTQ_DEPTH - 1)) == 0);

    data.tcpcb_sesq_db_addr = tcpcb_sesq_db_addr(sesq_qid);
    data.tcpcb_sesq_db_data = tcpcb_sesq_db_data(sesq_qid);
    
    HAL_TRACE_DEBUG("Programming txsessprodcb with tcpcb sesq db_addr: {:#x} data: {:#x}",
                    data.tcpcb_sesq_db_addr, data.tcpcb_sesq_db_data);
    
    HAL_TRACE_DEBUG("Programming txsessprodcb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write txsessprodcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    ret = p4pd_update_sesq_ci_addr(sesq_qid, 
                                   data_addr + nvme_sesscb_pd_sesq_prod_ci_offset_get());
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write TCP CB with sesq_ci_addr");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_sessxtstxcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_sessxtstxcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id;
    uint64_t tx_sess_xtsq_base;
    uint64_t offset;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_SESS,
                                                   (lif_sess_id+(NVME_SESS_XTS_TX*HAL_MAX_NVME_SESSIONS)));
    SDK_ASSERT(data_addr != 0);

    //total and host are reversed in auto-generated structs
    data.host = MAX_SESSXTSTX_RINGS;
    data.total = MAX_SESSXTSTX_HOST_RINGS;

    get_program_offset((char *)"txdma_stage0.bin",
                       (char *)"nvme_tx_sessxts_stage0",
                       &offset);
    data.pc = offset; //pc

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
    nvme_sessdgsttxcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id;
    uint64_t tx_sess_dgstq_base;
    uint64_t offset;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_SESS,
                                                   (lif_sess_id+(NVME_SESS_DGST_TX*HAL_MAX_NVME_SESSIONS)));
    SDK_ASSERT(data_addr != 0);

    //total and host are reversed in auto-generated structs
    data.host = MAX_SESSDGSTTX_RINGS;
    data.total = MAX_SESSDGSTTX_HOST_RINGS;

    get_program_offset((char *)"txdma_stage0.bin",
                       (char *)"nvme_tx_sessdgst_stage0",
                       &offset);
    data.pc = offset; //pc

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
p4pd_add_or_del_rxsessprodcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del, NvmeSessResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_rxsessprodcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t g_sess_id, lif_sess_id;
    uint64_t rx_sess_xtsq_base;
    uint64_t rx_sess_dgstq_base;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = g_pd_nvme_global->rxsessprodcb_base + g_sess_id * sizeof(nvme_rxsessprodcb_t);
    if (rsp != NULL) {
        rsp->set_rxsessprodcb_addr(data_addr);
    }

    // Get Rx Sess XTSQ base address
    rx_sess_xtsq_base = g_pd_nvme_global->rx_sess_xtsq_base + g_sess_id * NVME_RX_SESS_XTSQ_SIZE;
    if (rsp != NULL) {
        rsp->set_rx_xtsq_base(rx_sess_xtsq_base);
        rsp->set_rx_xtsq_num_entries(NVME_RX_SESS_XTSQ_DEPTH);
    }

    data.xts_q_base_addr = rx_sess_xtsq_base;
    data.xts_q_pi = 0;
    data.xts_q_ci = 0;
    data.log_num_xts_q_entries = log2(NVME_RX_SESS_XTSQ_DEPTH);
    data.xts_qid = lif_sess_id;
    SDK_ASSERT((NVME_RX_SESS_XTSQ_DEPTH & (NVME_RX_SESS_XTSQ_DEPTH - 1)) == 0);

    // Get Rx Sess DGSTQ base address
    rx_sess_dgstq_base = g_pd_nvme_global->rx_sess_dgstq_base + g_sess_id * NVME_RX_SESS_DGSTQ_SIZE;
    if (rsp != NULL) {
        rsp->set_rx_dgstq_base(rx_sess_dgstq_base);
        rsp->set_rx_dgstq_num_entries(NVME_RX_SESS_DGSTQ_DEPTH);
    }

    data.dgst_q_base_addr = rx_sess_dgstq_base;
    data.dgst_q_pi = 0;
    data.dgst_q_ci = 0;
    data.log_num_dgst_q_entries = log2(NVME_RX_SESS_DGSTQ_DEPTH);
    data.dgst_qid = lif_sess_id;

    HAL_TRACE_DEBUG("lif: {} g_sess_id: {} lif_sess_id: {} ns_sess_id: {}",
                    nvme_sesscb_pd->nvme_sesscb->lif,
                    g_sess_id, lif_sess_id, nvme_sesscb_pd->nvme_sesscb->ns_sess_id);

    HAL_TRACE_DEBUG("xts_qid: {} xts_q_base_addr: {:#x} dgst_qid: {} dgst_q_base_addr: {:#x}",
                    data.xts_qid, data.xts_q_base_addr, data.dgst_qid, data.dgst_q_base_addr);

    HAL_TRACE_DEBUG("Programming rxsessprodcb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write rxsessprodcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_rqcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del, NvmeSessResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_rqcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id, serq_qid;
    uint64_t offset;
    
    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;
    serq_qid = nvme_sesscb_pd->nvme_sesscb->serq_qid;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_SESS,
                                                   (lif_sess_id+(NVME_SESS_RQ*HAL_MAX_NVME_SESSIONS)));
    SDK_ASSERT(data_addr != 0);

    HAL_TRACE_DEBUG("lif: {} g_sess_id: {} lif_sess_id: {} ns_sess_id: {}",
                    nvme_sesscb_pd->nvme_sesscb->lif,
                    g_sess_id, lif_sess_id, nvme_sesscb_pd->nvme_sesscb->ns_sess_id);

    //total and host are reversed in auto-generated structs
    data.host = 10;
    data.total = 10;

    get_program_offset((char *)"txdma_stage0.bin",
                       (char *)"nvme_rq_stage0",
                       &offset);
    data.pc = offset; //pc

    // Get SERQ address of the TCP flow
    wring_hw_id_t  serq_base;
    ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                 serq_qid,
                                 &serq_base);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive serq base for tcp qid: {}",
                      serq_qid);
    }

    uint32_t serq_size;
    ret = wring_pd_get_num_entries(types::WRING_TYPE_SERQ,
                                   &serq_size);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive serq num_entries for tcp qid: {}",
                      serq_qid);
    }

    HAL_TRACE_DEBUG("Serq id: {:#x} Serq base: {:#x}, size: {}",
                    serq_qid, serq_base, serq_size);

    if (rsp != NULL) {
        rsp->set_rx_serq_base(serq_base);
        rsp->set_rx_serq_num_entries(serq_size);
    }

    data.base_addr = serq_base;
    data.log_num_entries = log2(serq_size);
    data.tcp_serq_ci_addr = tcpcb_pd_serq_prod_ci_addr_get(serq_qid);

    HAL_TRACE_DEBUG("Programming rqcb with tcpcb serq base_addr: {:#x} log_num_entries: {:#x} \
                    tcp_serq_ci_addr: {:#x}",
                    data.base_addr, data.log_num_entries, data.tcp_serq_ci_addr);

    //data.tcpcb_serq_db_addr = tcpcb_serq_db_addr(serq_qid);
    //data.tcpcb_serq_db_data = tcpcb_serq_db_data(serq_qid);
    
    //HAL_TRACE_DEBUG("Programming rqcb with tcpcb serq db_addr: {:#x} data: {:#x}",
    //                data.tcpcb_serq_db_addr, data.tcpcb_serq_db_data);
    

    HAL_TRACE_DEBUG("Programming rqcb at addr: {:#x}", 
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write rqcb for NVME Session CB");
        return (HAL_RET_HW_FAIL);
    }

    HAL_TRACE_DEBUG("Programming tcpcb consumer lif, qtype, qid: {:#x}, "
                    "{:#x}, {:#x}", lif, NVME_QTYPE_SESS,
                    (lif_sess_id+(NVME_SESS_RQ*HAL_MAX_NVME_SESSIONS)));

    ret = tcpcb_pd_serq_lif_qtype_qstate_ring_set(serq_qid, lif,
                           NVME_QTYPE_SESS,
                           lif_sess_id+(NVME_SESS_RQ*HAL_MAX_NVME_SESSIONS),
                           0 /* ring_id */);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write TCP CB with sesq_ci_addr");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_sessxtsrxcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_sessxtsrxcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id;
    uint64_t rx_sess_xtsq_base;
    uint64_t offset;

    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_SESS,
                                                   (lif_sess_id+(NVME_SESS_XTS_RX*HAL_MAX_NVME_SESSIONS)));
    SDK_ASSERT(data_addr != 0);

    //total and host are reversed in auto-generated structs
    data.host = MAX_SESSXTSRX_RINGS;
    data.total = MAX_SESSXTSRX_HOST_RINGS;

    get_program_offset((char *)"rxdma_stage0.bin",
                       (char *)"nvme_rx_sessxts_stage0",
                       &offset);
    data.pc = offset; //pc

    HAL_TRACE_DEBUG("sess_num: {}, rx sessxts pc: {:#x}", g_sess_id, offset);

    rx_sess_xtsq_base = g_pd_nvme_global->rx_sess_xtsq_base + g_sess_id * NVME_RX_SESS_XTSQ_SIZE;

    data.base_addr = rx_sess_xtsq_base;
    data.log_num_entries = log2(NVME_RX_SESS_XTSQ_DEPTH);

    data.session_id = g_sess_id;

    HAL_TRACE_DEBUG("Programming sessxtsrxcb at addr: {:#x}",
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write sessxtsrxcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_sessdgstrxcb_entry (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del)
{
    hal_ret_t ret = HAL_RET_OK;
    nvme_sessdgstrxcb_t data = { 0 };
    uint64_t data_addr = 0;
    uint32_t lif, g_sess_id, lif_sess_id;
    uint64_t rx_sess_dgstq_base;
    uint64_t offset;

    SDK_ASSERT(nvme_sesscb_pd != NULL);
    SDK_ASSERT(nvme_sesscb_pd->nvme_sesscb != NULL);

    lif = nvme_sesscb_pd->nvme_sesscb->lif;
    g_sess_id = nvme_sesscb_pd->nvme_sesscb->g_sess_id;
    lif_sess_id = nvme_sesscb_pd->nvme_sesscb->lif_sess_id;

    data_addr = lif_manager()->get_lif_qstate_addr(lif, NVME_QTYPE_SESS,
                                                   (lif_sess_id+(NVME_SESS_DGST_RX*HAL_MAX_NVME_SESSIONS)));
    SDK_ASSERT(data_addr != 0);

    //total and host are reversed in auto-generated structs
    data.host = MAX_SESSDGSTRX_RINGS;
    data.total = MAX_SESSDGSTRX_HOST_RINGS;

    get_program_offset((char *)"rxdma_stage0.bin",
                       (char *)"nvme_rx_sessdgst_stage0",
                       &offset);
    data.pc = offset; //pc

    HAL_TRACE_DEBUG("sess_num: {}, rx sessdgst pc: {:#x}", g_sess_id, offset);

    rx_sess_dgstq_base = g_pd_nvme_global->rx_sess_dgstq_base + g_sess_id * NVME_RX_SESS_DGSTQ_SIZE;

    data.base_addr = rx_sess_dgstq_base;
    data.log_num_entries = log2(NVME_RX_SESS_DGSTQ_DEPTH);

    data.session_id = g_sess_id;

    HAL_TRACE_DEBUG("Programming sessdgstrxcb at addr: {:#x}",
                    data_addr);

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write sessdgstrxcb for NVME Session CB");
        ret = HAL_RET_HW_FAIL;
    }

    return (ret);
}

hal_ret_t
p4pd_add_or_del_nvme_sesscb (pd_nvme_sesscb_t *nvme_sesscb_pd, bool del, NvmeSessResponse *rsp)
{
    hal_ret_t               ret;

    ret = p4pd_add_or_del_txsessprodcb_entry(nvme_sesscb_pd, del, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program txsessprodcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_sessxtstxcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program sessxtstxcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_sessdgsttxcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program sessdgsttxcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_rxsessprodcb_entry(nvme_sesscb_pd, del, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program rxsessprodcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_rqcb_entry(nvme_sesscb_pd, del, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program rqcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_sessxtsrxcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program sessxtsrxcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = p4pd_add_or_del_sessdgstrxcb_entry(nvme_sesscb_pd, del);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program sessdgstrxcb for session-id: {}, ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id, ret);
        return ret;
    }

    ret = nvme_ns_update_session_id(nvme_sesscb_pd->nvme_sesscb->g_nsid,
                                    nvme_sesscb_pd->nvme_sesscb->ns_sess_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update nscb bitmap for session-id: {} g_nsid: {} ret: {:#x}",
                      nvme_sesscb_pd->nvme_sesscb->g_sess_id,
                      nvme_sesscb_pd->nvme_sesscb->ns_sess_id, ret);
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
    nvme_sesscb_pd->hw_id = args->nvme_sesscb->g_sess_id;

    HAL_TRACE_DEBUG("Creating NVME Session CB, PD Handle: {}, PI Handle: {}",
            nvme_sesscb_pd->hw_id, nvme_sesscb_pd->nvme_sesscb->cb_id);

    // program nvme_sesscb
    ret = p4pd_add_or_del_nvme_sesscb(nvme_sesscb_pd, false, args->rsp);
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

    if (args->rsp != NULL) {
        args->rsp->set_sess_id(args->nvme_sesscb->lif_sess_id);
    }
    return HAL_RET_OK;

cleanup:

    if (nvme_sesscb_pd) {
        nvme_sesscb_pd_free(nvme_sesscb_pd);
    }
    return ret;
}


} //namespace pd
} //namespace hal
