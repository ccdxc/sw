#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/tlscb_pd.hpp"
#include "nic/hal/pd/iris/internal/tcpcb_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/tls_txdma_pre_crypto_enc/include/tls_txdma_pre_crypto_enc_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "platform/capri/capri_common.hpp"
#include "nic/include/tcp_common.h"
#include "nic/include/app_redir_shared.h"

namespace hal {
namespace pd {

void *
tlscb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_tlscb_t *)entry)->hw_id);
}

uint32_t
tlscb_pd_hw_key_size ()
{
    return sizeof(tlscb_hw_id_t);
}

/********************************************
 * TxDMA
 * ******************************************/

hal_ret_t
p4pd_get_tls_tx_s0_t0_read_tls_stg0_entry(pd_tlscb_t* tlscb_pd)
{
    tx_table_s0_t0_d        data = {0};

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: s0_t0_read_tls_stg0 entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tlscb_pd->tlscb->sesq_base = data.u.read_tls_stg0_d.sesq_base;
    tlscb_pd->tlscb->serq_base = data.u.read_tls_stg0_d.serq_base;
    tlscb_pd->tlscb->serq_pi = data.u.read_tls_stg0_d.pi_0;
    tlscb_pd->tlscb->serq_ci = data.u.read_tls_stg0_d.ci_0;
    tlscb_pd->tlscb->bsq_pi = data.u.read_tls_stg0_d.pi_1;
    tlscb_pd->tlscb->bsq_ci = data.u.read_tls_stg0_d.ci_1;
    tlscb_pd->tlscb->debug_dol = ntohl(data.u.read_tls_stg0_d.debug_dol);
    tlscb_pd->tlscb->command = data.u.read_tls_stg0_d.barco_command;
    tlscb_pd->tlscb->is_decrypt_flow = data.u.read_tls_stg0_d.dec_flow;
    tlscb_pd->tlscb->l7_proxy_type = types::AppRedirType(data.u.read_tls_stg0_d.l7_proxy_type);

    HAL_TRACE_DEBUG("Received sesq_base: 0x{:x}", tlscb_pd->tlscb->sesq_base);
    HAL_TRACE_DEBUG("Received serq_base: 0x{:x}", tlscb_pd->tlscb->serq_base);
    HAL_TRACE_DEBUG("Received serq_pi: 0x{:x}", tlscb_pd->tlscb->serq_pi);
    HAL_TRACE_DEBUG("Received serq_ci: 0x{:x}", tlscb_pd->tlscb->serq_ci);
    HAL_TRACE_DEBUG("Received bsq_pi: 0x{:x}", tlscb_pd->tlscb->bsq_pi);
    HAL_TRACE_DEBUG("Received bsq_ci: 0x{:x}", tlscb_pd->tlscb->bsq_ci);
    HAL_TRACE_DEBUG("Received debug_dol: 0x{:x}", tlscb_pd->tlscb->debug_dol);
    HAL_TRACE_DEBUG("Received command: 0x{:x}", tlscb_pd->tlscb->command);
    HAL_TRACE_DEBUG("Received dec_flow : 0x{:x}", tlscb_pd->tlscb->is_decrypt_flow);
    HAL_TRACE_DEBUG("Received L7 proxy type: {}", tlscb_pd->tlscb->l7_proxy_type);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_tls_tx_s1_t0_read_tls_stg1_7_entry(pd_tlscb_t* tlscb_pd)
{
    tx_table_s1_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S1_T0_READ_TLS_ST1_7);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s1_t0_read_tls_stg1_7 entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tlscb_pd->tlscb->other_fid = ntohs(data.u.read_tls_stg1_7_d.other_fid);
    HAL_TRACE_DEBUG("Received other fid: 0x{:x}", tlscb_pd->tlscb->other_fid);

    tlscb_pd->tlscb->crypto_hmac_key_idx = data.u.read_tls_stg1_7_d.barco_hmac_key_desc_index;
    HAL_TRACE_DEBUG("Received HMAC key index: 0x{:x}", tlscb_pd->tlscb->crypto_hmac_key_idx);
    return ret;
}

hal_ret_t
p4pd_get_tls_tx_s6_t0_pre_crypto_stats_entry(pd_tlscb_t* tlscb_pd)
{
    tx_table_s6_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_PRE_CRYPTO_STATS_U16);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s6_t0_pre_crypto_stats_entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tlscb_pd->tlscb->tnmdpr_alloc = ntohs(data.u.tls_pre_crypto_stats6_d.tnmdpr_alloc);
    tlscb_pd->tlscb->enc_requests = ntohs(data.u.tls_pre_crypto_stats6_d.enc_requests);
    tlscb_pd->tlscb->dec_requests = ntohs(data.u.tls_pre_crypto_stats6_d.dec_requests);
    tlscb_pd->tlscb->mac_requests = ntohs(data.u.tls_pre_crypto_stats6_d.mac_requests);
    tlscb_pd->tlscb->pre_debug_stage0_7_thread =
      (ntohs(data.u.tls_pre_crypto_stats6_d.debug_stage4_7_thread) << 16) |
      ntohs(data.u.tls_pre_crypto_stats6_d.debug_stage0_3_thread);
    HAL_TRACE_DEBUG("hwid : {:#x}", hwid);
    HAL_TRACE_DEBUG("Received tnmdpr alloc: 0x{:x}", tlscb_pd->tlscb->tnmdpr_alloc);
    HAL_TRACE_DEBUG("Received enc requests: 0x{:x}", tlscb_pd->tlscb->enc_requests);
    HAL_TRACE_DEBUG("Received dec requests: 0x{:x}", tlscb_pd->tlscb->dec_requests);
    HAL_TRACE_DEBUG("Received mac requests: 0x{:x}", tlscb_pd->tlscb->mac_requests);
    HAL_TRACE_DEBUG("Received pre debug stage0_7 thread: 0x{:x}", tlscb_pd->tlscb->pre_debug_stage0_7_thread);
    HAL_TRACE_DEBUG("Received l7_proxy_type: {}", tlscb_pd->tlscb->l7_proxy_type);
    return ret;
}

hal_ret_t
p4pd_get_tls_tx_config_entry(pd_tlscb_t* tlscb_pd)
{
    tx_table_s2_t0_d                data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_CONFIG);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: config entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }

    tlscb_pd->tlscb->crypto_key_idx = data.u.tls_rx_serq_d.barco_key_desc_index;
    tlscb_pd->tlscb->explicit_iv = data.u.tls_rx_serq_d.sequence_no;
    tlscb_pd->tlscb->salt = ntohl(data.u.tls_rx_serq_d.salt);
    tlscb_pd->tlscb->cpu_id = data.u.tls_rx_serq_d.cpu_id;


    HAL_TRACE_DEBUG("Received crypto_key_idx: 0x{:x}", tlscb_pd->tlscb->crypto_key_idx);
    HAL_TRACE_DEBUG("Received explicit_iv: 0x{:x}", tlscb_pd->tlscb->explicit_iv);
    HAL_TRACE_DEBUG("Received salt: 0x{:x}", tlscb_pd->tlscb->salt);
    HAL_TRACE_DEBUG("Received cpu_id: {}", tlscb_pd->tlscb->cpu_id);

    return ret;
}

hal_ret_t
p4pd_get_tlscb_txdma_entry(pd_tlscb_t* tlscb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_tls_tx_s0_t0_read_tls_stg0_entry(tlscb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_s1_t0_read_tls_stg1_7_entry(tlscb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_config_entry(tlscb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_s6_t0_pre_crypto_stats_entry(tlscb_pd);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_get_tls_tx_s5_t0_post_crypto_stats_entry(tlscb_pd);
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
p4pd_add_or_del_tls_tx_s0_t0_read_tls_stg0_entry(pd_tlscb_t* tlscb_pd, bool del)
{
    tx_table_s0_t0_d        data = {0};
    hal_ret_t               ret = HAL_RET_OK;
    wring_hw_id_t           q_base;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0);

    if(!del) {
        HAL_TRACE_DEBUG("TLS TXDMA Stage0 Received debug_dol 0x{:x}", tlscb_pd->tlscb->debug_dol);

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
                                     tlscb_pd->tlscb->cb_id,
                                     &serq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive serq base for tlscbcb: {}",
                        tlscb_pd->tlscb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Serq id: 0x{:x}", tlscb_pd->tlscb->cb_id);
            HAL_TRACE_DEBUG("Serq base: 0x{:x}", serq_base);
            data.u.read_tls_stg0_d.serq_base = htonl(serq_base);
        }
        // Get Sesq address
        wring_hw_id_t  sesq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                     ((tlscb_pd->tlscb->other_fid == 0xFFFF) ?
                                      tlscb_pd->tlscb->cb_id : tlscb_pd->tlscb->other_fid),
                                     &sesq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive sesq base for tlscbcb: {}",
                        tlscb_pd->tlscb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Sesq id: 0x{:x}", tlscb_pd->tlscb->other_fid);
            HAL_TRACE_DEBUG("Sesq base: 0x{:x}", sesq_base);
            data.u.read_tls_stg0_d.sesq_base = htonl(sesq_base);
        }
        data.u.read_tls_stg0_d.debug_dol = htonl(tlscb_pd->tlscb->debug_dol);
        HAL_TRACE_DEBUG("debug_dol = 0x{:x}", data.u.read_tls_stg0_d.debug_dol);

        data.u.read_tls_stg0_d.barco_command = tlscb_pd->tlscb->command;
        HAL_TRACE_DEBUG("Barco Command = 0x{:x}", data.u.read_tls_stg0_d.barco_command);

        data.u.read_tls_stg0_d.pi_0 = tlscb_pd->tlscb->serq_pi;
        HAL_TRACE_DEBUG("SERQ PI = 0x{:x}", data.u.read_tls_stg0_d.pi_0);

        data.u.read_tls_stg0_d.ci_0 = tlscb_pd->tlscb->serq_ci;
        HAL_TRACE_DEBUG("SERQ CI = 0x{:x}", data.u.read_tls_stg0_d.ci_0);

        HAL_TRACE_DEBUG("RAW[0x{:x}]: is_decrypt_flow: 0x{:x}", tlscb_pd->tlscb->cb_id, tlscb_pd->tlscb->is_decrypt_flow);
        if (tlscb_pd->tlscb->is_decrypt_flow)
            data.u.read_tls_stg0_d.dec_flow = 1;
        HAL_TRACE_DEBUG("Is Decrypt Flow = {}", data.u.read_tls_stg0_d.dec_flow ? "TRUE" : "FALSE");

        data.u.read_tls_stg0_d.l7_proxy_type = tlscb_pd->tlscb->l7_proxy_type;
        HAL_TRACE_DEBUG("l7_proxy_type = {}", data.u.read_tls_stg0_d.l7_proxy_type);

        /* record ring */
        ret = wring_pd_get_base_addr(types::WRING_TYPE_BSQ,
                                     tlscb_pd->tlscb->cb_id, &q_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to retrieve record queue base base for proxyrcb_id: {}",
                    tlscb_pd->tlscb->cb_id);
        } else {
            HAL_TRACE_DEBUG("recqbase id: {:#x}, base: {:#x}", tlscb_pd->tlscb->cb_id, q_base);
            data.u.read_tls_stg0_d.recq_base = (uint32_t) q_base;
            data.u.read_tls_stg0_d.recq_pi = 0;
            data.u.read_tls_stg0_d.recq_ci = 0;
        }

        data.u.read_tls_stg0_d.serq_prod_ci_addr =
            htonl(tcpcb_pd_serq_prod_ci_addr_get(tlscb_pd->tlscb->cb_id));
        HAL_TRACE_DEBUG("qid {:#x} serq_prod_ci_addr {:#x}",
                tlscb_pd->tlscb->cb_id,
                ntohl(data.u.read_tls_stg0_d.serq_prod_ci_addr));

        uint64_t                gc_base;
        // get gc address
        gc_base = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_GC, CAPRI_HBM_GC_RNMDR_QTYPE,
                    CAPRI_RNMDR_GC_TLS_RING_PRODUCER) + TCP_GC_CB_SW_PI_OFFSET;
        HAL_TRACE_DEBUG("gc_base: {:#x}", gc_base);
        data.u.read_tls_stg0_d.gc_base = htonl(gc_base);
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
p4pd_add_or_del_tls_tx_s1_t0_read_tls_stg1_7_entry(pd_tlscb_t* tlscb_pd, bool del)
{
    tx_table_s1_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S1_T0_READ_TLS_ST1_7);

    if(!del) {
        data.u.read_tls_stg1_7_d.other_fid = htons(tlscb_pd->tlscb->other_fid);
        HAL_TRACE_DEBUG("other fid = 0x{:x}", data.u.read_tls_stg1_7_d.other_fid);

        // Get L7Q address
        wring_hw_id_t  q_base;
        uint32_t proxyrcb_id = tlscb_pd->tlscb->cb_id;

        ret = wring_pd_get_base_addr(types::WRING_TYPE_APP_REDIR_PROXYR,
                                     proxyrcb_id, &q_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive l7q base for proxyrcb_id: {}",
                    proxyrcb_id);
        } else {
            HAL_TRACE_DEBUG("l7q id: {:#x}, base: {:#x}", proxyrcb_id, q_base);
            data.u.read_tls_stg1_7_d.l7q_base = q_base;
        }

        data.u.read_tls_stg1_7_d.barco_hmac_key_desc_index = tlscb_pd->tlscb->crypto_hmac_key_idx;
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
p4pd_add_or_del_tls_tx_config_entry(pd_tlscb_t* tlscb_pd, bool del)
{
    tx_table_s2_t0_d                data = {0};
    hal_ret_t                       ret = HAL_RET_OK;

    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_CONFIG);

    if (!del) {
        data.u.tls_rx_serq_d.barco_key_desc_index = tlscb_pd->tlscb->crypto_key_idx;
        HAL_TRACE_DEBUG("Barco Key Desc Index = 0x{:x}", data.u.tls_rx_serq_d.barco_key_desc_index);

        data.u.tls_rx_serq_d.sequence_no = (tlscb_pd->tlscb->explicit_iv);
        HAL_TRACE_DEBUG("Sequence Number = 0x{:x}", data.u.tls_rx_serq_d.sequence_no);

        data.u.tls_rx_serq_d.salt = tlscb_pd->tlscb->salt;
        HAL_TRACE_DEBUG("Salt = 0x{:x}", data.u.tls_rx_serq_d.salt);

        data.u.tls_rx_serq_d.cpu_id = tlscb_pd->tlscb->cpu_id;
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
p4pd_add_or_del_tlscb_txdma_entry(pd_tlscb_t* tlscb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tls_tx_s0_t0_read_tls_stg0_entry(tlscb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tls_tx_s1_t0_read_tls_stg1_7_entry(tlscb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_tls_tx_config_entry(tlscb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    return HAL_RET_OK;

cleanup:

    /* TODO: Cleanup */
    return ret;
}

/**************************/

tlscb_hw_id_t
pd_tlscb_get_base_hw_index(pd_tlscb_t* tlscb_pd)
{
    SDK_ASSERT(NULL != tlscb_pd);
    SDK_ASSERT(NULL != tlscb_pd->tlscb);

    // Get the base address of TLS CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    uint64_t offset = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_TLS_PROXY, 0,
            tlscb_pd->tlscb->cb_id);
    HAL_TRACE_DEBUG("received offset 0x{:x}", offset);
    return offset;
}

hal_ret_t
p4pd_add_or_del_tlscb_entry(pd_tlscb_t* tlscb_pd, bool del)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_tlscb_txdma_entry(tlscb_pd, del);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add/delete TLS TxDMA Entry {}", ret);
    }

    return ret;
}

static
hal_ret_t
p4pd_get_tlscb_entry(pd_tlscb_t* tlscb_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_tlscb_txdma_entry(tlscb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for tlscb");
    }

    return ret;
}

/********************************************
 * APIs
 *******************************************/

hal_ret_t
pd_tlscb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tlscb_create_args_t *args = pd_func_args->pd_tlscb_create;
    pd_tlscb_s              *tlscb_pd;

    HAL_TRACE_DEBUG("Creating pd state for TLS CB.");

    // allocate PD tlscb state
    tlscb_pd = tlscb_pd_alloc_init();
    if (tlscb_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    tlscb_pd->tlscb = args->tlscb;
    // get hw-id for this TLSCB
    tlscb_pd->hw_id = pd_tlscb_get_base_hw_index(tlscb_pd);

    // program tlscb
    ret = p4pd_add_or_del_tlscb_entry(tlscb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    HAL_TRACE_DEBUG("Programming done");
    // add to db
    ret = add_tlscb_pd_to_db(tlscb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->tlscb->pd = tlscb_pd;

    return HAL_RET_OK;

cleanup:

    if (tlscb_pd) {
        tlscb_pd_free(tlscb_pd);
    }
    return ret;
}

hal_ret_t
pd_tlscb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tlscb_update_args_t *args = pd_func_args->pd_tlscb_update;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    tlscb_t*                tlscb = args->tlscb;
    pd_tlscb_t*             tlscb_pd = (pd_tlscb_t*)tlscb->pd;

    HAL_TRACE_DEBUG("TLSCB pd update");

    // program tlscb
    ret = p4pd_add_or_del_tlscb_entry(tlscb_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update tlscb");
    }
    return ret;
}

hal_ret_t
pd_tlscb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tlscb_delete_args_t *args = pd_func_args->pd_tlscb_delete;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    tlscb_t*                tlscb = args->tlscb;
    pd_tlscb_t*             tlscb_pd = (pd_tlscb_t*)tlscb->pd;

    HAL_TRACE_DEBUG("TLSCB pd delete");

    // program tlscb
    ret = p4pd_add_or_del_tlscb_entry(tlscb_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete tlscb entry");
    }

    del_tlscb_pd_from_db(tlscb_pd);

    tlscb_pd_free(tlscb_pd);

    return ret;
}

hal_ret_t
pd_tlscb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_tlscb_get_args_t *args = pd_func_args->pd_tlscb_get;
    pd_tlscb_t              tlscb_pd;

    HAL_TRACE_DEBUG("TLSCB pd get for id: {}", args->tlscb->cb_id);

    // allocate PD tlscb state
    tlscb_pd_init(&tlscb_pd);
    tlscb_pd.tlscb = args->tlscb;

    // get hw-id for this TLSCB
    tlscb_pd.hw_id = pd_tlscb_get_base_hw_index(&tlscb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{:x}", tlscb_pd.hw_id);

    // get hw tlscb entry
    ret = p4pd_get_tlscb_entry(&tlscb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{:x}", tlscb_pd.tlscb->cb_id);
    }
    return ret;
}

uint32_t
pd_tlscb_sesq_ci_offset_get(void)
{
    // offsetof() does not work on bitfields
    //return offsetof(tx_table_s0_t0_d, u.read_tls_stg0_d.sw_sesq_ci);
    return 44;
}

}    // namespace pd
}    // namespace hal
