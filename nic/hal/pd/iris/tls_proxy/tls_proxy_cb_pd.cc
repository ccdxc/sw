#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/tls_proxy_cb_pd.hpp"
#include "nic/hal/pd/iris/internal/tcpcb_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/tls_txdma_pre_crypto_enc/include/tls_txdma_pre_crypto_enc_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/include/app_redir_shared.h"

namespace hal {
namespace pd {

void *
tls_proxy_cb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_tls_proxy_cb_t *)entry)->hw_id);
}

uint32_t
tls_proxy_cb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(tls_proxy_cb_hw_id_t)) % ht_size;
}

bool
tls_proxy_cb_pd_compare_hw_key_func (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tls_proxy_cb_hw_id_t *)key1 == *(tls_proxy_cb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t
p4pd_get_tls_tx_s0_t0_read_tls_stg0_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    tx_table_s0_t0_d        data = {0};

    // hardware index for this entry
    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: s0_t0_read_tls_stg0 entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tls_proxy_cb_pd->tls_proxy_cb->sesq_base = data.u.read_tls_stg0_d.sesq_base;
    tls_proxy_cb_pd->tls_proxy_cb->serq_base = data.u.read_tls_stg0_d.serq_base;
    tls_proxy_cb_pd->tls_proxy_cb->serq_pi = data.u.read_tls_stg0_d.pi_0;
    tls_proxy_cb_pd->tls_proxy_cb->serq_ci = data.u.read_tls_stg0_d.ci_0;
    tls_proxy_cb_pd->tls_proxy_cb->bsq_pi = data.u.read_tls_stg0_d.pi_1;
    tls_proxy_cb_pd->tls_proxy_cb->bsq_ci = data.u.read_tls_stg0_d.ci_1;
    tls_proxy_cb_pd->tls_proxy_cb->debug_dol = ntohl(data.u.read_tls_stg0_d.debug_dol);
    tls_proxy_cb_pd->tls_proxy_cb->command = data.u.read_tls_stg0_d.barco_command;
    tls_proxy_cb_pd->tls_proxy_cb->is_decrypt_flow = data.u.read_tls_stg0_d.dec_flow;
    tls_proxy_cb_pd->tls_proxy_cb->l7_proxy_type = types::AppRedirType(data.u.read_tls_stg0_d.l7_proxy_type);

    HAL_TRACE_DEBUG("Received sesq_base: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->sesq_base);
    HAL_TRACE_DEBUG("Received serq_base: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->serq_base);
    HAL_TRACE_DEBUG("Received serq_pi: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->serq_pi);
    HAL_TRACE_DEBUG("Received serq_ci: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->serq_ci);
    HAL_TRACE_DEBUG("Received bsq_pi: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->bsq_pi);
    HAL_TRACE_DEBUG("Received bsq_ci: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->bsq_ci);
    HAL_TRACE_DEBUG("Received debug_dol: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->debug_dol);
    HAL_TRACE_DEBUG("Received command: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->command);
    HAL_TRACE_DEBUG("Received dec_flow : 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->is_decrypt_flow);
    HAL_TRACE_DEBUG("Received L7 proxy type: {}", tls_proxy_cb_pd->tls_proxy_cb->l7_proxy_type);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tls_tx_s1_t0_read_tls_stg1_7_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    tx_table_s1_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S1_T0_READ_TLS_ST1_7);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s1_t0_read_tls_stg1_7 entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tls_proxy_cb_pd->tls_proxy_cb->other_fid = ntohs(data.u.read_tls_stg1_7_d.other_fid);
    HAL_TRACE_DEBUG("Received other fid: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->other_fid);

    tls_proxy_cb_pd->tls_proxy_cb->crypto_hmac_key_idx = data.u.read_tls_stg1_7_d.barco_hmac_key_desc_index;
    HAL_TRACE_DEBUG("Received HMAC key index: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->crypto_hmac_key_idx);
    return ret;
}

hal_ret_t
p4pd_get_tls_tx_s6_t0_pre_crypto_stats_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    tx_table_s6_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS_U16);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s6_t0_pre_crypto_stats_entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tls_proxy_cb_pd->tls_proxy_cb->tnmdpr_alloc = ntohs(data.u.tls_pre_crypto_stats6_d.tnmdpr_alloc);
    tls_proxy_cb_pd->tls_proxy_cb->enc_requests = ntohs(data.u.tls_pre_crypto_stats6_d.enc_requests);
    tls_proxy_cb_pd->tls_proxy_cb->dec_requests = ntohs(data.u.tls_pre_crypto_stats6_d.dec_requests);
    tls_proxy_cb_pd->tls_proxy_cb->mac_requests = ntohs(data.u.tls_pre_crypto_stats6_d.mac_requests);
    tls_proxy_cb_pd->tls_proxy_cb->pre_debug_stage0_7_thread =
      (ntohs(data.u.tls_pre_crypto_stats6_d.debug_stage4_7_thread) << 16) |
      ntohs(data.u.tls_pre_crypto_stats6_d.debug_stage0_3_thread);
    HAL_TRACE_DEBUG("hwid : {:#x}", hwid);
    HAL_TRACE_DEBUG("Received tnmdpr alloc: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->tnmdpr_alloc);
    HAL_TRACE_DEBUG("Received enc requests: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->enc_requests);
    HAL_TRACE_DEBUG("Received dec requests: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->dec_requests);
    HAL_TRACE_DEBUG("Received mac requests: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->mac_requests);
    HAL_TRACE_DEBUG("Received pre debug stage0_7 thread: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->pre_debug_stage0_7_thread);
    HAL_TRACE_DEBUG("Received l7_proxy_type: {}", tls_proxy_cb_pd->tls_proxy_cb->l7_proxy_type);
    return ret;
}

hal_ret_t
p4pd_get_tls_tx_config_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    tx_table_s2_t0_d                data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_CONFIG);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: config entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }

    tls_proxy_cb_pd->tls_proxy_cb->crypto_key_idx = data.u.tls_rx_serq_d.barco_key_desc_index;
    tls_proxy_cb_pd->tls_proxy_cb->explicit_iv = data.u.tls_rx_serq_d.sequence_no;
    tls_proxy_cb_pd->tls_proxy_cb->salt = ntohl(data.u.tls_rx_serq_d.salt);
    tls_proxy_cb_pd->tls_proxy_cb->cpu_id = data.u.tls_rx_serq_d.cpu_id;


    HAL_TRACE_DEBUG("Received crypto_key_idx: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->crypto_key_idx);
    HAL_TRACE_DEBUG("Received explicit_iv: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->explicit_iv);
    HAL_TRACE_DEBUG("Received salt: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->salt);
    HAL_TRACE_DEBUG("Received cpu_id: {}", tls_proxy_cb_pd->tls_proxy_cb->cpu_id);

    return ret;
}

hal_ret_t
p4pd_get_tls_proxy_cb_txdma_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_tls_tx_s0_t0_read_tls_stg0_entry(tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_s1_t0_read_tls_stg1_7_entry(tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_config_entry(tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_s6_t0_pre_crypto_stats_entry(tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_s5_t0_post_crypto_stats_entry(tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

cleanup:
    return ret;
}

hal_ret_t
p4pd_get_tls_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "tls_tx_stage0";

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

hal_ret_t
p4pd_add_or_del_tls_tx_s0_t0_read_tls_stg0_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd, bool del)
{
    tx_table_s0_t0_d        data = {0};
    hal_ret_t               ret = HAL_RET_OK;
    wring_hw_id_t           q_base;

    // hardware index for this entry
    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0);

    if(!del) {
        HAL_TRACE_DEBUG("TLS TXDMA Stage0 Received debug_dol 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->debug_dol);

        uint64_t pc_offset;
        // get pc address
        if(p4pd_get_tls_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        HAL_TRACE_DEBUG("TLS TXDMA Stage0 Received pc address {:#x}", pc_offset);

        data.action_id = pc_offset;
        data.u.read_tls_stg0_d.total = 3;

        // Get Serq address
        wring_hw_id_t  serq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                     tls_proxy_cb_pd->tls_proxy_cb->cb_id,
                                     &serq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive serq base for tls_proxy_cbcb: {}",
                        tls_proxy_cb_pd->tls_proxy_cb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Serq id: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->cb_id);
            HAL_TRACE_DEBUG("Serq base: 0x{:x}", serq_base);
            data.u.read_tls_stg0_d.serq_base = htonl(serq_base);
        }
        // Get Sesq address
        wring_hw_id_t  sesq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                     ((tls_proxy_cb_pd->tls_proxy_cb->other_fid == 0xFFFF) ?
                                      tls_proxy_cb_pd->tls_proxy_cb->cb_id : tls_proxy_cb_pd->tls_proxy_cb->other_fid),
                                     &sesq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive sesq base for tls_proxy_cbcb: {}",
                        tls_proxy_cb_pd->tls_proxy_cb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Sesq id: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->other_fid);
            HAL_TRACE_DEBUG("Sesq base: 0x{:x}", sesq_base);
            data.u.read_tls_stg0_d.sesq_base = htonl(sesq_base);
        }
        data.u.read_tls_stg0_d.debug_dol = htonl(tls_proxy_cb_pd->tls_proxy_cb->debug_dol);
        HAL_TRACE_DEBUG("debug_dol = 0x{:x}", data.u.read_tls_stg0_d.debug_dol);

        data.u.read_tls_stg0_d.barco_command = tls_proxy_cb_pd->tls_proxy_cb->command;
        HAL_TRACE_DEBUG("Barco Command = 0x{:x}", data.u.read_tls_stg0_d.barco_command);

        data.u.read_tls_stg0_d.pi_0 = tls_proxy_cb_pd->tls_proxy_cb->serq_pi;
        HAL_TRACE_DEBUG("SERQ PI = 0x{:x}", data.u.read_tls_stg0_d.pi_0);

        data.u.read_tls_stg0_d.ci_0 = tls_proxy_cb_pd->tls_proxy_cb->serq_ci;
        HAL_TRACE_DEBUG("SERQ CI = 0x{:x}", data.u.read_tls_stg0_d.ci_0);

        HAL_TRACE_DEBUG("RAW[0x{:x}]: is_decrypt_flow: 0x{:x}", tls_proxy_cb_pd->tls_proxy_cb->cb_id, tls_proxy_cb_pd->tls_proxy_cb->is_decrypt_flow);
        if (tls_proxy_cb_pd->tls_proxy_cb->is_decrypt_flow)
            data.u.read_tls_stg0_d.dec_flow = 1;
        HAL_TRACE_DEBUG("Is Decrypt Flow = {}", data.u.read_tls_stg0_d.dec_flow ? "TRUE" : "FALSE");

        data.u.read_tls_stg0_d.l7_proxy_type = tls_proxy_cb_pd->tls_proxy_cb->l7_proxy_type;
        HAL_TRACE_DEBUG("l7_proxy_type = {}", data.u.read_tls_stg0_d.l7_proxy_type);

        /* record ring */
        ret = wring_pd_get_base_addr(types::WRING_TYPE_BSQ,
                                     tls_proxy_cb_pd->tls_proxy_cb->cb_id, &q_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to retrieve record queue base base for proxyrcb_id: {}",
                    tls_proxy_cb_pd->tls_proxy_cb->cb_id);
        } else {
            HAL_TRACE_DEBUG("recqbase id: {:#x}, base: {:#x}", tls_proxy_cb_pd->tls_proxy_cb->cb_id, q_base);
            data.u.read_tls_stg0_d.recq_base = (uint32_t) q_base;
            data.u.read_tls_stg0_d.recq_pi = 0;
            data.u.read_tls_stg0_d.recq_ci = 0;
        }

        data.u.read_tls_stg0_d.serq_prod_ci_addr =
            htonl(tcpcb_pd_serq_prod_ci_addr_get(tls_proxy_cb_pd->tls_proxy_cb->cb_id));
        HAL_TRACE_DEBUG("qid {:#x} serq_prod_ci_addr {:#x}",
                tls_proxy_cb_pd->tls_proxy_cb->cb_id,
                ntohl(data.u.read_tls_stg0_d.serq_prod_ci_addr));
    }

    HAL_TRACE_DEBUG("TLSCB: Programming at hw-id: 0x{:x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: s0_t0_read_tls_stg0 entry for TLS CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t
p4pd_add_or_del_tls_tx_s1_t0_read_tls_stg1_7_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd, bool del)
{
    tx_table_s1_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S1_T0_READ_TLS_ST1_7);

    if(!del) {
        data.u.read_tls_stg1_7_d.other_fid = htons(tls_proxy_cb_pd->tls_proxy_cb->other_fid);
        HAL_TRACE_DEBUG("other fid = 0x{:x}", data.u.read_tls_stg1_7_d.other_fid);

        // Get L7Q address
        wring_hw_id_t  q_base;
        uint32_t proxyrcb_id = tls_proxy_cb_pd->tls_proxy_cb->cb_id;

        ret = wring_pd_get_base_addr(types::WRING_TYPE_APP_REDIR_PROXYR,
                                     proxyrcb_id, &q_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive l7q base for proxyrcb_id: {}",
                    proxyrcb_id);
        } else {
            HAL_TRACE_DEBUG("l7q id: {:#x}, base: {:#x}", proxyrcb_id, q_base);
            data.u.read_tls_stg1_7_d.l7q_base = q_base;
        }

        data.u.read_tls_stg1_7_d.barco_hmac_key_desc_index = tls_proxy_cb_pd->tls_proxy_cb->crypto_hmac_key_idx;
        HAL_TRACE_DEBUG("Barco HMAC Key Desc Index = 0x{:x}", data.u.read_tls_stg1_7_d.barco_hmac_key_desc_index);
    }
    HAL_TRACE_DEBUG("TLSCB: Programming at hw-id: 0x{:x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: s3_t0_read_tls_stg1_7 entry for TLS CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_tls_tx_config_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd, bool del)
{
    tx_table_s2_t0_d                data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    tls_proxy_cb_hw_id_t hwid = tls_proxy_cb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_CONFIG);

    if (!del) {
        data.u.tls_rx_serq_d.barco_key_desc_index = tls_proxy_cb_pd->tls_proxy_cb->crypto_key_idx;
        HAL_TRACE_DEBUG("Barco Key Desc Index = 0x{:x}", data.u.tls_rx_serq_d.barco_key_desc_index);

        data.u.tls_rx_serq_d.sequence_no = (tls_proxy_cb_pd->tls_proxy_cb->explicit_iv);
        HAL_TRACE_DEBUG("Sequence Number = 0x{:x}", data.u.tls_rx_serq_d.sequence_no);

        data.u.tls_rx_serq_d.salt = tls_proxy_cb_pd->tls_proxy_cb->salt;
        HAL_TRACE_DEBUG("Salt = 0x{:x}", data.u.tls_rx_serq_d.salt);

        data.u.tls_rx_serq_d.cpu_id = tls_proxy_cb_pd->tls_proxy_cb->cpu_id;
        HAL_TRACE_DEBUG("CPU-id = {}", data.u.tls_rx_serq_d.cpu_id);

        data.u.tls_rx_serq_d.sw_bsq_pi = 0;
    }

    HAL_TRACE_DEBUG("TLSCB Config: Programming at hw-id: 0x{:x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: config entry for TLS CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;

}


hal_ret_t
p4pd_add_or_del_tls_proxy_cb_txdma_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tls_tx_s0_t0_read_tls_stg0_entry(tls_proxy_cb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tls_tx_s1_t0_read_tls_stg1_7_entry(tls_proxy_cb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tls_tx_config_entry(tls_proxy_cb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    return HAL_RET_OK;

cleanup:

    /* TODO: Cleanup */
    return ret;
}

/**************************/

tls_proxy_cb_hw_id_t
pd_tls_proxy_cb_get_base_hw_index(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    SDK_ASSERT(NULL != tls_proxy_cb_pd);
    SDK_ASSERT(NULL != tls_proxy_cb_pd->tls_proxy_cb);

    // Get the base address of TLS CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_TLS_PROXY, 0,
            tls_proxy_cb_pd->tls_proxy_cb->cb_id);
    HAL_TRACE_DEBUG("received offset 0x{:x}", offset);
    return offset;
}

hal_ret_t
p4pd_add_or_del_tls_proxy_cb_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd, bool del)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tls_proxy_cb_txdma_entry(tls_proxy_cb_pd, del);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add/delete TLS TxDMA Entry {}", ret);
    }

    return ret;
}

static
hal_ret_t
p4pd_get_tls_proxy_cb_entry(pd_tls_proxy_cb_t* tls_proxy_cb_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_tls_proxy_cb_txdma_entry(tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for tls_proxy_cb");
    }

    return ret;
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
pd_tls_proxy_cb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tls_proxy_cb_create_args_t *args = pd_func_args->pd_tls_proxy_cb_create;
    pd_tls_proxy_cb_s              *tls_proxy_cb_pd;

    HAL_TRACE_DEBUG("Creating pd state for TLS CB.");

    // allocate PD tls_proxy_cb state
    tls_proxy_cb_pd = tls_proxy_cb_pd_alloc_init();
    if (tls_proxy_cb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    tls_proxy_cb_pd->tls_proxy_cb = args->tls_proxy_cb;
    // get hw-id for this TLSCB
    tls_proxy_cb_pd->hw_id = pd_tls_proxy_cb_get_base_hw_index(tls_proxy_cb_pd);

    // program tls_proxy_cb
    ret = p4pd_add_or_del_tls_proxy_cb_entry(tls_proxy_cb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    HAL_TRACE_DEBUG("Programming done");
    // add to db
    ret = add_tls_proxy_cb_pd_to_db(tls_proxy_cb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->tls_proxy_cb->pd = tls_proxy_cb_pd;

    return HAL_RET_OK;

cleanup:

    if (tls_proxy_cb_pd) {
        tls_proxy_cb_pd_free(tls_proxy_cb_pd);
    }
    return ret;
}

hal_ret_t
pd_tls_proxy_cb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tls_proxy_cb_update_args_t *args = pd_func_args->pd_tls_proxy_cb_update;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    tls_proxy_cb_t*                tls_proxy_cb = args->tls_proxy_cb;
    pd_tls_proxy_cb_t*             tls_proxy_cb_pd = (pd_tls_proxy_cb_t*)tls_proxy_cb->pd;

    HAL_TRACE_DEBUG("TLSCB pd update");

    // program tls_proxy_cb
    ret = p4pd_add_or_del_tls_proxy_cb_entry(tls_proxy_cb_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update tls_proxy_cb");
    }
    return ret;
}

hal_ret_t
pd_tls_proxy_cb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tls_proxy_cb_delete_args_t *args = pd_func_args->pd_tls_proxy_cb_delete;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    tls_proxy_cb_t*                tls_proxy_cb = args->tls_proxy_cb;
    pd_tls_proxy_cb_t*             tls_proxy_cb_pd = (pd_tls_proxy_cb_t*)tls_proxy_cb->pd;

    HAL_TRACE_DEBUG("TLSCB pd delete");

    // program tls_proxy_cb
    ret = p4pd_add_or_del_tls_proxy_cb_entry(tls_proxy_cb_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete tls_proxy_cb entry");
    }

    del_tls_proxy_cb_pd_from_db(tls_proxy_cb_pd);

    tls_proxy_cb_pd_free(tls_proxy_cb_pd);

    return ret;
}

hal_ret_t
pd_tls_proxy_cb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tls_proxy_cb_get_args_t *args = pd_func_args->pd_tls_proxy_cb_get;
    pd_tls_proxy_cb_t              tls_proxy_cb_pd;

    HAL_TRACE_DEBUG("TLSCB pd get for id: {}", args->tls_proxy_cb->cb_id);

    // allocate PD tls_proxy_cb state
    tls_proxy_cb_pd_init(&tls_proxy_cb_pd);
    tls_proxy_cb_pd.tls_proxy_cb = args->tls_proxy_cb;

    // get hw-id for this TLSCB
    tls_proxy_cb_pd.hw_id = pd_tls_proxy_cb_get_base_hw_index(&tls_proxy_cb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{:x}", tls_proxy_cb_pd.hw_id);

    // get hw tls_proxy_cb entry
    ret = p4pd_get_tls_proxy_cb_entry(&tls_proxy_cb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{:x}", tls_proxy_cb_pd.tls_proxy_cb->cb_id);
    }
    return ret;
}

uint32_t
pd_tls_proxy_cb_sesq_ci_offset_get(void)
{
    // offsetof() does not work on bitfields
    //return offsetof(tx_table_s0_t0_d, u.read_tls_stg0_d.sw_sesq_ci);
    return 44;
}

}    // namespace pd
}    // namespace hal
