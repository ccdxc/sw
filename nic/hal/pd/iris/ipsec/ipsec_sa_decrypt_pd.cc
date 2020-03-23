#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/ipsec/ipsec_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/esp_v4_tunnel_n2h_rxdma/include/esp_v4_tunnel_n2h_rxdma_p4plus_ingress.h"
#include "gen/p4gen/esp_v4_tunnel_n2h_txdma1/include/esp_v4_tunnel_n2h_txdma1_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

namespace hal {
namespace pd {

hal_ret_t p4pd_get_ipsec_sa_decrypt_tx_stage0_prog_addr(uint64_t* offset);

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_ipsec_sa_decrypt_rx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "ipsec_rx_n2h_stage0";

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

static hal_ret_t
p4pd_add_or_del_ipsec_decrypt_rx_stage0_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    common_p4plus_stage0_app_header_table_d     data = {0};
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    uint64_t                                    ipsec_cb_ring_addr;
    uint64_t                                    ipsec_barco_ring_addr;
    uint16_t                                    key_index;
    pd_vrf_t                                    *pd_vrf;
    uint8_t                                     zeros[P4PD_HBM_IPSEC_CB_ENTRY_SIZE];

    memset(zeros, 0, P4PD_HBM_IPSEC_CB_ENTRY_SIZE);
    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_QSTATE1);

    if(!del) {
        // get pc address
        if(p4pd_get_ipsec_sa_decrypt_rx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        data.action_id = pc_offset;
        HAL_TRACE_DEBUG("Received Rx Decrypt pc address {}", pc_offset);
        if (p4pd_get_ipsec_sa_decrypt_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
        }
        HAL_TRACE_DEBUG("Received Decrypt TX pc address {}", pc_offset);
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.total = 2;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.iv_size = ipsec_sa_pd->ipsec_sa->iv_size;
        //data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.block_size = ipsec_sa_pd->ipsec_sa->block_size;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.icv_size = ipsec_sa_pd->ipsec_sa->icv_size;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.ipsec_cb_index = htons(ipsec_sa_pd->ipsec_sa->sa_id - (HAL_MAX_IPSEC_SA/4));

        //data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_enc_cmd = ipsec_sa_pd->ipsec_sa->barco_enc_cmd;
        // for now aes-decrypt-encoding hard-coded.
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_enc_cmd = 0x30100000;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.expected_seq_no = ipsec_sa_pd->ipsec_sa->esn_lo;
        key_index = ipsec_sa_pd->ipsec_sa->key_index;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.key_index = htons(key_index);
        key_index = ipsec_sa_pd->ipsec_sa->new_key_index;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.new_key_index = htons(key_index);

        HAL_TRACE_DEBUG("HW- key_index {}, new_key_index {}", data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.key_index, data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.new_key_index);
        HAL_TRACE_DEBUG("key_index {}, new_key_index {}", ipsec_sa_pd->ipsec_sa->key_index, ipsec_sa_pd->ipsec_sa->new_key_index);
        // the below may have to use a different range for the reverse direction

        ipsec_cb_ring_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_IPSECCB) +
            ((asicpd_get_mem_size_kb(CAPRI_HBM_REG_IPSECCB)/2) * 1024) +
            ((ipsec_sa_pd->ipsec_sa->sa_id - (HAL_MAX_IPSEC_SA/4)) *
             DEFAULT_WRING_SLOT_SIZE * IPSEC_PER_CB_RING_SIZE);
        HAL_TRACE_DEBUG("CB Ring Addr {:#x}", ipsec_cb_ring_addr);

        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_ring_base_addr = htonl((uint32_t)ipsec_cb_ring_addr & 0xFFFFFFFF);
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_cindex = 0;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_pindex = 0;

        ipsec_barco_ring_addr =
            asicpd_get_mem_addr(CAPRI_HBM_REG_IPSECCB_BARCO) +
            ((asicpd_get_mem_size_kb(CAPRI_HBM_REG_IPSECCB_BARCO)/2) * 1024) +
            ((ipsec_sa_pd->ipsec_sa->sa_id - (HAL_MAX_IPSEC_SA/4)) *
             IPSEC_PER_CB_BARCO_SLOT_ELEM_SIZE * IPSEC_PER_CB_BARCO_RING_SIZE);
        HAL_TRACE_DEBUG("Barco Ring Addr {:#x}", ipsec_barco_ring_addr);

        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_ring_base_addr = htonll(ipsec_barco_ring_addr);
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_cindex = 0;
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_pindex = 0;

        vrf_t *vrf = vrf_lookup_by_id(ipsec_sa_pd->ipsec_sa->vrf);
        if (vrf) {
            pd_vrf = (pd_vrf_t*)(vrf->pd);
            ipsec_sa_pd->ipsec_sa->vrf_vlan = pd_vrf->vrf_fromcpu_vlan_id;
            data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.vrf_vlan = htons(ipsec_sa_pd->ipsec_sa->vrf_vlan);
            HAL_TRACE_DEBUG("Vrf VLAN {}", ipsec_sa_pd->ipsec_sa->vrf_vlan);
        }
        data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.is_v6 = ipsec_sa_pd->ipsec_sa->is_v6;
    }
    HAL_TRACE_DEBUG("Programming Decrypt stage0 at hw-id: 0x{:#x}", hwid);
    if (!p4plus_hbm_write(hwid, (uint8_t*)&zeros, sizeof(zeros), P4PLUS_CACHE_INVALIDATE_BOTH)) {
        HAL_TRACE_ERR("Failed to create rx: stage0 entry for IPSECCB");
        ret = HAL_RET_HW_FAIL;
    }
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: stage0 entry for IPSECCB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

static hal_ret_t
p4pd_add_or_del_ipsec_decrypt_part2(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;
    tx_table_s1_t2_esp_v4_tunnel_n2h_load_part2_d decrypt_part2;

    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_QSTATE2);

