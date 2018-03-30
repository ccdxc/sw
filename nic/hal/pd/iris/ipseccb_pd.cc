#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/ipseccb_pd.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif/lif_manager.hpp"
#include "nic/hal/pd/iris/p4plus_pd_api.h"
#include "nic/gen/esp_ipv4_tunnel_h2n_rxdma/include/esp_ipv4_tunnel_h2n_rxdma_p4plus_ingress.h"

namespace hal {
namespace pd {

hal_ret_t p4pd_get_ipsec_tx_stage0_prog_addr(uint64_t* offset);

void *
ipseccb_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_ipseccb_encrypt_t *)entry)->hw_id);
}

uint32_t
ipseccb_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(ipseccb_hw_id_t)) % ht_size;
}

bool
ipseccb_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(ipseccb_hw_id_t *)key1 == *(ipseccb_hw_id_t *)key2) {
        return true;
    }
    return false;
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_ipsec_rx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "ipsec_rx_stage0";

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

static hal_ret_t 
p4pd_add_or_del_ipsec_rx_stage0_entry(pd_ipseccb_encrypt_t* ipseccb_pd, bool del)
{
    common_p4plus_stage0_app_header_table_d     data = {0};
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    uint64_t                                    ipsec_cb_ring_base;
    uint64_t                                    ipsec_cb_ring_addr;
    uint32_t                                    ipsec_barco_ring_base;
    uint32_t                                    ipsec_barco_ring_addr;
    uint16_t                                    key_index;

    // hardware index for this entry
    ipseccb_hw_id_t hwid = ipseccb_pd->hw_id + 
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_RX_STAGE0);

    if(!del) {
        // get pc address
        if(p4pd_get_ipsec_rx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        data.action_id = pc_offset;
        HAL_TRACE_DEBUG("Received pc address {}", pc_offset);
        if (p4pd_get_ipsec_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
        }
        HAL_TRACE_DEBUG("Received TX pc address {}", pc_offset);
        HAL_TRACE_DEBUG("Received cb_id as {}", ipseccb_pd->ipseccb->cb_id);
        data.u.ipsec_encap_rxdma_initial_table_d.total = 2;
        data.u.ipsec_encap_rxdma_initial_table_d.iv = ipseccb_pd->ipseccb->iv;
        data.u.ipsec_encap_rxdma_initial_table_d.iv_salt = ipseccb_pd->ipseccb->iv_salt;
        data.u.ipsec_encap_rxdma_initial_table_d.iv_size = ipseccb_pd->ipseccb->iv_size;
        data.u.ipsec_encap_rxdma_initial_table_d.block_size = ipseccb_pd->ipseccb->block_size;
        data.u.ipsec_encap_rxdma_initial_table_d.icv_size = ipseccb_pd->ipseccb->icv_size;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd = ipseccb_pd->ipseccb->barco_enc_cmd;
        data.u.ipsec_encap_rxdma_initial_table_d.esn_lo = htonl(ipseccb_pd->ipseccb->esn_lo);
        data.u.ipsec_encap_rxdma_initial_table_d.spi = htonl(ipseccb_pd->ipseccb->spi);
        key_index = ipseccb_pd->ipseccb->key_index;
        data.u.ipsec_encap_rxdma_initial_table_d.key_index = htons(key_index);
        HAL_TRACE_DEBUG("key_index = {}\n", ipseccb_pd->ipseccb->key_index);
   
        ipsec_cb_ring_base = get_start_offset(CAPRI_HBM_REG_IPSECCB);
        ipsec_cb_ring_addr = (ipsec_cb_ring_base+(ipseccb_pd->ipseccb->cb_id * IPSEC_CB_RING_ENTRY_SIZE));
        HAL_TRACE_DEBUG("Ring base 0x{0:x} CB Ring Addr 0x{0:x}", ipsec_cb_ring_base, ipsec_cb_ring_addr);
        //ipsec_cb_ring_addr = htonll(ipsec_cb_ring_addr);

        data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr = htonl((uint32_t)(ipsec_cb_ring_addr & 0xFFFFFFFF));
        //data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr_hi = (uint8_t) ((ipsec_cb_ring_addr & 0xFF00000000) >> 32);
        data.u.ipsec_encap_rxdma_initial_table_d.cb_cindex = 0;
        data.u.ipsec_encap_rxdma_initial_table_d.cb_pindex = 0;
        //HAL_TRACE_DEBUG("CB Ring Addr 0x{0:x} hi 0x{0:x}", data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr, 
        //                data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr_hi);
         
        ipsec_barco_ring_base = get_start_offset(CAPRI_HBM_REG_IPSECCB_BARCO);
        ipsec_barco_ring_addr = (ipsec_barco_ring_base+(ipseccb_pd->ipseccb->cb_id * IPSEC_BARCO_RING_ENTRY_SIZE));
        HAL_TRACE_DEBUG("Ring base 0x{0:x} Barco Ring Addr 0x{0:x}", ipsec_barco_ring_base, ipsec_barco_ring_addr);
        ipsec_barco_ring_addr = htonl(ipsec_barco_ring_addr);

        data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr = ipsec_barco_ring_addr;
        //data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr_hi = (ipsec_barco_ring_addr & 0xFF00000000) >> 32;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_cindex = 0;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_pindex = 0;
        //HAL_TRACE_DEBUG("Barco Ring Addr 0x{0:x} hi 0x{0:x}", data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr,
        //                data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr_hi);

        if (ipseccb_pd->ipseccb->is_v6) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 1;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xFE;
        }

        if (ipseccb_pd->ipseccb->is_nat_t) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 2;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xFD;
        }

        if (ipseccb_pd->ipseccb->is_random) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 4;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xFB;
        }
        if (ipseccb_pd->ipseccb->extra_pad) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 8;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xF7;
        }
        HAL_TRACE_DEBUG("is_v6 {} is_nat_t {} is_random {} extra_pad {}", ipseccb_pd->ipseccb->is_v6, 
                        ipseccb_pd->ipseccb->is_nat_t, ipseccb_pd->ipseccb->is_random, ipseccb_pd->ipseccb->extra_pad);
    }
    HAL_TRACE_DEBUG("Programming ipsec stage0 at hw-id: 0x{0:x}", hwid); 
    if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create rx: stage0 entry for IPSECCB");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_ipsec_ip_header_entry(pd_ipseccb_encrypt_t* ipseccb_pd, bool del)
{
    pd_ipseccb_eth_ip4_hdr_t   eth_ip_hdr = {0};
    pd_ipseccb_eth_ip6_hdr_t   eth_ip6_hdr = {0};
    pd_ipseccb_udp_nat_t_hdr_t nat_t_udp_hdr = {0};
    hal_ret_t                  ret = HAL_RET_OK;

    // hardware index for this entry
    ipseccb_hw_id_t hwid = ipseccb_pd->hw_id + 
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_IP_HDR);

    if (!del) {
        if (ipseccb_pd->ipseccb->is_v6 == 0) {
            memcpy(eth_ip_hdr.smac, ipseccb_pd->ipseccb->smac, ETH_ADDR_LEN);
            memcpy(eth_ip_hdr.dmac, ipseccb_pd->ipseccb->dmac, ETH_ADDR_LEN);
            eth_ip_hdr.ethertype = htons(0x800);
            eth_ip_hdr.version_ihl = 0x45;
            eth_ip_hdr.tos = 0;
            //p4 will update/correct this part - fixed for now.
            eth_ip_hdr.tot_len = htons(64); 
            eth_ip_hdr.id = 0;
            eth_ip_hdr.frag_off = 0;
            eth_ip_hdr.ttl = 255;
            if (ipseccb_pd->ipseccb->is_nat_t == 1) {
                eth_ip_hdr.protocol = 17; // UDP - will hash define it.
            } else {
                eth_ip_hdr.protocol = 50; // ESP - will hash define it.
            }
            eth_ip_hdr.check = 0; // P4 to fill the right checksum
            eth_ip_hdr.saddr = htonl(ipseccb_pd->ipseccb->tunnel_sip4);
            eth_ip_hdr.daddr = htonl(ipseccb_pd->ipseccb->tunnel_dip4);
        } else {
            memcpy(eth_ip6_hdr.smac, ipseccb_pd->ipseccb->smac, ETH_ADDR_LEN);
            memcpy(eth_ip6_hdr.dmac, ipseccb_pd->ipseccb->dmac, ETH_ADDR_LEN);
            eth_ip6_hdr.ethertype = htons(0x86dd);
            eth_ip6_hdr.ver_tc_flowlabel = htonl(0x60000000);
            eth_ip6_hdr.payload_length = 128;
            eth_ip6_hdr.next_hdr = 50;
            eth_ip6_hdr.hop_limit = 255;
            memcpy(eth_ip6_hdr.src, ipseccb_pd->ipseccb->sip6.addr.v6_addr.addr8, IP6_ADDR8_LEN);
            memcpy(eth_ip6_hdr.dst, ipseccb_pd->ipseccb->dip6.addr.v6_addr.addr8, IP6_ADDR8_LEN);
            HAL_TRACE_DEBUG("Adding IPV6 header"); 
        }
    }
    HAL_TRACE_DEBUG("Programming stage0 at hw-id: 0x{0:x}", hwid); 
    if (ipseccb_pd->ipseccb->is_v6 == 0) {
        if(!p4plus_hbm_write(hwid,  (uint8_t *)&eth_ip_hdr, sizeof(eth_ip_hdr),
                    P4PLUS_CACHE_INVALIDATE_BOTH)){
            HAL_TRACE_ERR("Failed to create ip_hdr entry for IPSECCB");
            ret = HAL_RET_HW_FAIL;
        }
        if (ipseccb_pd->ipseccb->is_nat_t == 1) {
            nat_t_udp_hdr.sport = htons(UDP_PORT_NAT_T);
            nat_t_udp_hdr.dport = htons(UDP_PORT_NAT_T);
            if(!p4plus_hbm_write(hwid+34,  (uint8_t *)&nat_t_udp_hdr, sizeof(nat_t_udp_hdr),
                        P4PLUS_CACHE_INVALIDATE_BOTH)){
                HAL_TRACE_ERR("Failed to create nat_t_hdr entry for IPSECCB");
                ret = HAL_RET_HW_FAIL;
            }
        }    
    } else {
        if(!p4plus_hbm_write(hwid,  (uint8_t *)&eth_ip6_hdr, sizeof(eth_ip6_hdr),
                    P4PLUS_CACHE_INVALIDATE_BOTH)){
            HAL_TRACE_ERR("Failed to create ipv6_hdr entry for IPSECCB");
            ret = HAL_RET_HW_FAIL;
        }
        if (ipseccb_pd->ipseccb->is_nat_t == 1) {
            nat_t_udp_hdr.sport = htons(UDP_PORT_NAT_T);
            nat_t_udp_hdr.dport = htons(UDP_PORT_NAT_T);
            if(!p4plus_hbm_write(hwid+54,  (uint8_t *)&nat_t_udp_hdr, sizeof(nat_t_udp_hdr),
                        P4PLUS_CACHE_INVALIDATE_BOTH)){
                HAL_TRACE_ERR("Failed to create nat_t_hdr entry for IPSECCB");
                ret = HAL_RET_HW_FAIL;
            }
        }    
    }
    return ret;
}

