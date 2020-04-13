#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_tx_table_s4_t0_cfg_action_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .param esp_ipv4_tunnel_n2h_txdma2_ipsec_free_resources

esp_v4_tunnel_n2h_txdma2_build_decap_packet:
    // Ethernet Hdr
    add r2, r0, k.ipsec_to_stage4_in_page 
    blti  r2, ASIC_HBM_BASE, esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma_in_page
    nop
    phvwr  p.eth_hdr_dma_cmd_addr, k.ipsec_to_stage4_in_page
    // take only MAC addresses, etype will come from next DMA command based on v4 or v6
    sub r1, k.ipsec_to_stage4_headroom, 2
    phvwr  p.eth_hdr_dma_cmd_size, r1 
    // Vlan Header
    phvwri p.{vrf_vlan_hdr_dma_cmd_phv_end_addr...vrf_vlan_hdr_dma_cmd_type}, ((IPSEC_TXDMA2_VRF_VLAN_HEADER_END << 17) | (IPSEC_TXDMA2_VRF_VLAN_HEADER_START << 7) | CAPRI_DMA_COMMAND_PHV_TO_PKT)
    // Decrypted payload 
    add r4, k.t0_s2s_out_page_addr, ESP_FIXED_HDR_SIZE
    blti  r4, ASIC_HBM_BASE, esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma_out_page
    nop
    phvwr p.dec_pay_load_dma_cmd_addr, r4 
    //payload-size includes pad - subtract pad_size now
    sub r3, k.txdma2_global_payload_size, k.txdma2_global_pad_size
    subi r3, r3, 2
    phvwr p.dec_pay_load_dma_cmd_size, r3
    phvwri p.dec_pay_load_dma_cmd_pkt_eop, 1
    addi r5, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    addi r5, r5, IPSEC_N2H_SEM_CINDEX_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_txdma2_ipsec_free_resources, r5, TABLE_SIZE_32_BITS)
    phvwri.e p.{app_header_table1_valid...app_header_table3_valid}, 0 
    nop


esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma_in_page:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_S4_IN_PAGE_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop

esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma_out_page:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_OUT_PAGE_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop

