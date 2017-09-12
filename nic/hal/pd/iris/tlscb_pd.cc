#include <base.h>
#include <arpa/inet.h>
#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <tlscb_pd.hpp>
#include <capri_loader.h>
#include <capri_hbm.hpp>
#include <proxy.hpp>
#include <hal.hpp>
#include <lif_manager.hpp>
#include <tls_txdma_pre_crypto_enc_p4plus_ingress.h>
#include <wring_pd.hpp>
#include <p4plus_pd_api.h>

namespace hal {
namespace pd {

void *
tlscb_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_tlscb_t *)entry)->hw_id);
}

uint32_t
tlscb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(tlscb_hw_id_t)) % ht_size;
}

bool
tlscb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tlscb_hw_id_t *)key1 == *(tlscb_hw_id_t *)key2) {
        return true;
    }
    return false;
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
    
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get tx: s0_t0_read_tls_stg0 entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tlscb_pd->tlscb->sesq_base = data.u.read_tls_stg0_d.sesq_base;
    tlscb_pd->tlscb->serq_base = data.u.read_tls_stg0_d.serq_base;
    tlscb_pd->tlscb->serq_pi = data.u.read_tls_stg0_d.pi_0;
    tlscb_pd->tlscb->serq_ci = data.u.read_tls_stg0_d.ci_0;
    tlscb_pd->tlscb->bsq_pi = data.u.read_tls_stg0_d.pi_1;
    tlscb_pd->tlscb->bsq_ci = data.u.read_tls_stg0_d.ci_1;
    tlscb_pd->tlscb->debug_dol = data.u.read_tls_stg0_d.debug_dol;
    HAL_TRACE_DEBUG("Received sesq_base: 0x{0:x}", tlscb_pd->tlscb->sesq_base);
    HAL_TRACE_DEBUG("Received serq_base: 0x{0:x}", tlscb_pd->tlscb->serq_base);
    HAL_TRACE_DEBUG("Received serq_pi: 0x{0:x}", tlscb_pd->tlscb->serq_pi);
    HAL_TRACE_DEBUG("Received serq_ci: 0x{0:x}", tlscb_pd->tlscb->serq_ci);
    HAL_TRACE_DEBUG("Received bsq_pi: 0x{0:x}", tlscb_pd->tlscb->bsq_pi);
    HAL_TRACE_DEBUG("Received bsq_ci: 0x{0:x}", tlscb_pd->tlscb->bsq_ci);
    HAL_TRACE_DEBUG("Received debug_dol: 0x{0:x}", tlscb_pd->tlscb->debug_dol);
    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_tls_tx_s3_t0_read_tls_stg1_7_entry(pd_tlscb_t* tlscb_pd)
{
    tx_table_s3_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id + 
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S3_T0_READ_TLS_ST1_7);
    
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s3_t0_read_tls_stg1_7 entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
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
    
    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s6_t0_pre_crypto_stats_entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }
    tlscb_pd->tlscb->tnmdr_alloc = ntohs(data.u.tls_pre_crypto_stats6_d.tnmdr_alloc);
    tlscb_pd->tlscb->tnmpr_alloc = ntohs(data.u.tls_pre_crypto_stats6_d.tnmpr_alloc);
    tlscb_pd->tlscb->enc_requests = ntohs(data.u.tls_pre_crypto_stats6_d.enc_requests);
    tlscb_pd->tlscb->dec_requests = ntohs(data.u.tls_pre_crypto_stats6_d.dec_requests);
    tlscb_pd->tlscb->pre_debug_stage0_7_thread = 
      (ntohs(data.u.tls_pre_crypto_stats6_d.debug_stage4_7_thread) << 16) |
      ntohs(data.u.tls_pre_crypto_stats6_d.debug_stage0_3_thread);
    HAL_TRACE_DEBUG("hwid : 0x{0:x}", hwid);    
    HAL_TRACE_DEBUG("Received tnmdr alloc: 0x{0:x}", tlscb_pd->tlscb->tnmdr_alloc);
    HAL_TRACE_DEBUG("Received tnmpr alloc: 0x{0:x}", tlscb_pd->tlscb->tnmpr_alloc);
    HAL_TRACE_DEBUG("Received enc requests: 0x{0:x}", tlscb_pd->tlscb->enc_requests);
    HAL_TRACE_DEBUG("Received dec requests: 0x{0:x}", tlscb_pd->tlscb->dec_requests);
    HAL_TRACE_DEBUG("Received pre debug stage0_7 thread: 0x{0:x}", tlscb_pd->tlscb->pre_debug_stage0_7_thread);
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
    
    ret = p4pd_get_tls_tx_s3_t0_read_tls_stg1_7_entry(tlscb_pd);
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

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            labelname,
                                            offset);
    if(ret < 0) {
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

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id + 
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S0_T0_READ_TLS_STG0);
    
    if(!del) {
        HAL_TRACE_DEBUG("TLS TXDMA Stage0 Received debug_dol 0x{0:x}", tlscb_pd->tlscb->debug_dol);

        uint64_t pc_offset;
        // get pc address
        if(p4pd_get_tls_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        HAL_TRACE_DEBUG("TLS TXDMA Stage0 Received pc address 0x{0:x}", pc_offset);

        data.action_id = pc_offset;
        data.u.read_tls_stg0_d.total = 2;

        // Get Serq address
        wring_hw_id_t  serq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SERQ,
                                     tlscb_pd->tlscb->cb_id,
                                     &serq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive serq base for tlscbcb: {}", 
                        tlscb_pd->tlscb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Serq base: 0x{0:x}", serq_base);
            data.u.read_tls_stg0_d.serq_base = htonl(serq_base);    
        }
        // Get Sesq address
        wring_hw_id_t  sesq_base;
        ret = wring_pd_get_base_addr(types::WRING_TYPE_SESQ,
                                     tlscb_pd->tlscb->cb_id,
                                     &sesq_base);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to receive sesq base for tlscbcb: {}", 
                        tlscb_pd->tlscb->cb_id);
        } else {
            HAL_TRACE_DEBUG("Sesq base: 0x{0:x}", sesq_base);
            data.u.read_tls_stg0_d.sesq_base = htonl(sesq_base);    
        }
        data.u.read_tls_stg0_d.debug_dol = htonl(tlscb_pd->tlscb->debug_dol);
        HAL_TRACE_DEBUG("debug_dol = 0x{0:x}", data.u.read_tls_stg0_d.debug_dol);

        /* FIXME : */
        data.u.read_tls_stg0_d.barco_command = (0x30000000);
        HAL_TRACE_DEBUG("Barco Command = 0x{0:x}", data.u.read_tls_stg0_d.barco_command);

        /* FIXME : */
        data.u.read_tls_stg0_d.barco_key_desc_index = 0;
        HAL_TRACE_DEBUG("Barco Key Desc Index = 0x{0:x}", data.u.read_tls_stg0_d.barco_key_desc_index);

        data.u.read_tls_stg0_d.salt = htonl(tlscb_pd->tlscb->salt);
        HAL_TRACE_DEBUG("Salt = 0x{0:x}", data.u.read_tls_stg0_d.salt);

        data.u.read_tls_stg0_d.explicit_iv = tlscb_pd->tlscb->explicit_iv;
        HAL_TRACE_DEBUG("Explicit IV = 0x{0:x}", data.u.read_tls_stg0_d.salt);
    }

    HAL_TRACE_DEBUG("TLSCB: Programming at hw-id: 0x{0:x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s0_t0_read_tls_stg0 entry for TLS CB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}


hal_ret_t 
p4pd_add_or_del_tls_tx_s3_t0_read_tls_stg1_7_entry(pd_tlscb_t* tlscb_pd, bool del)
{
    tx_table_s3_t0_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id + 
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_S3_T0_READ_TLS_ST1_7);
    
    if(!del) {
    }
    HAL_TRACE_DEBUG("TLSCB: Programming at hw-id: 0x{0:x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: s3_t0_read_tls_stg1_7 entry for TLS CB");
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
    
    ret = p4pd_add_or_del_tls_tx_s3_t0_read_tls_stg1_7_entry(tlscb_pd, del);
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
    HAL_ASSERT(NULL != tlscb_pd);
    HAL_ASSERT(NULL != tlscb_pd->tlscb);
    
    // Get the base address of TLS CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset. 
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_TLS_PROXY, 0, 0);
    HAL_TRACE_DEBUG("received offset 0x{0:x}", offset);
    return offset + \
        (tlscb_pd->tlscb->cb_id * P4PD_HBM_TLS_CB_ENTRY_SIZE);
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
pd_tlscb_create (pd_tlscb_args_t *args)
{
    hal_ret_t               ret;
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
pd_tlscb_update (pd_tlscb_args_t *args)
{
    hal_ret_t               ret;
    
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
pd_tlscb_delete (pd_tlscb_args_t *args)
{
    hal_ret_t               ret;
    
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
pd_tlscb_get (pd_tlscb_args_t *args)
{
    hal_ret_t               ret;
    pd_tlscb_t              tlscb_pd;

    HAL_TRACE_DEBUG("TLSCB pd get for id: {}", args->tlscb->cb_id);

    // allocate PD tlscb state
    tlscb_pd_init(&tlscb_pd);
    tlscb_pd.tlscb = args->tlscb;
    
    // get hw-id for this TLSCB
    tlscb_pd.hw_id = pd_tlscb_get_base_hw_index(&tlscb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{0:x}", tlscb_pd.hw_id);

    // get hw tlscb entry
    ret = p4pd_get_tlscb_entry(&tlscb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", tlscb_pd.tlscb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
