#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/ipsec/ipsec_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/sdk/platform/capri/capri_barco_crypto.hpp"
#include "nic/sdk/platform/capri/capri_common.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "gen/p4gen/esp_ipv4_tunnel_h2n_rxdma/include/esp_ipv4_tunnel_h2n_rxdma_p4plus_ingress.h"

namespace hal {
namespace pd {

hal_ret_t p4pd_get_ipsec_sa_tx_stage0_prog_addr(uint64_t* offset);

void *
ipsec_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_ipsec_t *)entry)->hw_id);
}

uint32_t
ipsec_pd_hw_key_size ()
{
    return sizeof(ipsec_sa_hw_id_t);
}

/********************************************
 * RxDMA
 * ******************************************/

hal_ret_t
p4pd_get_ipsec_sa_rx_stage0_prog_addr(uint64_t* offset)
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
p4pd_add_or_del_ipsec_rx_stage0_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    common_p4plus_stage0_app_header_table_d     data = {0};
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    uint64_t                                    ipsec_cb_ring_addr;
    uint64_t                                    ipsec_barco_ring_addr;
    uint16_t                                    key_index;
    ipsec_sa_t*                                 ipsec_sa = ipsec_sa_pd->ipsec_sa;
    uint8_t                                     zeros[P4PD_HBM_IPSEC_CB_ENTRY_SIZE];

    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_QSTATE1);

    if(!del) {
        // get pc address
        if(p4pd_get_ipsec_sa_rx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        data.action_id = pc_offset;
        HAL_TRACE_DEBUG("Received pc address {}", pc_offset);
        if (p4pd_get_ipsec_sa_tx_stage0_prog_addr(&pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
        }
        HAL_TRACE_DEBUG("Received TX pc address {}", pc_offset);
        HAL_TRACE_DEBUG("Received sa_id as {}", ipsec_sa->sa_id);
        data.u.ipsec_encap_rxdma_initial_table_d.total = 2;
        data.u.ipsec_encap_rxdma_initial_table_d.iv = ipsec_sa->iv;
        data.u.ipsec_encap_rxdma_initial_table_d.iv_salt = ipsec_sa->iv_salt;
        data.u.ipsec_encap_rxdma_initial_table_d.iv_size = ipsec_sa->iv_size;
        //data.u.ipsec_encap_rxdma_initial_table_d.block_size = ipsec_sa->block_size;
        data.u.ipsec_encap_rxdma_initial_table_d.icv_size = ipsec_sa->icv_size;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd = ipsec_sa->barco_enc_cmd;
        data.u.ipsec_encap_rxdma_initial_table_d.esn_lo = htonl(ipsec_sa->esn_lo);
        data.u.ipsec_encap_rxdma_initial_table_d.spi = htonl(ipsec_sa->spi);
        data.u.ipsec_encap_rxdma_initial_table_d.ipsec_cb_index = htons(ipsec_sa->sa_id);

        HAL_TRACE_DEBUG("iv {:#x} salt {:#x} iv_size {} icv_size {} barco_cmd {:#x}  esn_lo {} spi {}",
            data.u.ipsec_encap_rxdma_initial_table_d.iv,
            data.u.ipsec_encap_rxdma_initial_table_d.iv_salt,
            data.u.ipsec_encap_rxdma_initial_table_d.iv_size,
            data.u.ipsec_encap_rxdma_initial_table_d.icv_size,
            data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd,
            data.u.ipsec_encap_rxdma_initial_table_d.esn_lo,
            data.u.ipsec_encap_rxdma_initial_table_d.spi);

        key_index = ipsec_sa->key_index;
        data.u.ipsec_encap_rxdma_initial_table_d.key_index = htons(key_index);
        HAL_TRACE_DEBUG("key_index = {}", ipsec_sa->key_index);

        ipsec_cb_ring_addr = asicpd_get_mem_addr(ASIC_HBM_REG_IPSECCB) + ((ipsec_sa_pd->ipsec_sa->sa_id) * DEFAULT_WRING_SLOT_SIZE * IPSEC_PER_CB_RING_SIZE);
        HAL_TRACE_DEBUG("CB Ring Addr {:#x}", ipsec_cb_ring_addr);

        data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr = htonl((uint32_t)(ipsec_cb_ring_addr & 0xFFFFFFFF));
        data.u.ipsec_encap_rxdma_initial_table_d.cb_cindex = 0;
        data.u.ipsec_encap_rxdma_initial_table_d.cb_pindex = 0;

        ipsec_barco_ring_addr = asicpd_get_mem_addr(ASIC_HBM_REG_IPSECCB_BARCO) + ((ipsec_sa_pd->ipsec_sa->sa_id) * IPSEC_PER_CB_BARCO_SLOT_ELEM_SIZE * IPSEC_PER_CB_BARCO_RING_SIZE);
        HAL_TRACE_DEBUG("Barco Ring Addr {:#x}", ipsec_barco_ring_addr);

        data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr = htonl((uint32_t) (ipsec_barco_ring_addr & 0xFFFFFFFF));
        data.u.ipsec_encap_rxdma_initial_table_d.barco_cindex = 0;
        data.u.ipsec_encap_rxdma_initial_table_d.barco_pindex = 0;

        if (ipsec_sa_pd->ipsec_sa->is_v6) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 1;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xFE;
        }

        if (ipsec_sa_pd->ipsec_sa->is_nat_t) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 2;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xFD;
        }

        if (ipsec_sa_pd->ipsec_sa->is_random) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 4;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xFB;
        }
        if (ipsec_sa_pd->ipsec_sa->extra_pad) {
            data.u.ipsec_encap_rxdma_initial_table_d.flags |= 8;
        } else {
            data.u.ipsec_encap_rxdma_initial_table_d.flags &= 0xF7;
        }
        data.u.ipsec_encap_rxdma_initial_table_d.flags |= 16;

        HAL_TRACE_DEBUG("flags {} is_v6 {} is_nat_t {} is_random {} extra_pad {}", data.u.ipsec_encap_rxdma_initial_table_d.flags,
                        ipsec_sa->is_v6, ipsec_sa->is_nat_t, ipsec_sa->is_random, ipsec_sa->extra_pad);
        HAL_TRACE_DEBUG("Programming ipsec stage0 at hw-id: {:#x}", hwid);
        if(!p4plus_hbm_write(hwid,  (uint8_t *)&data, sizeof(data),
                    P4PLUS_CACHE_INVALIDATE_BOTH)){
            HAL_TRACE_ERR("Failed to create rx: stage0 entry for IPSECCB");
            ret = HAL_RET_HW_FAIL;
        }
    } else {
        memset(zeros, 0, P4PD_HBM_IPSEC_CB_ENTRY_SIZE);
        if (!p4plus_hbm_write(hwid, (uint8_t*)&zeros, sizeof(zeros), P4PLUS_CACHE_INVALIDATE_BOTH)) {
            HAL_TRACE_ERR("Failed to create rx: stage0 entry for IPSECCB");
            ret = HAL_RET_HW_FAIL;
        }
    }
    return ret;
}

hal_ret_t
p4pd_add_or_del_ipsec_ip_header_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    pd_ipsec_eth_ip4_hdr_t   eth_ip_hdr = {0};
    pd_ipsec_eth_ip6_hdr_t   eth_ip6_hdr = {0};
    pd_ipsec_udp_nat_t_hdr_t nat_t_udp_hdr = {0};
    hal_ret_t                  ret = HAL_RET_OK;

    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_ETH_IP_HDR);

    if (!del) {
        if (ipsec_sa_pd->ipsec_sa->is_v6 == 0) {
            memcpy(eth_ip_hdr.smac, ipsec_sa_pd->ipsec_sa->smac, ETH_ADDR_LEN);
            memcpy(eth_ip_hdr.dmac, ipsec_sa_pd->ipsec_sa->dmac, ETH_ADDR_LEN);
            eth_ip_hdr.dot1q_ethertype = htons(0x8100);
            vrf_t *vrf = vrf_lookup_by_id(ipsec_sa_pd->ipsec_sa->vrf);
            if (vrf) {
                pd_vrf_t *pd_vrf = (pd_vrf_t*)(vrf->pd);
                ipsec_sa_pd->ipsec_sa->vrf_vlan = pd_vrf->vrf_fromcpu_vlan_id;
                HAL_TRACE_DEBUG("Vrf VLAN {}", ipsec_sa_pd->ipsec_sa->vrf_vlan);
            }

            eth_ip_hdr.vlan = htons(ipsec_sa_pd->ipsec_sa->vrf_vlan);
            HAL_TRACE_DEBUG("vrf vlan : {}", ipsec_sa_pd->ipsec_sa->vrf_vlan);
            eth_ip_hdr.ethertype = htons(0x800);
            eth_ip_hdr.version_ihl = 0x45;
            eth_ip_hdr.tos = 0;
            //p4 will update/correct this part - fixed for now.
            eth_ip_hdr.tot_len = htons(64);
            eth_ip_hdr.id = 0;
            eth_ip_hdr.frag_off = 0;
            eth_ip_hdr.ttl = 255;
            if (ipsec_sa_pd->ipsec_sa->is_nat_t == 1) {
                eth_ip_hdr.protocol = 17; // UDP - will hash define it.
            } else {
                eth_ip_hdr.protocol = 50; // ESP - will hash define it.
            }
            eth_ip_hdr.check = 0; // P4 to fill the right checksum
            eth_ip_hdr.saddr = htonl(ipsec_sa_pd->ipsec_sa->tunnel_sip4.addr.v4_addr);
            eth_ip_hdr.daddr = htonl(ipsec_sa_pd->ipsec_sa->tunnel_dip4.addr.v4_addr);
            HAL_TRACE_DEBUG("Tunnel SIP {:#x} Tunnel DIP {:#x}", eth_ip_hdr.saddr, eth_ip_hdr.daddr);
        } else {
            memcpy(eth_ip6_hdr.smac, ipsec_sa_pd->ipsec_sa->smac, ETH_ADDR_LEN);
            memcpy(eth_ip6_hdr.dmac, ipsec_sa_pd->ipsec_sa->dmac, ETH_ADDR_LEN);
            eth_ip6_hdr.ethertype = htons(0x86dd);
            eth_ip6_hdr.ver_tc_flowlabel = htonl(0x60000000);
            eth_ip6_hdr.payload_length = 128;
            eth_ip6_hdr.next_hdr = 50;
            eth_ip6_hdr.hop_limit = 255;
            //memcpy(eth_ip6_hdr.src, ipsec_sa_pd->ipsec_sa->sip6.addr.v6_addr.addr8, IP6_ADDR8_LEN);
            //memcpy(eth_ip6_hdr.dst, ipsec_sa_pd->ipsec_sa->dip6.addr.v6_addr.addr8, IP6_ADDR8_LEN);
            HAL_TRACE_DEBUG("Adding IPV6 header");
        }
    }
    HAL_TRACE_DEBUG("Programming stage0 at hw-id: {:#x}", hwid);
    if (ipsec_sa_pd->ipsec_sa->is_v6 == 0) {
        if(!p4plus_hbm_write(hwid,  (uint8_t *)&eth_ip_hdr, sizeof(eth_ip_hdr),
                    P4PLUS_CACHE_INVALIDATE_BOTH)){
            HAL_TRACE_ERR("Failed to create ip_hdr entry for IPSECCB");
            ret = HAL_RET_HW_FAIL;
        }
        if (ipsec_sa_pd->ipsec_sa->is_nat_t == 1) {
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
        if (ipsec_sa_pd->ipsec_sa->is_nat_t == 1) {
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
p4pd_add_or_del_ipseccb_encrypt_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_ipsec_rx_stage0_entry(ipsec_sa_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    ret = p4pd_add_or_del_ipsec_ip_header_entry(ipsec_sa_pd, del);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;
cleanup:
    /* TODO: CLEANUP */
    return ret;
}

hal_ret_t
p4pd_get_ipsec_rx_stage0_entry(pd_ipsec_t* ipsec_sa_pd)
{
    common_p4plus_stage0_app_header_table_d data = {0};
    uint64_t                                    ipsec_cb_ring_addr, ipsec_barco_ring_addr;

    ipsec_sa_t  *ipsec_sa = ipsec_sa_pd->ipsec_sa;

    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_QSTATE1);

    HAL_TRACE_DEBUG("Getting from hw-id {:#x}", hwid);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipsec_sa->iv = ntohll(data.u.ipsec_encap_rxdma_initial_table_d.iv);
    ipsec_sa->iv_salt = data.u.ipsec_encap_rxdma_initial_table_d.iv_salt;
    ipsec_sa->iv_size = data.u.ipsec_encap_rxdma_initial_table_d.iv_size;
    //ipsec_sa->block_size = data.u.ipsec_encap_rxdma_initial_table_d.block_size;
    ipsec_sa->icv_size = data.u.ipsec_encap_rxdma_initial_table_d.icv_size;
    ipsec_sa->barco_enc_cmd = data.u.ipsec_encap_rxdma_initial_table_d.barco_enc_cmd;
    ipsec_sa->esn_lo = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.esn_lo);
    ipsec_sa->spi = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.spi);
    ipsec_sa->key_index = ntohs(data.u.ipsec_encap_rxdma_initial_table_d.key_index);
    ipsec_cb_ring_addr = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.cb_ring_base_addr);

    ipsec_barco_ring_addr  = ntohl(data.u.ipsec_encap_rxdma_initial_table_d.barco_ring_base_addr);

    ipsec_sa->is_v6 = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x1;
    ipsec_sa->is_nat_t = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x2;
    ipsec_sa->is_random = data.u.ipsec_encap_rxdma_initial_table_d.flags & 0x4;
    HAL_TRACE_DEBUG("CB Ring Addr {:#x} Barco Ring Addr {:#x} Pindex {} CIndex {}",
                   ipsec_cb_ring_addr, ipsec_barco_ring_addr,
                   data.u.ipsec_encap_rxdma_initial_table_d.barco_pindex,
                   data.u.ipsec_encap_rxdma_initial_table_d.barco_cindex);
    HAL_TRACE_DEBUG("Flags : is_v6 : {} is_nat_t : {} is_random : {}", ipsec_sa->is_v6,
                    ipsec_sa->is_nat_t, ipsec_sa->is_random);
    return HAL_RET_OK;
}


hal_ret_t
p4pd_get_ipsec_rx_stage0_entry_part2(pd_ipsec_t* ipsec_sa_pd)
{
    ipsec_sa_t  *ipsec_sa = ipsec_sa_pd->ipsec_sa;
    pd_ipsec_qstate_addr_part2_t data = {0};

    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_ETH_IP_HDR);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to get rx: stage0 entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipsec_sa->tunnel_sip4.addr.v4_addr =  ntohl(data.u.eth_ip4_hdr.saddr);
    ipsec_sa->tunnel_dip4.addr.v4_addr =  ntohl(data.u.eth_ip4_hdr.daddr);
    HAL_TRACE_DEBUG("SIP {:#x} DIP {:#x}", ipsec_sa->tunnel_sip4.addr.v4_addr, ipsec_sa->tunnel_dip4.addr.v4_addr );

    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_ipsec_sa_encrypt_stats(pd_ipsec_t* ipsec_sa_pd)
{
    ipsec_sa_t  *ipsec_sa = ipsec_sa_pd->ipsec_sa;
    rx_table_s5_t1_ipsec_rxdma_stats_update_d stats_data;
    // hardware index for this entry
    ipsec_sa_hw_id_t hwid = ipsec_sa_pd->hw_id +
        (P4PD_IPSECCB_STAGE_ENTRY_OFFSET * P4PD_HWID_IPSEC_ENCRYPT_STATS);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&stats_data, sizeof(stats_data))){
        HAL_TRACE_ERR("Failed to get Stats: entry for IPSEC CB");
        return HAL_RET_HW_FAIL;
    }
    ipsec_sa->total_rx_pkts = ntohll(stats_data.h2n_rx_pkts);
    ipsec_sa->total_rx_bytes = ntohll(stats_data.h2n_rx_bytes);
    ipsec_sa->total_rx_drops = ntohll(stats_data.h2n_rx_drops);
    ipsec_sa->total_pkts = ntohll(stats_data.h2n_tx_pkts);
    ipsec_sa->total_bytes = ntohll(stats_data.h2n_tx_bytes);
    ipsec_sa->total_drops = ntohll(stats_data.h2n_tx_drops);

    HAL_TRACE_DEBUG("Stats: h2n: rx_pkts {} rx_bytes {} rx_drops {} tx_pkts {} tx_bytes {} tx_drops {}",
        ipsec_sa->total_rx_pkts, ipsec_sa->total_rx_bytes,
        ipsec_sa->total_rx_drops, ipsec_sa->total_pkts,
        ipsec_sa->total_bytes, ipsec_sa->total_drops);
    return HAL_RET_OK;
}

hal_ret_t
p4pd_get_ipsec_sa_rxdma_entry(pd_ipsec_t* ipsec_sa_pd)
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = p4pd_get_ipsec_rx_stage0_entry(ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ipsec_rx entry");
        goto cleanup;
    }
    ret = p4pd_get_ipsec_rx_stage0_entry_part2(ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ipsec_rx entry");
        goto cleanup;
    }
    ret = p4pd_get_ipsec_sa_encrypt_stats(ipsec_sa_pd);
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
p4pd_get_ipsec_sa_tx_stage0_prog_addr(uint64_t* offset)
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

/**************************/

ipsec_sa_hw_id_t
pd_ipsec_encrypt_get_base_hw_index(pd_ipsec_t* ipsec_sa_pd)
{
    SDK_ASSERT(NULL != ipsec_sa_pd);
    SDK_ASSERT(NULL != ipsec_sa_pd->ipsec_sa);

    // Get the base address of IPSEC CB from LIF Manager.
    // Set qtype and qid as 0 to get the start offset.
    // uint64_t base = lif_manager()->GetLIFQStateAddr(SERVICE_LIF_IPSEC_ESP, 0, 0);
    uint64_t base = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_IPSEC_ESP, 0, 0);
    uint64_t offset = (ipsec_sa_pd->ipsec_sa->sa_id * P4PD_HBM_IPSEC_CB_ENTRY_SIZE);
    HAL_TRACE_DEBUG("For PD SA ID {} received base {:#x} offset {:#x}", ipsec_sa_pd->ipsec_sa->sa_id, base, offset);
    return (base+offset);
}

hal_ret_t
p4pd_add_or_del_ipsec_entry(pd_ipsec_t* ipsec_sa_pd, bool del)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_add_or_del_ipseccb_encrypt_entry(ipsec_sa_pd, del);
    if(ret != HAL_RET_OK) {
        goto err;
    }

err:
    return ret;
}

static
hal_ret_t
p4pd_get_ipsec_sa_encrypt_entry(pd_ipsec_t* ipsec_sa_pd)
{
    hal_ret_t                   ret = HAL_RET_OK;

    ret = p4pd_get_ipsec_sa_rxdma_entry(ipsec_sa_pd);
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
pd_ipsec_encrypt_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t                  ret = HAL_RET_OK;
    pd_ipsec_encrypt_create_args_t *args = pd_func_args->pd_ipsec_encrypt_create;
    pd_func_args_t             pd_func_args1 = {0};
    pd_ipsec_s                 *ipsec_sa_pd;
    pd_crypto_alloc_key_args_t key_index_args;
    pd_crypto_write_key_args_t write_key_args;
    crypto_key_t               crypto_key;
    ipsec_sa_t                 *ipsec_sa;
    int32_t                   key_index;

    HAL_TRACE_DEBUG("Creating pd state for IPSEC CB.");

    // allocate PD ipseccb state
    ipsec_sa_pd = ipsec_pd_alloc_init();
    if (ipsec_sa_pd == NULL) {
        return HAL_RET_OOM;
    }
    ipsec_sa_pd->ipsec_sa = args->ipsec_sa;
    ipsec_sa = args->ipsec_sa;
    if (ipsec_sa == NULL) {
        HAL_TRACE_ERR("IPSec SA ptr NULL");
        goto cleanup;
    }
    // get hw-id for this IPSECCB
    ipsec_sa_pd->hw_id = pd_ipsec_encrypt_get_base_hw_index(ipsec_sa_pd);
    HAL_TRACE_DEBUG("HW ID {:#x}", ipsec_sa_pd->hw_id);

    key_index_args.key_idx = &key_index;

    pd_func_args1.pd_crypto_alloc_key = &key_index_args;
    ret = pd_crypto_alloc_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create key index");
        goto cleanup;
    }
    ipsec_sa_pd->ipsec_sa->key_index = *(key_index_args.key_idx);
    crypto_key.key_type = (types::CryptoKeyType)ipsec_sa->key_type;
    crypto_key.key_size = ipsec_sa->key_size;
    memcpy(crypto_key.key, ipsec_sa->key, ipsec_sa->key_size);
    write_key_args.key_idx = ipsec_sa->key_index;
    write_key_args.key = &crypto_key;
    pd_func_args1.pd_crypto_write_key = &write_key_args;
    ret = pd_crypto_write_key(&pd_func_args1);

    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write key at index {}", ipsec_sa->key_index);
        goto cleanup;
    }
    // program ipseccb
    ret = p4pd_add_or_del_ipsec_entry(ipsec_sa_pd, false);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    // add to db
    ret = add_ipsec_pd_to_db(ipsec_sa_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    args->ipsec_sa->pd = ipsec_sa_pd;

    return HAL_RET_OK;

cleanup:

    if (ipsec_sa_pd) {
        ipsec_pd_free(ipsec_sa_pd);
    }
    return ret;
}

hal_ret_t
pd_ipsec_encrypt_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipsec_encrypt_update_args_t *args = pd_func_args->pd_ipsec_encrypt_update;
    pd_func_args_t             pd_func_args1 = {0};
    pd_crypto_write_key_args_t write_key_args;
    crypto_key_t crypto_key;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    ipsec_sa_t*                ipsec_sa = args->ipsec_sa;
    pd_ipsec_t*       ipsec_sa_pd = (pd_ipsec_t*)ipsec_sa->pd;

    HAL_TRACE_DEBUG("IPSECCB pd update");

    crypto_key.key_type = (types::CryptoKeyType)ipsec_sa->key_type;
    crypto_key.key_size = ipsec_sa->key_size;
    memcpy(crypto_key.key, ipsec_sa->key, ipsec_sa->key_size);
    write_key_args.key_idx = ipsec_sa->key_index;
    write_key_args.key = &crypto_key;
    pd_func_args1.pd_crypto_write_key = &write_key_args;

    ret = pd_crypto_write_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to write key at index");
    }
    // program ipsec_sa
    ret = p4pd_add_or_del_ipsec_entry(ipsec_sa_pd, false);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update ipsec_sa");
    }
    return ret;
}

hal_ret_t
pd_ipsec_encrypt_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t                  ret;
    pd_ipsec_encrypt_delete_args_t *args = pd_func_args->pd_ipsec_encrypt_delete;
    pd_func_args_t             pd_func_args1 = {0};
    pd_crypto_free_key_args_t  free_key_args;

    if(!args) {
       return HAL_RET_INVALID_ARG;
    }

    ipsec_sa_t*                ipsec_sa = args->ipsec_sa;
    pd_ipsec_t*        ipsec_sa_pd = (pd_ipsec_t*)ipsec_sa->pd;

    if (ipsec_sa_pd == NULL) {
       return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("pd delete for id {}", ipsec_sa->sa_id);

    free_key_args.key_idx = ipsec_sa->key_index;
    pd_func_args1.pd_crypto_free_key = &free_key_args;
    ret = pd_crypto_free_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ipsec key at key_index {}", ipsec_sa->key_index);
    }
    // program ipsec_sa
    ret = p4pd_add_or_del_ipsec_entry(ipsec_sa_pd, true);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ipsec_sa entry");
    }

    del_ipsec_pd_from_db(ipsec_sa_pd);

    ipsec_pd_free(ipsec_sa_pd);

    return ret;
}

hal_ret_t
pd_ipsec_global_stats_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t                  ret;
    pd_ipsec_global_stats_get_args_t *args = pd_func_args->pd_ipsec_global_stats_get;
    ipsec_global_stats_cb_t stats;
    uint32_t counter;
    uint16_t barco_counter;
    uint64_t hwid =  asicpd_get_mem_addr(ASIC_HBM_REG_IPSEC_GLOBAL_DROP_STATS);

    if (args == NULL) {
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("IPSEC Global Stats Addr {:#x}", hwid);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&stats, sizeof(ipsec_global_stats_cb_t))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    
    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + CAPRI_IPSEC_ENC_NMDR_ALLOC_PI;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&counter, sizeof(counter))){
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Enc RNMDPR PI {}", counter); 
    stats.enc_rnmdpr_pi_counters = counter;

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + CAPRI_IPSEC_ENC_NMDR_ALLOC_CI;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&counter, sizeof(counter))){
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Enc RNMDPR CI {}", counter);
    stats.enc_rnmdpr_ci_counters = counter;

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + CAPRI_IPSEC_DEC_NMDR_ALLOC_PI;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&counter, sizeof(counter))){
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Dec RNMDPR PI {}", counter); 
    stats.dec_rnmdpr_pi_counters = counter;

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + CAPRI_IPSEC_DEC_NMDR_ALLOC_CI;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&counter, sizeof(counter))){
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Dec RNMDPR CI {}", counter);
    stats.dec_rnmdpr_ci_counters = counter;

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + BARCO_GCM0_PI_HBM_TABLE_OFFSET + 4;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&counter, sizeof(counter))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Barco Encrypt Full Errors {}", ntohl(counter));
    stats.gcm0_full_counters = ntohl(counter);

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + BARCO_GCM1_PI_HBM_TABLE_OFFSET + 4;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&counter, sizeof(counter))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Barco Decrypt Full Errors {}", htonl(counter));
    stats.gcm1_full_counters = ntohl(counter);
    
    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + BARCO_GCM0_PI_HBM_TABLE_OFFSET;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&barco_counter, sizeof(barco_counter))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    stats.enc_global_barco_pi = ntohs(barco_counter);

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + BARCO_GCM0_PI_HBM_TABLE_OFFSET + sizeof(barco_counter);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&barco_counter, sizeof(barco_counter))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    stats.enc_global_barco_ci = ntohs(barco_counter);

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + BARCO_GCM1_PI_HBM_TABLE_OFFSET;
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&barco_counter, sizeof(barco_counter))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    stats.dec_global_barco_pi = ntohs(barco_counter);

    hwid = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) + BARCO_GCM1_PI_HBM_TABLE_OFFSET + sizeof(barco_counter);
    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&barco_counter, sizeof(barco_counter))) {
        HAL_TRACE_ERR("Failed to read IPSec global stats memory");
        return HAL_RET_HW_FAIL;
    }
    stats.dec_global_barco_ci = ntohs(barco_counter);

    memcpy(args->stats_cb, &stats, sizeof(ipsec_global_stats_cb_t));
    
    ret = HAL_RET_OK;
    return ret;
}


hal_ret_t
pd_ipsec_encrypt_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_ipsec_encrypt_get_args_t *args = pd_func_args->pd_ipsec_encrypt_get;
    pd_func_args_t             pd_func_args1 = {0};
    pd_ipsec_t              ipsec_sa_pd;

    pd_crypto_read_key_args_t read_key;
    crypto_key_t              key;

    HAL_TRACE_DEBUG("IPSEC SA pd get for id: {}", args->ipsec_sa->sa_id);

    // allocate PD ipsec_sa state
    ipsec_pd_encrypt_init(&ipsec_sa_pd);
    ipsec_sa_pd.ipsec_sa = args->ipsec_sa;

    // get hw-id for this IPSECCB
    ipsec_sa_pd.hw_id = pd_ipsec_encrypt_get_base_hw_index(&ipsec_sa_pd);
    HAL_TRACE_DEBUG("Received hw-id {:#x}", ipsec_sa_pd.hw_id);

    // get hw ipsec_sa entry
    ret = p4pd_get_ipsec_sa_encrypt_entry(&ipsec_sa_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for id: 0x{:#x}", ipsec_sa_pd.ipsec_sa->sa_id);
        return ret;
    }
    HAL_TRACE_DEBUG("IPSEC SA pd get key_index for id: {}", args->ipsec_sa->key_index);
    read_key.key_idx = ipsec_sa_pd.ipsec_sa->key_index;
    memset(&key, 0, sizeof(crypto_key_t));
    read_key.key = &key;

    pd_func_args1.pd_crypto_read_key = &read_key;
    ret = pd_crypto_read_key(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Get request failed for key_index: {}", ipsec_sa_pd.ipsec_sa->key_index);
        //return ret;
        ret = HAL_RET_OK;
    }
    HAL_TRACE_DEBUG("IPSEC SA key_type, key_size : {}, {}", read_key.key->key_type, read_key.key->key_size);
    if ((read_key.key->key_size > 0) && (read_key.key->key_size <= 32)) {
        memcpy(ipsec_sa_pd.ipsec_sa->key, read_key.key->key, read_key.key->key_size);
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
