#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_tx_table_s4_t0_cfg_action_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma2_build_decap_packet:
    // Ethernet Hdr
    add r2, r0, k.ipsec_to_stage4_in_page 
    blti  r2, CAPRI_HBM_BASE, esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma
    phvwr  p.eth_hdr_dma_cmd_addr, k.ipsec_to_stage4_in_page
    // take only MAC addresses, etype will come from next DMA command based on v4 or v6
    sub r1, k.ipsec_to_stage4_headroom, 2
    phvwr  p.eth_hdr_dma_cmd_size, r1 
   
    // Vlan Header
    phvwri p.{vrf_vlan_hdr_dma_cmd_phv_end_addr...vrf_vlan_hdr_dma_cmd_type}, ((IPSEC_TXDMA2_VRF_VLAN_HEADER_END << 17) | (IPSEC_TXDMA2_VRF_VLAN_HEADER_START << 7) | CAPRI_DMA_COMMAND_PHV_TO_PKT)

    // Decrypted payload 
    add r4, k.t0_s2s_out_page_addr, ESP_FIXED_HDR_SIZE
    blti  r4, CAPRI_HBM_BASE, esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma
    phvwr p.dec_pay_load_dma_cmd_addr, r4 
    //payload-size includes pad - subtract pad_size now
    sub r3, k.txdma2_global_payload_size, k.txdma2_global_pad_size
    subi r3, r3, 2
    phvwr p.dec_pay_load_dma_cmd_size, r3
    phvwri p.{dec_pay_load_dma_pkt_eop...dec_pay_load_dma_cmd_eop}, 3
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0 
    nop


esp_v4_tunnel_n2h_txdma2_build_decap_packet_illegal_dma:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop


