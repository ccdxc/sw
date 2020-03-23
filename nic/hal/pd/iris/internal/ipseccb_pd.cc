#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/ipseccb_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/vrf_api.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "gen/p4gen/esp_ipv4_tunnel_h2n_rxdma/include/esp_ipv4_tunnel_h2n_rxdma_p4plus_ingress.h"

namespace hal {
namespace pd {

hal_ret_t p4pd_get_ipsec_tx_stage0_prog_addr(uint64_t* offset);

void *
ipseccb_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_ipseccb_encrypt_t *)entry)->hw_id);
}

uint32_t
ipseccb_pd_hw_key_size ()
{
    return sizeof(ipseccb_hw_id_t);
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_ipsec_rx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "rxdma_stage0.bin";
    char labelname[]= "ipsec_rx_stage0";

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
p4pd_add_or_del_ipsec_rx_stage0_entry(pd_ipseccb_encrypt_t* ipseccb_pd, bool del)
{
    common_p4plus_stage0_app_header_table_d     data = {0};
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    uint64_t                                    ipsec_cb_ring_addr;
    uint64_t                                    ipsec_barco_ring_addr;
    uint16_t                                    key_index;

    // hardware index for this entry
    ipseccb_hw_id_t hwid = ipseccb_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_RX_STAGE0);

    HAL_TRACE_DEBUG("Create/Delete Encrypt CB id {}", ipseccb_pd->ipseccb->cb_id);
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
        data.u.ipsec_encap_rxdma_initial_table_d.cosA = 1;
        data.u.ipsec_encap_rxdma_initial_table_d.cosB = 1;
        data.u.ipsec_encap_rxdma_initial_table_d.iv = ipseccb_pd->ipseccb->iv;
        data.u.ipsec_encap_rxdma_initial_table_d.iv_salt = ipseccb_pd->ipseccb->iv_salt;
        data.u.ipsec_encap_rxdma_initial_table_d.iv_size = ipseccb_pd->ipseccb->iv_size;
        //data.u.ipsec_encap_rxdma_initial_table_d.block_size = ipseccb_pd->ipseccb->block_size;
        data.u.ipsec_encap_rxdma_initial_table_d.icv_size = ipseccb_pd->ipseccb->icv_size;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd = ipseccb_pd->ipseccb->barco_enc_cmd;
        data.u.ipsec_encap_rxdma_initial_table_d.esn_lo = htonl(ipseccb_pd->ipseccb->esn_lo);
        data.u.ipsec_encap_rxdma_initial_table_d.spi = htonl(ipseccb_pd->ipseccb->spi);
        data.u.ipsec_encap_rxdma_initial_table_d.ipsec_cb_index = htons((ipseccb_pd->ipseccb->cb_id) & 0xffff);
        key_index = ipseccb_pd->ipseccb->key_index;
        data.u.ipsec_encap_rxdma_initial_table_d.key_index = htons(key_index);
        HAL_TRACE_DEBUG("key_index = {}\n", ipseccb_pd->ipseccb->key_index);

        ipsec_cb_ring_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_IPSECCB) + ((ipseccb_pd->ipseccb->cb_id) * DEFAULT_WRING_SLOT_SIZE * IPSEC_PER_CB_RING_SIZE);
        HAL_TRACE_DEBUG("CB Ring Addr {:#x}", ipsec_cb_ring_addr);
#if 0
        ipsec_cb_ring_base = asicpd_get_mem_addr(CAPRI_HBM_REG_IPSECCB);
        ipsec_cb_ring_addr = (ipsec_cb_ring_base+(ipseccb_pd->ipseccb->cb_id * IPSEC_CB_RING_ENTRY_SIZE));
        //ipsec_cb_ring_addr = htonll(ipsec_cb_ring_addr);
#endif
        data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr = htonl((uint32_t)(ipsec_cb_ring_addr & 0xFFFFFFFF));
        data.u.ipsec_encap_rxdma_initial_table_d.cb_cindex = 0;
        data.u.ipsec_encap_rxdma_initial_table_d.cb_pindex = 0;

        ipsec_barco_ring_addr = asicpd_get_mem_addr(CAPRI_HBM_REG_IPSECCB_BARCO) + ((ipseccb_pd->ipseccb->cb_id) * IPSEC_PER_CB_BARCO_SLOT_ELEM_SIZE * IPSEC_PER_CB_BARCO_RING_SIZE);
        HAL_TRACE_DEBUG("Barco Ring Addr {:#x}", ipsec_barco_ring_addr);
#if 0

        ipsec_barco_ring_base = asicpd_get_mem_addr(CAPRI_HBM_REG_IPSECCB_BARCO);
        ipsec_barco_ring_addr = (ipsec_barco_ring_base+(ipseccb_pd->ipseccb->cb_id * IPSEC_BARCO_RING_ENTRY_SIZE));
        ipsec_barco_ring_addr = htonl(ipsec_barco_ring_addr);
#endif

        data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr = htonl((uint32_t)(ipsec_barco_ring_addr & 0xFFFFFFFF));
        data.u.ipsec_encap_rxdma_initial_table_d.barco_cindex = 0;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_pindex = 0;

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
        data.u.ipsec_encap_rxdma_initial_table_d.flags |= 16;
        HAL_TRACE_DEBUG("is_v6 {} is_nat_t {} is_random {} extra_pad {}", ipseccb_pd->ipseccb->is_v6,
                        ipseccb_pd->ipseccb->is_nat_t, ipseccb_pd->ipseccb->is_random, ipseccb_pd->ipseccb->extra_pad);
    }
    HAL_TRACE_DEBUG("Programming ipsec stage0 at hw-id: {:#x}", hwid);
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
    vrf_t  *vrf = vrf_get_infra_vrf();
    pd_vrf_t *pd_vrf = NULL;

    // hardware index for this entry
    ipseccb_hw_id_t hwid = ipseccb_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_IP_HDR);
    if (vrf) {
        pd_vrf = (pd_vrf_t*)(vrf->pd);
        HAL_TRACE_DEBUG("vrf_fromcpu_vlan_id : {}", pd_vrf->vrf_fromcpu_vlan_id);
    }
    if (!del) {
        if (ipseccb_pd->ipseccb->is_v6 == 0) {

            memcpy(eth_ip_hdr.smac, ipseccb_pd->ipseccb->smac, ETH_ADDR_LEN);
            memcpy(eth_ip_hdr.dmac, ipseccb_pd->ipseccb->dmac, ETH_ADDR_LEN);
            eth_ip_hdr.dot1q_ethertype = htons(0x8100);
            if (vrf) {
                pd_vrf_t *pd_vrf = (pd_vrf_t*)(vrf->pd);
                eth_ip_hdr.vlan = htons(pd_vrf->vrf_fromcpu_vlan_id);
                HAL_TRACE_DEBUG("Vrf VLAN {}", pd_vrf->vrf_fromcpu_vlan_id);
            }

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
            eth_ip6_hdr.dot1q_ethertype = htons(0x8100);
            if (vrf) {
                pd_vrf_t *pd_vrf = (pd_vrf_t*)(vrf->pd);
                eth_ip6_hdr.vlan = htons(pd_vrf->vrf_fromcpu_vlan_id);
                HAL_TRACE_DEBUG("Vrf VLAN {}", pd_vrf->vrf_fromcpu_vlan_id);
            }

            //eth_ip6_hdr.vlan = htons(1);
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
    HAL_TRACE_DEBUG("Programming stage0 at hw-id: {:#x}", hwid);
    if (ipseccb_pd->ipseccb->is_v6 == 0) {
        if(!p4plus_hbm_write(hwid,  (uint8_t *)&eth_ip_hdr, sizeof(eth_ip_hdr),
                    P4PLUS_CACHE_INVALIDATE_BOTH)){
            HAL_TRACE_ERR("Failed to create ip_hdr entry for IPSECCB");
            ret = HAL_RET_HW_FAIL;
        }
        if (ipseccb_pd->ipseccb->is_nat_t == 1) {
            nat_t_udp_hdr.sport = htons(UDP_PORT_NAT_T);
            nat_t_udp_hdr.dport = htons(UDP_PORT_NAT_T);
            if(!p4plus_hbm_write(hwid+38,  (uint8_t *)&nat_t_udp_hdr, sizeof(nat_t_udp_hdr),
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
            if(!p4plus_hbm_write(hwid+58,  (uint8_t *)&nat_t_udp_hdr, sizeof(nat_t_udp_hdr),
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

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipseccb_pd->ipseccb->iv = ntohll(data.u.ipsec_encap_rxdma_initial_table_d.iv);
    ipseccb_pd->ipseccb->iv_salt = data.u.ipsec_encap_rxdma_initial_table_d.iv_salt;
    ipseccb_pd->ipseccb->iv_size = data.u.ipsec_encap_rxdma_initial_table_d.iv_size;
    //ipseccb_pd->ipseccb->block_size = data.u.ipsec_encap_rxdma_initial_table_d.block_size;
    ipseccb_pd->ipseccb->icv_size = data.u.ipsec_encap_rxdma_initial_table_d.icv_size;
    ipseccb_pd->ipseccb->barco_enc_cmd = data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd;
    ipseccb_pd->ipseccb->esn_lo = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.esn_lo);
    ipseccb_pd->ipseccb->spi = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.spi);
    ipseccb_pd->ipseccb->key_index = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.key_index);

    ipsec_cb_ring_addr = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr);

    ipsec_barco_ring_addr  = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr);

    ipseccb_pd->ipseccb->pi = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.cb_pindex);
    ipseccb_pd->ipseccb->ci = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.cb_cindex);
    ipseccb_pd->ipseccb->is_v6 = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x1;
    ipseccb_pd->ipseccb->is_nat_t = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x2;
    ipseccb_pd->ipseccb->is_random = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x4;
    HAL_TRACE_DEBUG("CB Ring Addr {:#x} Pindex {} CIndex {}", ipsec_cb_ring_addr,
                    data.u.ipsec_encap_rxdma_initial_table_d.cb_pindex,
                    data.u.ipsec_encap_rxdma_initial_table_d.cb_cindex);
    HAL_TRACE_DEBUG("Barco Ring Addr {:#x} Pindex {} CIndex {}", ipsec_barco_ring_addr,
                   data.u.ipsec_encap_rxdma_initial_table_d.barco_pindex,
                   data.u.ipsec_encap_rxdma_initial_table_d.barco_cindex);
    HAL_TRACE_DEBUG("Flags : is_v6 : {} is_nat_t : {} is_random : {}", ipseccb_pd->ipseccb->is_v6,
                    ipseccb_pd->ipseccb->is_nat_t, ipseccb_pd->ipseccb->is_random);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_ipsec_cb_stats(pd_ipseccb_encrypt_t* ipseccb_pd)
{
    rx_table_s5_t1_ipsec_rxdma_stats_update_d stats_data;
    // hardware index for this entry
    ipseccb_hw_id_t hwid = ipseccb_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_STATS);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&stats_data, sizeof(stats_data))){
        HAL_TRACE_ERR("Failed to get Stats: entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipseccb_pd->ipseccb->h2n_rx_pkts = ntohll(stats_data.h2n_rx_pkts);
    ipseccb_pd->ipseccb->h2n_rx_bytes = ntohll(stats_data.h2n_rx_bytes);
    ipseccb_pd->ipseccb->h2n_rx_drops = ntohll(stats_data.h2n_rx_drops);
    ipseccb_pd->ipseccb->h2n_tx_pkts = ntohll(stats_data.h2n_tx_pkts);
    ipseccb_pd->ipseccb->h2n_tx_bytes = ntohll(stats_data.h2n_tx_bytes);
    ipseccb_pd->ipseccb->h2n_tx_drops = ntohll(stats_data.h2n_tx_drops);

    HAL_TRACE_DEBUG("Stats: h2n: rx_pkts {} rx_bytes {} rx_drops {} tx_pkts {} tx_bytes {} tx_drops {}",
        ipseccb_pd->ipseccb->h2n_rx_pkts, ipseccb_pd->ipseccb->h2n_rx_bytes,
        ipseccb_pd->ipseccb->h2n_rx_drops, ipseccb_pd->ipseccb->h2n_tx_pkts,
        ipseccb_pd->ipseccb->h2n_tx_bytes, ipseccb_pd->ipseccb->h2n_tx_drops);
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
    ret = p4pd_get_ipsec_cb_stats(ipseccb_pd);
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
p4pd_get_ipsec_tx_stage0_prog_addr(uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    char labelname[]= "ipsec_tx_stage0";

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
    SDK_ASSERT(NULL != ipseccb_pd);
    SDK_ASSERT(NULL != ipseccb_pd->ipseccb);

    // Get the base address of IPSEC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    uint64_t offset = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_IPSEC_ESP, 0, 0);
    HAL_TRACE_DEBUG("received offset {:#x}", offset);
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
pd_ipseccb_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipseccb_create_args_t *args = pd_func_args->pd_ipseccb_create;
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
pd_ipseccb_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipseccb_update_args_t *args = pd_func_args->pd_ipseccb_update;

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
pd_ipseccb_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipseccb_delete_args_t *args = pd_func_args->pd_ipseccb_delete;

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
pd_ipseccb_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipseccb_get_args_t *args = pd_func_args->pd_ipseccb_get;
    pd_ipseccb_encrypt_t              ipseccb_pd;

    HAL_TRACE_DEBUG("IPSECCB pd get for id: {}", args->ipseccb->cb_id);

    // allocate PD ipseccb state
    ipseccb_pd_init(&ipseccb_pd);
    ipseccb_pd.ipseccb = args->ipseccb;

    // get hw-id for this IPSECCB
    ipseccb_pd.hw_id = pd_ipseccb_get_base_hw_index(&ipseccb_pd);
    HAL_TRACE_DEBUG("Received hw-id {:#x}", ipseccb_pd.hw_id);

    // get hw ipseccb entry
    ret = p4pd_get_ipseccb_entry(&ipseccb_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: {:#x}", ipseccb_pd.ipseccb->cb_id);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
