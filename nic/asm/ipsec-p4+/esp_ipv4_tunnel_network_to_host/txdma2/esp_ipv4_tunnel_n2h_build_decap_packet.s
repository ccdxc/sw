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
    phvwr  p.eth_hdr_dma_cmd_addr, k.ipsec_to_stage4_in_page
    // take only MAC addresses, etype will come from next DMA command based on v4 or v6
    sub r1, k.ipsec_to_stage4_headroom, 2
    phvwr  p.eth_hdr_dma_cmd_size, r1 
   
    // Vlan Header
    phvwri p.vrf_vlan_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.vrf_vlan_hdr_dma_cmd_phv_start_addr, IPSEC_TXDMA2_VRF_VLAN_HEADER_START
    phvwri p.vrf_vlan_hdr_dma_cmd_phv_end_addr, IPSEC_TXDMA2_VRF_VLAN_HEADER_END

    // Decrypted payload 
    add r4, k.t0_s2s_out_page_addr, ESP_FIXED_HDR_SIZE
    phvwr p.dec_pay_load_dma_cmd_addr, r4 
    //payload-size includes pad - subtract pad_size now
    sub r3, k.txdma2_global_payload_size, k.txdma2_global_pad_size
    subi r3, r3, 2
    phvwr p.dec_pay_load_dma_cmd_size, r3
    phvwri p.dec_pay_load_dma_cmd_eop, 1
    phvwri p.dec_pay_load_dma_pkt_eop, 1

    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0 
    nop.e 
    nop
