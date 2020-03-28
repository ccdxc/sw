//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nvme_global_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/sdk/platform/capri/capri_barco_rings.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/sdk/nvme/nvme_common.h"
#include "p4pd_nvme_api.h"

namespace hal {
namespace pd {

pd_nvme_global_t *g_pd_nvme_global = NULL;



static hal_ret_t
setup_resourcecb (uint64_t data_addr,
                  int max_cmd_context,
                  int tx_max_pdu_context,
                  int rx_max_pdu_context)
{
    //Setup resourcecb
    hal_ret_t ret = HAL_RET_OK;
    nvme_resourcecb_t data = { 0 };

    data.cmdid_ring_log_sz = log2(max_cmd_context);
    data.cmdid_ring_ci = 0;
    data.cmdid_ring_pi = max_cmd_context-1;
    data.cmdid_ring_proxy_pi = max_cmd_context-1;
    data.cmdid_ring_proxy_ci = 0;

    data.tx_pduid_ring_log_sz = log2(tx_max_pdu_context);
    data.tx_pduid_ring_ci = 0;
    data.tx_pduid_ring_pi = tx_max_pdu_context-1;
    data.tx_pduid_ring_proxy_pi = tx_max_pdu_context-1;
    data.tx_pduid_ring_proxy_ci = 0;

    data.rx_pduid_ring_log_sz = log2(rx_max_pdu_context);
    data.rx_pduid_ring_ci = 0;
    data.rx_pduid_ring_pi = rx_max_pdu_context-1;
    data.rx_pduid_ring_proxy_pi = rx_max_pdu_context-1;
    data.rx_pduid_ring_proxy_ci = 0;

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write to Resource CB for NVME Global Info");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

static hal_ret_t
setup_txhwxtscb (uint64_t data_addr)
{
    hal_ret_t ret = HAL_RET_OK;

    //Setup Tx HW/Barco XTS CB
    nvme_txhwxtscb_t data = { 0 };

    data.log_sz = log2(CAPRI_BARCO_XTS_RING_SLOTS);
    data.xts_ring_base_addr =
        asicpd_get_mem_addr(CAPRI_HBM_REG_BARCO_RING_XTS0);
    data.ci = 0;
    data.pi = 0;
    data.choke_counter = 0;

    memrev((uint8_t*)&data, sizeof(data));

    if(!p4plus_hbm_write(data_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write to Tx HW XTS CB for NVME Global Info");
        ret = HAL_RET_HW_FAIL;
    }

    return ret;
}

static hal_ret_t
setup_txhwdgstcb (uint64_t data_addr)
{
    hal_ret_t ret = HAL_RET_OK;

    //Setup Tx HW/Barco XTS CB
    nvme_txhwdgstcb_t data = { 0 };

    data.log_sz = log2(BARCO_CRYPTO_CP_RING_SIZE);
    data.dgst_ring_base_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_BARCO_RING_CP);
    data.ci = 0;
    data.pi = 0;
    data.choke_counter = 0;

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write to Tx HW DGST CB for NVME Global Info");
        ret = HAL_RET_HW_FAIL;
    }

    return ret;
}

static hal_ret_t
setup_rxhwxtscb (uint64_t data_addr)
{
    hal_ret_t ret = HAL_RET_OK;

    //Setup Rx HW/Barco XTS CB
    nvme_rxhwxtscb_t data = { 0 };

    data.log_sz = log2(CAPRI_BARCO_XTS_RING_SLOTS);
    data.xts_ring_base_addr =
        asicpd_get_mem_addr(CAPRI_HBM_REG_BARCO_RING_XTS1);
    data.ci = 0;
    data.pi = 0;
    data.choke_counter = 0;

    memrev((uint8_t*)&data, sizeof(data));

    if(!p4plus_hbm_write(data_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write to Rx HW XTS CB for NVME Global Info");
        ret = HAL_RET_HW_FAIL;
    }

    return ret;
}

static hal_ret_t
setup_rxhwdgstcb (uint64_t data_addr)
{
    hal_ret_t ret = HAL_RET_OK;

    //Setup Rx HW/Barco XTS CB
    nvme_rxhwdgstcb_t data = { 0 };

    data.log_sz = log2(BARCO_CRYPTO_DC_RING_SIZE);
    data.dgst_ring_base_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_BARCO_RING_DC);
    data.ci = 0;
    data.pi = 0;
    data.choke_counter = 0;

    memrev((uint8_t*)&data, sizeof(data));
    if(!p4plus_hbm_write(data_addr, (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to write to Rx HW DGST CB for NVME Global Info");
        ret = HAL_RET_HW_FAIL;
    }

    return ret;
}

static hal_ret_t
nvme_cmd_context_ring_entry_prepare (pd_nvme_global_t *nvme_global_pd,
                                     void *tmp_cmd_context_ring,
                                     uint16_t index,
                                     uint16_t cmd_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    nvme_cmd_context_ring_entry_t data = { 0 };
    nvme_cmd_context_ring_entry_t *ring
        = (nvme_cmd_context_ring_entry_t *)tmp_cmd_context_ring;

    SDK_ASSERT(index < nvme_global_pd->nvme_global->max_cmd_context);
    SDK_ASSERT(sizeof(nvme_cmd_context_ring_entry_t) == sizeof(uint16_t));

    data.cmdid = cmd_id;

    memrev((uint8_t*)&data, sizeof(data));

    memcpy(&ring[index], &data, sizeof(data));

    HAL_TRACE_DEBUG("Writing cmd_context_ring[{}] = {}, ret: {:#x}",
                    index, cmd_id, ret);
    return (ret);
}

static hal_ret_t
nvme_cmd_context_ring_hbm_write (uint64_t dst_addr,
                                 void *tmp_cmd_context_ring)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t data_size = nvme_hbm_resource_max(NVME_TYPE_CMD_CONTEXT_RING) * \
                         nvme_hbm_resource_size(NVME_TYPE_CMD_CONTEXT_RING);

    SDK_ASSERT(nvme_hbm_resource_size(NVME_TYPE_CMD_CONTEXT_RING) \
               == sizeof(nvme_cmd_context_ring_entry_t));

    if(!p4plus_hbm_write(dst_addr, (uint8_t *)tmp_cmd_context_ring, data_size,
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
       HAL_TRACE_ERR("Failed to write to HBM CMD Context Ring for NVME Global Info");
       ret = HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Writing cmd_context_ring to hbm at addr: {:#x} size: {}",
                    dst_addr, data_size);
    return ret;
}

static hal_ret_t
nvme_tx_pdu_context_ring_entry_prepare (pd_nvme_global_t *nvme_global_pd,
                                        void *tmp_tx_pdu_context_ring,
                                        uint16_t index,
                                        uint16_t pdu_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    nvme_tx_pdu_context_ring_entry_t data = { 0 };
    nvme_tx_pdu_context_ring_entry_t *ring
        = (nvme_tx_pdu_context_ring_entry_t *)tmp_tx_pdu_context_ring;

    SDK_ASSERT(index < nvme_global_pd->nvme_global->max_cmd_context);

    data.pduid = pdu_id;

    memrev((uint8_t*)&data, sizeof(data));

    memcpy(&ring[index], &data, sizeof(data));

    HAL_TRACE_DEBUG("Writing tx_pdu_context_ring[{}] = {}, ret: {:#x}",
                    index, pdu_id, ret);
    return (ret);
}

static hal_ret_t
nvme_tx_pdu_context_ring_hbm_write (uint64_t dst_addr,
                                    void *tmp_tx_pdu_context_ring)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t data_size = nvme_hbm_resource_total_mem(NVME_TYPE_TX_PDU_CONTEXT_RING);

    if(!p4plus_hbm_write(dst_addr, (uint8_t *)tmp_tx_pdu_context_ring, data_size,
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
       HAL_TRACE_ERR("Failed to write to HBM CMD Context Ring for NVME Global Info");
       ret = HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Writing tx_pdu_context_ring to hbm at addr: {:#x} size: {}",
                    dst_addr, data_size);
    return ret;
}

static hal_ret_t
nvme_rx_pdu_context_ring_entry_prepare (pd_nvme_global_t *nvme_global_pd,
                                        void *tmp_rx_pdu_context_ring,
                                        uint16_t index,
                                        uint16_t pdu_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    nvme_rx_pdu_context_ring_entry_t data = { 0 };
    nvme_rx_pdu_context_ring_entry_t *ring
        = (nvme_rx_pdu_context_ring_entry_t *)tmp_rx_pdu_context_ring;

    SDK_ASSERT(index < nvme_global_pd->nvme_global->max_cmd_context);

    data.pduid = pdu_id;

    memrev((uint8_t*)&data, sizeof(data));

    memcpy(&ring[index], &data, sizeof(data));

    HAL_TRACE_DEBUG("Writing rx_pdu_context_ring[{}] = {}, ret: {:#x}",
                    index, pdu_id, ret);
    return (ret);
}

static hal_ret_t
nvme_rx_pdu_context_ring_hbm_write (uint64_t dst_addr,
                                    void *tmp_rx_pdu_context_ring)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t data_size = nvme_hbm_resource_total_mem(NVME_TYPE_RX_PDU_CONTEXT_RING);

    if(!p4plus_hbm_write(dst_addr, (uint8_t *)tmp_rx_pdu_context_ring, data_size,
                P4PLUS_CACHE_INVALIDATE_BOTH)) {
       HAL_TRACE_ERR("Failed to write to HBM CMD Context Ring for NVME Global Info");
       ret = HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Writing rx_pdu_context_ring to hbm at addr: {:#x} size: {}",
                    dst_addr, data_size);
    return ret;
}


static hal_ret_t
create_nvme_global_state (pd_nvme_global_t *nvme_global_pd)
{
    hal_ret_t               ret = HAL_RET_OK;
    uint64_t                opaque_tag_addr;
    int32_t                 index = 0;


    int32_t max_lif, max_ns, max_sess;
    int32_t max_cmd_context, tx_max_pdu_context, rx_max_pdu_context;

    SDK_ASSERT(nvme_global_pd != NULL);
    SDK_ASSERT(nvme_global_pd->nvme_global != NULL);

    max_lif = nvme_global_pd->nvme_global->max_lif;
    max_ns = nvme_global_pd->nvme_global->max_ns;
    max_sess = nvme_global_pd->nvme_global->max_sess;
    max_cmd_context = nvme_global_pd->nvme_global->max_cmd_context;
    tx_max_pdu_context = nvme_global_pd->nvme_global->tx_max_pdu_context;
    rx_max_pdu_context = nvme_global_pd->nvme_global->rx_max_pdu_context;

    HAL_TRACE_DEBUG("max_lif: {} max_ns: {} max_sess: {} "
                    "max_cmd_context: {} tx_max_pdu_context: {} "
                    "rx_max_pdu_context: {}",
                    max_lif, max_ns, max_sess,
                    max_cmd_context,
                    tx_max_pdu_context,
                    rx_max_pdu_context);

    SDK_ASSERT(max_ns <= nvme_hbm_resource_max(NVME_TYPE_NSCB));
    SDK_ASSERT(max_sess <= nvme_hbm_resource_max(NVME_TYPE_TX_SESSPRODCB));
    SDK_ASSERT(max_cmd_context <= nvme_hbm_resource_max(NVME_TYPE_CMD_CONTEXT));
    SDK_ASSERT(tx_max_pdu_context <= nvme_hbm_resource_max(NVME_TYPE_TX_PDU_CONTEXT));
    SDK_ASSERT(rx_max_pdu_context <= nvme_hbm_resource_max(NVME_TYPE_RX_PDU_CONTEXT));
    SDK_ASSERT(nvme_hbm_offset(NVME_TYPE_MAX) <=
               (int) (asicpd_get_mem_size_kb(CAPRI_HBM_REG_NVME) * 1024));


    uint64_t nvme_hbm_start = asicpd_get_mem_addr(CAPRI_HBM_REG_NVME);
    //ns
    nvme_global_pd->nscb_base_addr = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_NSCB);

    //txsessprodcb
    nvme_global_pd->txsessprodcb_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_SESSPRODCB);

    //rxsessprodcb
    nvme_global_pd->rxsessprodcb_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_SESSPRODCB);

    //tx_sess_xtsq
    nvme_global_pd->tx_sess_xtsq_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_SESS_XTSQ);

    //tx_sess_dgstq
    nvme_global_pd->tx_sess_dgstq_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_SESS_DGSTQ);

    //rx_sess_xtsq
    nvme_global_pd->rx_sess_xtsq_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_SESS_XTSQ);

    //rx_sess_dgstq
    nvme_global_pd->rx_sess_dgstq_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_SESS_DGSTQ);

    //resourcecb
    nvme_global_pd->resourcecb_addr = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RESOURCECB);

    //tx_hwxtscb
    opaque_tag_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_OPAQUE_TAG) +
        get_opaque_tag_offset(BARCO_RING_XTS0);
    nvme_global_pd->tx_hwxtscb_addr = opaque_tag_addr;

    //rx_hwxtscb
    opaque_tag_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_OPAQUE_TAG) +
        get_opaque_tag_offset(BARCO_RING_XTS1);
    nvme_global_pd->rx_hwxtscb_addr = opaque_tag_addr;

    //tx_hwdgstcb
    opaque_tag_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_OPAQUE_TAG) +
        get_opaque_tag_offset(BARCO_RING_CP);
    nvme_global_pd->tx_hwdgstcb_addr = opaque_tag_addr;

    //rx_hwdgstcb
    opaque_tag_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_OPAQUE_TAG) +
        get_opaque_tag_offset(BARCO_RING_DC);
    nvme_global_pd->rx_hwdgstcb_addr = opaque_tag_addr;

    //cmd context page
    nvme_global_pd->cmd_context_page_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_CMD_CONTEXT);

    //cmd context ring
    nvme_global_pd->cmd_context_ring_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_CMD_CONTEXT_RING);

    //tx pdu context page
    nvme_global_pd->tx_pdu_context_page_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_PDU_CONTEXT);

    //tx pdu context ring
    nvme_global_pd->tx_pdu_context_ring_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_PDU_CONTEXT_RING);

    //rx pdu context page
    nvme_global_pd->rx_pdu_context_page_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_PDU_CONTEXT);

    //rx pdu context ring
    nvme_global_pd->rx_pdu_context_ring_base = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_PDU_CONTEXT_RING);

    //tx aol ring
    nvme_global_pd->tx_xts_aol_array_addr = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_XTS_AOL_ARRAY);

    //tx iv ring
    nvme_global_pd->tx_xts_iv_array_addr = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_TX_XTS_IV_ARRAY);

    //rx aol ring
    nvme_global_pd->rx_xts_aol_array_addr = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_XTS_AOL_ARRAY);

    //rx iv ring
    nvme_global_pd->rx_xts_iv_array_addr = nvme_hbm_start + nvme_hbm_offset(NVME_TYPE_RX_XTS_IV_ARRAY);

    HAL_TRACE_DEBUG("nscb_base_addr: {:#x} "
                    "txsessprodcb_base: {:#x}, "
                    "tx_sess_xtsq_base: {:#x}, "
                    "tx_sess_dgstq_base: {:#x}, "
                    "resourcecb_addr: {:#x}, "
                    "tx_hwxtscb_addr: {:#x}, "
                    "tx_hwdgstcb_addr: {:#x}, "
                    "rxsessprodcb_base: {:#x}, "
                    "rx_sess_xtsq_base: {:#x}, "
                    "rx_sess_dgstq_base: {:#x}, "
                    "rx_hwxtscb_addr: {:#x}, "
                    "rx_hwdgstcb_addr: {:#x}, "
                    "sess_bitmap_addr: {:#x}\n",
                    nvme_global_pd->nscb_base_addr,
                    nvme_global_pd->txsessprodcb_base,
                    nvme_global_pd->tx_sess_xtsq_base,
                    nvme_global_pd->tx_sess_dgstq_base,
                    nvme_global_pd->resourcecb_addr,
                    nvme_global_pd->tx_hwxtscb_addr,
                    nvme_global_pd->tx_hwdgstcb_addr,
                    nvme_global_pd->rxsessprodcb_base,
                    nvme_global_pd->rx_sess_xtsq_base,
                    nvme_global_pd->rx_sess_dgstq_base,
                    nvme_global_pd->rx_hwxtscb_addr,
                    nvme_global_pd->rx_hwdgstcb_addr,
                    nvme_global_pd->sess_bitmap_addr);


    HAL_TRACE_DEBUG("cmd_context_page_base: {:#x}, "
                    "cmd_context_ring_base: {:#x}, "
                    "tx_pdu_context_page_base: {:#x}, tx_pdu_context_ring_base: {:#x}, "
                    "rx_pdu_context_page_base: {:#x}, rx_pdu_context_ring_base: {:#x}, "
                    "tx_xts_aol_array_addr: {:#x}, tx_xts_iv_array_addr : {:#x}, "
                    "rx_xts_aol_array_addr: {:#x}, rx_xts_iv_array_addr : {:#x}",
                    nvme_global_pd->cmd_context_page_base,
                    nvme_global_pd->cmd_context_ring_base,
                    nvme_global_pd->tx_pdu_context_page_base,
                    nvme_global_pd->tx_pdu_context_ring_base,
                    nvme_global_pd->rx_pdu_context_page_base,
                    nvme_global_pd->rx_pdu_context_ring_base,
                    nvme_global_pd->tx_xts_aol_array_addr,
                    nvme_global_pd->tx_xts_iv_array_addr,
                    nvme_global_pd->rx_xts_aol_array_addr,
                    nvme_global_pd->rx_xts_iv_array_addr);

    int max = MAX3(nvme_hbm_resource_total_mem(NVME_TYPE_CMD_CONTEXT_RING),
                   nvme_hbm_resource_total_mem(NVME_TYPE_TX_PDU_CONTEXT_RING),
                   nvme_hbm_resource_total_mem(NVME_TYPE_RX_PDU_CONTEXT_RING));

    //Allocate temp memory to prepare the ring array
    //Assumption: all are of uint16_t
    SDK_ASSERT(nvme_hbm_resource_size(NVME_TYPE_CMD_CONTEXT_RING) == sizeof(uint16_t));
    SDK_ASSERT(nvme_hbm_resource_size(NVME_TYPE_TX_PDU_CONTEXT_RING) == sizeof(uint16_t));
    SDK_ASSERT(nvme_hbm_resource_size(NVME_TYPE_RX_PDU_CONTEXT_RING) == sizeof(uint16_t));

    void *temp = malloc(sizeof(uint16_t) * max);
    if (temp == NULL) {
        HAL_TRACE_ERR("Failed to allocate temp memory of size: {}", sizeof(uint16_t) * max);
        return (HAL_RET_OOM);
    }

    //Fill the ring with cmd context page addresses
    for (index = 0; index < max_cmd_context-1; index++) {
        nvme_cmd_context_ring_entry_prepare(nvme_global_pd, temp, index, index);
    }
    nvme_cmd_context_ring_hbm_write(nvme_global_pd->cmd_context_ring_base, temp);

    //Fill the ring with tx pdu context page addresses
    for (index = 0; index < tx_max_pdu_context-1; index++) {
        nvme_tx_pdu_context_ring_entry_prepare(nvme_global_pd, temp, index, index);
    }
    nvme_tx_pdu_context_ring_hbm_write(nvme_global_pd->tx_pdu_context_ring_base, temp);

    //Fill the ring with tx pdu context page addresses
    for (index = 0; index < rx_max_pdu_context-1; index++) {
        nvme_rx_pdu_context_ring_entry_prepare(nvme_global_pd, temp, index, index);
    }
    nvme_rx_pdu_context_ring_hbm_write(nvme_global_pd->rx_pdu_context_ring_base, temp);

    free(temp);
    temp = NULL;

    wring_hw_id_t base_addr;
    uint32_t num_entries;
    // Get TX_NMDPR_RING_BASE and Num Entries
    ret = wring_pd_get_base_addr(types::WRING_TYPE_NMDPR_BIG_TX,
                                 0,
                                 &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive NMDPR ring base");
        return HAL_RET_ERR;
    } else {
        nvme_global_pd->tx_nmdpr_ring_base = base_addr;
    }
    ret = wring_pd_get_num_entries(types::WRING_TYPE_NMDPR_BIG_TX,
                                   &num_entries);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive NMDPR ring base");
        return HAL_RET_ERR;
    } else {
        nvme_global_pd->tx_nmdpr_ring_size = num_entries;
    }
    HAL_TRACE_DEBUG("tx_nmdpr_ring_base: {:#x}, size: {}",
                    nvme_global_pd->tx_nmdpr_ring_base,
                    nvme_global_pd->tx_nmdpr_ring_size);

    // Get RX_NMDPR_RING_BASE and Num Entries
    ret = wring_pd_get_base_addr(types::WRING_TYPE_NMDPR_BIG_RX,
                                 0,
                                 &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to receive NMDPR ring base");
        return HAL_RET_ERR;
    } else {
        nvme_global_pd->rx_nmdpr_ring_base = base_addr;
    }
    ret = wring_pd_get_num_entries(types::WRING_TYPE_NMDPR_BIG_RX,
                                   &num_entries);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to receive NMDPR ring base", ret);
        return HAL_RET_ERR;
    } else {
        nvme_global_pd->rx_nmdpr_ring_size = num_entries;
    }
    HAL_TRACE_DEBUG("rx_nmdpr_ring_base: {:#x}, size: {}",
                    nvme_global_pd->rx_nmdpr_ring_base,
                    nvme_global_pd->rx_nmdpr_ring_size);



    ret = setup_resourcecb(nvme_global_pd->resourcecb_addr,
                           max_cmd_context, tx_max_pdu_context, rx_max_pdu_context);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to write to Resource CB for NVME Global Info",
                      ret);
        return ret;
    }

    ret = setup_txhwxtscb(nvme_global_pd->tx_hwxtscb_addr);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to write to Tx HW XTS CB for NVME Global Info",
                      ret);
        return ret;
    }

    ret = setup_rxhwxtscb(nvme_global_pd->rx_hwxtscb_addr);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to write to Rx HW XTS CB for NVME Global Info",
                      ret);
        return ret;
    }

    ret = setup_txhwdgstcb(nvme_global_pd->tx_hwdgstcb_addr);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to write to Tx HW DGST CB for NVME Global Info",
                      ret);
        return ret;
    }

    ret = setup_rxhwdgstcb(nvme_global_pd->rx_hwdgstcb_addr);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed(ret: {:#x}) to write to Rx HW DGST CB for NVME Global Info",
                      ret);
        return ret;
    }

    return ret;
}


