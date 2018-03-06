#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_tx_table_s3_t0_cfg_action_d d;
struct phv_ p;

%%
        .param IPSEC_IP_HDR_BASE
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    //phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA2_DMA_COMMANDS_OFFSET
    // intrinsic
    //phvwri p.intrinsic_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    //phvwri p.intrinsic_app_hdr_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    //phvwri p.intrinsic_app_hdr_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)
    // app-header
    //phvwri p.ipsec_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    //phvwri p.ipsec_app_hdr_dma_cmd_phv_start_addr, IPSEC_TXDMA2_APP_HEADER_START 
    //phvwri p.ipsec_app_hdr_dma_cmd_phv_end_addr,  IPSEC_TXDMA2_APP_HEADER_END 
    // Ethernet Header 
    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    add r1, k.ipsec_to_stage3_ipsec_cb_addr, IPSEC_IP_HDR_OFFSET
    phvwr  p.eth_hdr_dma_cmd_addr, r1
    phvwri  p.eth_hdr_dma_cmd_size, ETH_FIXED_HDR_SIZE 
    // Outer-IP  
    phvwri p.ip_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    addi r1, r1, 14
    phvwr  p.ip_hdr_dma_cmd_addr, r1 
    seq c1, k.ipsec_to_stage3_is_v6, 1
    cmov r6, c1, IPV6_HDR_SIZE, IPV4_HDR_SIZE 
    seq c3, k.ipsec_to_stage3_is_nat_t, 1
    addi.c3 r6, r6, UDP_FIXED_HDR_SIZE 
    phvwr  p.ip_hdr_dma_cmd_size, r6 
    // P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN | P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | P4PLUS_TO_P4_FLAGS_COMP_OUTER_CSUM 
    phvwri.c3 p.p4plus2p4_hdr_flags, 0x2a 
    //ESP Header with IV 
    phvwri p.esp_iv_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.esp_iv_hdr_dma_cmd_addr, k.t0_s2s_in_page_addr 
    add r2, k.txdma2_global_iv_size, ESP_FIXED_HDR_SIZE 
    phvwr p.esp_iv_hdr_dma_cmd_size, r2
    // Payload
    phvwri p.enc_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    add r4, k.t0_s2s_out_page_addr, ESP_FIXED_HDR_SIZE 
    phvwr p.enc_pay_load_dma_cmd_addr, r4 
    add r3, k.txdma2_global_payload_size, k.txdma2_global_pad_size
    addi r3, r3, 2
    phvwr p.enc_pay_load_dma_cmd_size, r3
    //ICV
    phvwri p.icv_header_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.icv_header_dma_cmd_size, k.txdma2_global_icv_size
    add r1, k.t0_s2s_out_page_addr, ESP_FIXED_HDR_SIZE+2 
    add r1, r1, k.txdma2_global_pad_size
    add r1, r1, k.t0_s2s_tailroom_offset
    phvwr p.icv_header_dma_cmd_addr, r1
    phvwri p.icv_header_dma_pkt_eop, 1
    phvwri p.icv_header_dma_cmd_eop, 1
    nop.e
    nop 