hal_ret_t 
p4pd_add_or_del_ipseccb_rxdma_entry(pd_ipseccb_encrypt_t* ipseccb_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_ipsec_rx_stage0_entry(ipseccb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_ipsec_ip_header_entry(ipseccb_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

hal_ret_t 
p4pd_get_ipsec_rx_stage0_entry(pd_ipseccb_encrypt_t* ipseccb_pd)
{
    common_p4plus_stage0_app_header_table_d data = {0};
    uint64_t                                    ipsec_cb_ring_addr, ipsec_barco_ring_addr;
    
    // hardware index for this entry
    ipseccb_hw_id_t hwid = ipseccb_pd->hw_id + 
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_RX_STAGE0);

    if(!p4plus_hbm_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipseccb_pd->ipseccb->iv = ntohll(data.u.ipsec_encap_rxdma_initial_table_d.iv);
    ipseccb_pd->ipseccb->iv_salt = data.u.ipsec_encap_rxdma_initial_table_d.iv_salt;
    ipseccb_pd->ipseccb->iv_size = data.u.ipsec_encap_rxdma_initial_table_d.iv_size;
    ipseccb_pd->ipseccb->block_size = data.u.ipsec_encap_rxdma_initial_table_d.block_size;
    ipseccb_pd->ipseccb->icv_size = data.u.ipsec_encap_rxdma_initial_table_d.icv_size;
    ipseccb_pd->ipseccb->barco_enc_cmd = data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd;
    ipseccb_pd->ipseccb->esn_lo = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.esn_lo);
    ipseccb_pd->ipseccb->spi = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.spi);
    ipseccb_pd->ipseccb->key_index = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.key_index);

    //ipsec_cb_ring_addr_hi = data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr_hi;
    //ipsec_cb_ring_addr = (ipsec_cb_ring_addr_hi << 32) & 0xff00000000; 
    ipsec_cb_ring_addr = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr);

    //ipsec_barco_ring_addr_hi = data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr_hi;
    //ipsec_barco_ring_addr = (ipsec_barco_ring_addr_hi << 32) & 0xff00000000;
    ipsec_barco_ring_addr  = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr);

    ipseccb_pd->ipseccb->pi = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.cb_pindex);
    ipseccb_pd->ipseccb->ci = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.cb_cindex);
    ipseccb_pd->ipseccb->is_v6 = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x1;
    ipseccb_pd->ipseccb->is_nat_t = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x2;
    ipseccb_pd->ipseccb->is_random = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x4;
    HAL_TRACE_DEBUG("CB Ring Addr {:#x} Pindex {} CIndex {}", ipsec_cb_ring_addr, ipseccb_pd->ipseccb->pi, ipseccb_pd->ipseccb->ci);
    HAL_TRACE_DEBUG("Barco Ring Addr {:#x} Pindex {} CIndex {}", ipsec_barco_ring_addr, 
                   data.u.ipsec_encap_rxdma_initial_table_d.barco_pindex, 
                   data.u.ipsec_encap_rxdma_initial_table_d.barco_cindex);
    HAL_TRACE_DEBUG("Flags : is_v6 : {} is_nat_t : {} is_random : {}", ipseccb_pd->ipseccb->is_v6, 
                    ipseccb_pd->ipseccb->is_nat_t, ipseccb_pd->ipseccb->is_random); 
    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_ipseccb_rxdma_entry(pd_ipseccb_encrypt_t* ipseccb_pd)
{
    hal_ret_t   ret = HAL_RET_OK;
    
    ret = p4pd_get_ipsec_rx_stage0_entry(ipseccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ipsec_rx entry");
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
p4pd_get_ipsec_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "ipsec_tx_stage0";

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
p4pd_add_or_del_ipseccb_txdma_entry(pd_ipseccb_encrypt_t* ipseccb_pd, bool del)
{
    return HAL_RET_OK;
}

hal_ret_t 
p4pd_get_ipseccb_txdma_entry(pd_ipseccb_encrypt_t* ipseccb_pd)
{
    /* TODO */
    return HAL_RET_OK;
}

/**************************/

ipseccb_hw_id_t
pd_ipseccb_get_base_hw_index(pd_ipseccb_encrypt_t* ipseccb_pd)
{
    HAL_ASSERT(NULL != ipseccb_pd);
    HAL_ASSERT(NULL != ipseccb_pd->ipseccb);
    
    // Get the base address of IPSEC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset. 
    uint64_t offset = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_IPSEC_ESP, 0, 0);
    HAL_TRACE_DEBUG("received offset 0x{0:x}", offset);
    return offset + \
        (ipseccb_pd->ipseccb->cb_id * P4PD_HBM_IPSEC_CB_ENTRY_SIZE);
}

hal_ret_t
p4pd_add_or_del_ipseccb_entry(pd_ipseccb_encrypt_t* ipseccb_pd, bool del) 
{
    hal_ret_t                   ret = HAL_RET_OK;
 
    ret = p4pd_add_or_del_ipseccb_rxdma_entry(ipseccb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }
   
    ret = p4pd_add_or_del_ipseccb_txdma_entry(ipseccb_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;    
    }

err:
    return ret;
}

static
hal_ret_t
p4pd_get_ipseccb_entry(pd_ipseccb_encrypt_t* ipseccb_pd) 
{
    hal_ret_t                   ret = HAL_RET_OK;
    
    ret = p4pd_get_ipseccb_rxdma_entry(ipseccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get rxdma entry for ipseccb");
        goto err;    
    }
   
    ret = p4pd_get_ipseccb_txdma_entry(ipseccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get txdma entry for ipseccb");
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
pd_ipseccb_create (pd_ipseccb_create_args_t *args)
{
    hal_ret_t               ret;
    pd_ipseccb_encrypt_s              *ipseccb_pd;

    HAL_TRACE_DEBUG("Creating pd state for IPSEC CB.");

    // allocate PD ipseccb state
    ipseccb_pd = ipseccb_pd_alloc_init();
    if (ipseccb_pd == NULL) {
        return HAL_RET_OOM;
    }
    ipseccb_pd->ipseccb = args->ipseccb;
    // get hw-id for this IPSECCB
    ipseccb_pd->hw_id = pd_ipseccb_get_base_hw_index(ipseccb_pd);
    
    // program ipseccb
    ret = p4pd_add_or_del_ipseccb_entry(ipseccb_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    // add to db
    ret = add_ipseccb_pd_to_db(ipseccb_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->ipseccb->pd = ipseccb_pd;

    return HAL_RET_OK;

cleanup:

    if (ipseccb_pd) {
        ipseccb_pd_free(ipseccb_pd);
    }
    return ret;
}

hal_ret_t
pd_ipseccb_update (pd_ipseccb_update_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    ipseccb_t*                ipseccb = args->ipseccb;
    pd_ipseccb_encrypt_t*             ipseccb_pd = (pd_ipseccb_encrypt_t*)ipseccb->pd;

    HAL_TRACE_DEBUG("IPSECCB pd update");
    
    // program ipseccb
    ret = p4pd_add_or_del_ipseccb_entry(ipseccb_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update ipseccb");
    }
    return ret;
}

hal_ret_t
pd_ipseccb_delete (pd_ipseccb_delete_args_t *args)
{
    hal_ret_t               ret;
    
    if(!args) {
       return HAL_RET_INVALID_ARG; 
    }

    ipseccb_t*                ipseccb = args->ipseccb;
    pd_ipseccb_encrypt_t*             ipseccb_pd = (pd_ipseccb_encrypt_t*)ipseccb->pd;

    HAL_TRACE_DEBUG("IPSECCB pd delete");
    
    // program ipseccb
    ret = p4pd_add_or_del_ipseccb_entry(ipseccb_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ipseccb entry"); 
    }
    
    del_ipseccb_pd_from_db(ipseccb_pd);

    ipseccb_pd_free(ipseccb_pd);

    return ret;
}

hal_ret_t
pd_ipseccb_get (pd_ipseccb_get_args_t *args)
{
    hal_ret_t               ret;
    pd_ipseccb_encrypt_t              ipseccb_pd;

    HAL_TRACE_DEBUG("IPSECCB pd get for id: {}", args->ipseccb->cb_id);

    // allocate PD ipseccb state
    ipseccb_pd_init(&ipseccb_pd);
    ipseccb_pd.ipseccb = args->ipseccb;
    
    // get hw-id for this IPSECCB
    ipseccb_pd.hw_id = pd_ipseccb_get_base_hw_index(&ipseccb_pd);
    HAL_TRACE_DEBUG("Received hw-id 0x{0:x}", ipseccb_pd.hw_id);

    // get hw ipseccb entry
    ret = p4pd_get_ipseccb_entry(&ipseccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{0:x}", ipseccb_pd.ipseccb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