static void
fill_response_params (NvmeEnableResponse *rsp)
{
    SDK_ASSERT(rsp != NULL);

    rsp->set_cmd_context_page_base(g_pd_nvme_global->cmd_context_page_base);
    rsp->set_cmd_context_ring_base(g_pd_nvme_global->cmd_context_ring_base);
    rsp->set_resourcecb_addr(g_pd_nvme_global->resourcecb_addr);
    rsp->set_tx_pdu_context_ring_base(g_pd_nvme_global->tx_pdu_context_ring_base);
    rsp->set_tx_pdu_context_page_base(g_pd_nvme_global->tx_pdu_context_page_base);
    rsp->set_rx_pdu_context_ring_base(g_pd_nvme_global->rx_pdu_context_ring_base);
    rsp->set_rx_pdu_context_page_base(g_pd_nvme_global->rx_pdu_context_page_base);
    rsp->set_tx_hwxtscb_addr(g_pd_nvme_global->tx_hwxtscb_addr);
    rsp->set_tx_hwdgstcb_addr(g_pd_nvme_global->tx_hwdgstcb_addr);
    rsp->set_rx_hwxtscb_addr(g_pd_nvme_global->rx_hwxtscb_addr);
    rsp->set_rx_hwdgstcb_addr(g_pd_nvme_global->rx_hwdgstcb_addr);
}


hal_ret_t
pd_nvme_global_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t                    ret = HAL_RET_OK;
    pd_nvme_global_create_args_t *args = pd_func_args->pd_nvme_global_create;
    pd_nvme_global_s             *nvme_global_pd;

    HAL_TRACE_DEBUG("Creating pd state for NVME Global.");

    // allocate PD nvme_global state
    nvme_global_pd = nvme_global_pd_alloc_init();
    if (nvme_global_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    nvme_global_pd->nvme_global = args->nvme_global;

    ret = create_nvme_global_state(nvme_global_pd);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Programming done");

    // add to db
    g_pd_nvme_global = nvme_global_pd;

    HAL_TRACE_DEBUG("DB add done");
    args->nvme_global->pd = nvme_global_pd;

    if (args->rsp != NULL) {
        fill_response_params(args->rsp);
    }

cleanup:
    return ret;
}


} //namespace pd
} //namespace hal