    decrypt_part2.spi = htonl(ipsec_sa_pd->ipsec_sa->spi);
    decrypt_part2.new_spi = htonl(ipsec_sa_pd->ipsec_sa->new_spi);

    HAL_TRACE_DEBUG("Programming Decrypt part2 at hw-id: 0x{:#x}", hwid);
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&decrypt_part2, sizeof(decrypt_part2),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create part2 entry for IPSECCB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_ipsec_decrypt_rxdma_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_ipsec_decrypt_rx_stage0_entry(ipsec_sa_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_or_del_ipsec_decrypt_part2(ipsec_sa_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

hal_ret_t
p4pd_get_ipsec_decrypt_rx_stage0_entry(pd_ipsec_t* ipsec_sa_pd)
{
    common_p4plus_stage0_app_header_table_d data = {0};
    uint64_t ipsec_cb_ring_addr;
    uint8_t cb_cindex, cb_pindex;
    uint64_t ipsec_barco_ring_addr;
    uint8_t barco_cindex, barco_pindex;
    uint64_t replay_seq_no_bmp = 0;
    uint32_t expected_seq_no;

    ipsec_sa_t *ipsec_sa = ipsec_sa_pd->ipsec_sa;

    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_QSTATE1);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }

    ipsec_sa->iv_size = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.iv_size;
    //ipsec_sa->block_size = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.block_size;
    ipsec_sa->icv_size = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.icv_size;
    ipsec_sa->barco_enc_cmd = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_enc_cmd;
    ipsec_sa->key_index = ntohs(data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.key_index);
    ipsec_sa->new_key_index = ntohs(data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.new_key_index);

    ipsec_cb_ring_addr = ntohll(data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_ring_base_addr);
    cb_cindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_cindex;
    cb_pindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_pindex;
    HAL_TRACE_DEBUG("CB Ring Addr {:#x} Pindex {} CIndex {}", ipsec_cb_ring_addr, cb_pindex, cb_cindex);

    ipsec_barco_ring_addr = ntohll(data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_ring_base_addr);
    cb_cindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_cindex;
    cb_pindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_pindex;
    HAL_TRACE_DEBUG("Barco Ring Addr {:#x} Pindex {} CIndex {}", ipsec_barco_ring_addr, barco_pindex, barco_cindex);

    expected_seq_no = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.expected_seq_no;
    replay_seq_no_bmp = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.replay_seq_no_bmp;
    ipsec_sa->expected_seq_no = ntohl(expected_seq_no);
    ipsec_sa->seq_no_bmp = ntohll(replay_seq_no_bmp);
    ipsec_sa->vrf_vlan = ntohs(data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.vrf_vlan);
    ipsec_sa->is_v6 = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.is_v6;

    ipsec_sa->cb_cindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_cindex;
    ipsec_sa->cb_pindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.cb_pindex;
    ipsec_sa->barco_pindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_pindex;
    ipsec_sa->barco_cindex = data.u.esp_v4_tunnel_n2h_rxdma_initial_table_d.barco_cindex;
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_ipsec_decrypt_rx_stage0_entry_part2(pd_ipsec_t* ipsec_sa_pd)
{
    pd_ipsec_decrypt_part2_t  decrypt_part2;
    ipsec_sa_t *ipsec_sa = ipsec_sa_pd->ipsec_sa;

    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_QSTATE2);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&decrypt_part2, sizeof(decrypt_part2))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipsec_sa->last_replay_seq_no = ntohl(decrypt_part2.last_replay_seq_no);
    ipsec_sa->iv_salt = decrypt_part2.iv_salt;
    ipsec_sa->spi = ntohl(decrypt_part2.spi);
    ipsec_sa->new_spi = ntohl(decrypt_part2.new_spi);
    HAL_TRACE_DEBUG("spi {} new_spi {}", ipsec_sa->spi, ipsec_sa->new_spi);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_ipsec_sa_decrypt_stats(pd_ipsec_t* ipsec_sa_pd)
{
    ipsec_sa_t  *ipsec_sa = ipsec_sa_pd->ipsec_sa;
    rx_table_s4_t1_ipsec_rxdma_stats_update_d stats_data;
    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_DECRYPT_STATS);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&stats_data, sizeof(stats_data))){
        HAL_TRACE_ERR("Failed to get Stats: entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipsec_sa->total_rx_pkts = ntohll(stats_data.n2h_rx_pkts);
    ipsec_sa->total_rx_bytes = ntohll(stats_data.n2h_rx_bytes);
    ipsec_sa->total_rx_drops = ntohll(stats_data.n2h_rx_drops);
    ipsec_sa->total_pkts = ntohll(stats_data.n2h_tx_pkts);
    ipsec_sa->total_bytes = ntohll(stats_data.n2h_tx_bytes);
    ipsec_sa->total_drops = ntohll(stats_data.n2h_tx_drops);

    HAL_TRACE_DEBUG("Stats: h2n: rx_pkts {} rx_bytes {} rx_drops {} tx_pkts {} tx_bytes {} tx_drops {}",
        ipsec_sa->total_rx_pkts, ipsec_sa->total_rx_bytes,
        ipsec_sa->total_rx_drops, ipsec_sa->total_pkts,
        ipsec_sa->total_bytes, ipsec_sa->total_drops);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_ipsec_sa_decrypt_rxdma_entry(pd_ipsec_t* ipsec_sa_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_ipsec_decrypt_rx_stage0_entry(ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ipsec_rx entry");
        goto cleanup;
    }
    ret = p4pd_get_ipsec_decrypt_rx_stage0_entry_part2(ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ipsec_rx entry");
        goto cleanup;
    }
    ret = p4pd_get_ipsec_sa_decrypt_stats(ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ipsec stats entry");
        goto cleanup;
    }
    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

/********************************************
 * TxDMA
 * ******************************************/
hal_ret_t
p4pd_get_ipsec_sa_decrypt_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "ipsec_tx_n2h_stage0";

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

/**************************/

ipsec_sa_hw_id_t
pd_ipsec_decrypt_get_base_hw_index(pd_ipsec_t* ipsec_sa_pd)
{
    SDK_ASSERT(NULL != ipsec_sa_pd);
    SDK_ASSERT(NULL != ipsec_sa_pd->ipsec_sa);

    // Get the base address of IPSEC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    uint64_t base = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_IPSEC_ESP, 1, 0);
    uint64_t offset = base + ((ipsec_sa_pd->ipsec_sa->sa_id - (HAL_MAX_IPSEC_SA/4)) * P4PD_HBM_IPSEC_CB_ENTRY_SIZE);
    HAL_TRACE_DEBUG("received decrypt base {:#x} offset {:#x}", base, offset);
    return offset;
}

hal_ret_t
p4pd_add_or_del_ipsec_decrypt_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_ipsec_decrypt_rxdma_entry(ipsec_sa_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;
    }

err:
    return ret;
}

static
hal_ret_t
p4pd_get_ipsec_decrypt_entry(pd_ipsec_t* ipsec_sa_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_ipsec_sa_decrypt_rxdma_entry(ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rxdma entry for ipseccb");
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
pd_ipsec_decrypt_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipsec_decrypt_create_args_t *args = pd_func_args->pd_ipsec_decrypt_create;
    pd_func_args_t          pd_func_args1 = {0};
    pd_ipsec_s              *ipsec_sa_pd;
    ipsec_sa_t*             ipsec_sa = args->ipsec_sa;

    pd_crypto_alloc_key_args_t key_index_args;
    pd_crypto_write_key_args_t write_key_args;
    crypto_key_t crypto_key;

    HAL_TRACE_DEBUG("Creating pd state for IPSEC CB.");

    // allocate PD ipsec_sa state
    ipsec_sa_pd = ipsec_pd_decrypt_alloc_init();
    if (ipsec_sa_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    ipsec_sa_pd->ipsec_sa = args->ipsec_sa;
    // get hw-id for this IPSECCB
    ipsec_sa_pd->hw_id = pd_ipsec_decrypt_get_base_hw_index(ipsec_sa_pd);

    //Orig Key
    key_index_args.key_idx = &ipsec_sa->key_index;

    pd_func_args1.pd_crypto_alloc_key = &key_index_args;
    ret = pd_crypto_alloc_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create key index");
        goto cleanup;
    }
    //ipsec_sa_pd->ipsec_sa->key_index = key_index_args.key_idx;
    crypto_key.key_type = (types::CryptoKeyType)ipsec_sa->key_type;
    crypto_key.key_size = ipsec_sa->key_size;
    memcpy(crypto_key.key, ipsec_sa->key, ipsec_sa->key_size);
    write_key_args.key_idx = ipsec_sa->key_index;
    write_key_args.key = &crypto_key;
    pd_func_args1.pd_crypto_write_key = &write_key_args;
    ret = pd_crypto_write_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write key at index");
        goto cleanup;
    }

    //New Key
    key_index_args.key_idx = &ipsec_sa->new_key_index;

    pd_func_args1.pd_crypto_alloc_key = &key_index_args;
    ret = pd_crypto_alloc_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create key index");
        goto cleanup;
    }
    ipsec_sa_pd->ipsec_sa->new_key_index = *(key_index_args.key_idx);
    crypto_key.key_type = (types::CryptoKeyType)ipsec_sa->new_key_type;
    crypto_key.key_size = ipsec_sa->new_key_size;
    memcpy(crypto_key.key, ipsec_sa->new_key, ipsec_sa->new_key_size);
    write_key_args.key_idx = ipsec_sa->new_key_index;
    write_key_args.key = &crypto_key;
    pd_func_args1.pd_crypto_write_key = &write_key_args;
    ret = pd_crypto_write_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write key at index");
        goto cleanup;
    }

    // program ipsec_sa
    ret = p4pd_add_or_del_ipsec_decrypt_entry(ipsec_sa_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    // add to db
    ret = add_ipsec_pd_decrypt_to_db(ipsec_sa_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->ipsec_sa->pd_decrypt = ipsec_sa_pd;

    return HAL_RET_OK;

cleanup:

    if (ipsec_sa_pd) {
        ipsec_pd_decrypt_free(ipsec_sa_pd);
    }
    return ret;
}

hal_ret_t
pd_ipsec_decrypt_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipsec_decrypt_update_args_t *args = pd_func_args->pd_ipsec_decrypt_update;
    pd_func_args_t          pd_func_args1 = {0};

    pd_crypto_write_key_args_t write_key_args;
    pd_crypto_alloc_key_args_t key_index_args;
    crypto_key_t crypto_key;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    ipsec_sa_t*                ipsec_sa = args->ipsec_sa;
    pd_ipsec_t*        ipsec_sa_pd = (pd_ipsec_t*)ipsec_sa->pd_decrypt;

    HAL_TRACE_DEBUG("IPSECCB pd update");

    crypto_key.key_type = (types::CryptoKeyType)ipsec_sa->key_type;
    crypto_key.key_size = ipsec_sa->key_size;
    memcpy(crypto_key.key, ipsec_sa->key, ipsec_sa->key_size);
    write_key_args.key_idx = ipsec_sa->key_index;
    write_key_args.key = &crypto_key;

    pd_func_args1.pd_crypto_write_key = &write_key_args;
    ret = pd_crypto_write_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write key at index{}", ipsec_sa->key_index);
       return HAL_RET_INVALID_ARG;
    }
    //New Key
    key_index_args.key_idx = &ipsec_sa->new_key_index;

    pd_func_args1.pd_crypto_alloc_key = &key_index_args;
    ret = pd_crypto_alloc_key(&pd_func_args1);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create key index");
        return HAL_RET_INVALID_ARG;
    }
    ipsec_sa_pd->ipsec_sa->new_key_index = *(key_index_args.key_idx);
    crypto_key.key_type = (types::CryptoKeyType)ipsec_sa->new_key_type;
    crypto_key.key_size = ipsec_sa->new_key_size;
    memcpy(crypto_key.key, ipsec_sa->new_key, ipsec_sa->new_key_size);
    write_key_args.key_idx = ipsec_sa->new_key_index;
    write_key_args.key = &crypto_key;
    pd_func_args1.pd_crypto_write_key = &write_key_args;
    ret = pd_crypto_write_key(&pd_func_args1);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write key at index");
        return HAL_RET_INVALID_ARG;
    }

    // program ipsec_sa
    ret = p4pd_add_or_del_ipsec_decrypt_entry(ipsec_sa_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update ipsec_sa");
    }
    return ret;
}

hal_ret_t
pd_ipsec_decrypt_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipsec_decrypt_delete_args_t *args = pd_func_args->pd_ipsec_decrypt_delete;
    pd_func_args_t          pd_func_args1 = {0};
    pd_crypto_free_key_args_t  free_key_args;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    ipsec_sa_t*                ipsec_sa = args->ipsec_sa;
    pd_ipsec_t*       ipsec_sa_pd = (pd_ipsec_t*)ipsec_sa->pd_decrypt;

    if (ipsec_sa_pd == NULL) {
       return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("IPSECCB pd delete");

    free_key_args.key_idx = ipsec_sa->key_index;

    pd_func_args1.pd_crypto_free_key = &free_key_args;
    ret = pd_crypto_free_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ipsec key at key_index {}", ipsec_sa->key_index);
    }


    if (ipsec_sa->new_key_index != 0) {
        free_key_args.key_idx = ipsec_sa->new_key_index;

        pd_func_args1.pd_crypto_free_key = &free_key_args;
        ret = pd_crypto_free_key(&pd_func_args1);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete ipsec key at key_index {}", ipsec_sa->new_key_index);
        }
    }

    // program ipsec_sa
    ret = p4pd_add_or_del_ipsec_decrypt_entry(ipsec_sa_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ipsec_sa entry");
    }

    del_ipsec_pd_decrypt_from_db(ipsec_sa_pd);

    ipsec_pd_decrypt_free(ipsec_sa_pd);

    return ret;
}

hal_ret_t
pd_ipsec_decrypt_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipsec_decrypt_get_args_t *args = pd_func_args->pd_ipsec_decrypt_get;
    pd_func_args_t          pd_func_args1 = {0};
    pd_ipsec_t      ipsec_sa_pd;

    pd_crypto_read_key_args_t read_key;
    crypto_key_t              key;

    HAL_TRACE_DEBUG("IPSECCB pd get for id: {}", (args->ipsec_sa->sa_id - (HAL_MAX_IPSEC_SA/4)));

    // allocate PD ipsec_sa state
    ipsec_pd_decrypt_init(&ipsec_sa_pd);
    ipsec_sa_pd.ipsec_sa = args->ipsec_sa;

    // get hw-id for this IPSECCB
    ipsec_sa_pd.hw_id = pd_ipsec_decrypt_get_base_hw_index(&ipsec_sa_pd);
    HAL_TRACE_DEBUG("Received hw-id {:#x}", ipsec_sa_pd.hw_id);

    // get hw ipsec_sa entry
    ret = p4pd_get_ipsec_decrypt_entry(&ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{:#x}", (ipsec_sa_pd.ipsec_sa->sa_id - (HAL_MAX_IPSEC_SA/4)));
    }

    read_key.key_idx = ipsec_sa_pd.ipsec_sa->key_index;
    memset(&key, 0, sizeof(crypto_key_t));
    read_key.key = &key;

    pd_func_args1.pd_crypto_read_key = &read_key;
    ret = pd_crypto_read_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for key_index: {} ret{}", ipsec_sa_pd.ipsec_sa->key_index, ret);
        //return ret;
        ret = HAL_RET_OK;
    }
    HAL_TRACE_DEBUG("IPSEC SA pd get key_index for id: {}", args->ipsec_sa->key_index);
    HAL_TRACE_DEBUG("IPSEC SA key_type, key_size : {}, {}", read_key.key->key_type, read_key.key->key_size);
    if ((read_key.key->key_size > 0) && (read_key.key->key_size <= 32)) {
        memcpy(ipsec_sa_pd.ipsec_sa->key, read_key.key->key, read_key.key->key_size);
    }

    read_key.key_idx = ipsec_sa_pd.ipsec_sa->new_key_index;
    memset(&key, 0, sizeof(crypto_key_t));
    read_key.key = &key;

    pd_func_args1.pd_crypto_read_key = &read_key;
    ret = pd_crypto_read_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for new_key_index: {} ret{}", ipsec_sa_pd.ipsec_sa->new_key_index, ret);
        //return ret;
        ret = HAL_RET_OK;
    }
    HAL_TRACE_DEBUG("IPSEC SA pd get new_key_index for id: {}", args->ipsec_sa->new_key_index);
    HAL_TRACE_DEBUG("IPSEC SA key_type, key_size : {}, {}", read_key.key->key_type, read_key.key->key_size);
    if ((read_key.key->key_size > 0) && (read_key.key->key_size <= 32)) {
        memcpy(ipsec_sa_pd.ipsec_sa->new_key, read_key.key->key, read_key.key->key_size);
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
