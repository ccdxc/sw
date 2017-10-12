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
    phvwri p.app_header_table0_valid, 0
    phvwri p.app_header_table1_valid, 0
    phvwri p.app_header_table2_valid, 0
    phvwri p.app_header_table3_valid, 0

    phvwri p.p4_txdma_intr_dma_cmd_ptr, H2N_TXDMA2_DMA_COMMANDS_OFFSET

    // intrinsic
    phvwri p.intrinsic_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_start_addr, 0
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_end_addr, 16

    // app-header
    phvwri p.ipsec_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.ipsec_app_hdr_dma_cmd_phv_start_addr, IPSEC_TXDMA2_APP_HEADER_START 
    phvwri p.ipsec_app_hdr_dma_cmd_phv_end_addr,  IPSEC_TXDMA2_APP_HEADER_END 

    // Ethernet Header 
    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
#if 0
    addi r7, r0, ESP_FIXED_HDR_SIZE 
    add r7, r7, k.txdma2_global_iv_size
    add r1, r7, k.t0_s2s_in_page_addr
#endif
    add r1, r0, k.ipsec_to_stage3_ipsec_cb_addr
    addi r1, r1, IPSEC_IP_HDR_OFFSET
    phvwr  p.eth_hdr_dma_cmd_addr, r1
    //phvwr  p.eth_hdr_dma_cmd_size, k.t0_s2s_headroom_offset
    phvwri  p.eth_hdr_dma_cmd_size, 14 

    // Outer-IP  
    phvwri p.ip_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    add r1, r0, k.ipsec_to_stage3_ipsec_cb_addr
    addi r1, r1, IPSEC_IP_HDR_OFFSET
    addi r1, r1, 14
    phvwr  p.ip_hdr_dma_cmd_addr, r1 
    phvwri  p.ip_hdr_dma_cmd_size, IPV4_HDR_SIZE 

    //ESP Header with IV 
    phvwri p.esp_iv_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    //add r3, r0, k.txdma2_global_iv_size 
    //addi r3, r3, IPSEC_SALT_HEADROOM
    add r1, r0, k.t0_s2s_in_page_addr
    phvwr p.esp_iv_hdr_dma_cmd_addr, r1
    add r2, r0, k.txdma2_global_iv_size
    addi r2, r2, ESP_FIXED_HDR_SIZE 
    phvwr p.esp_iv_hdr_dma_cmd_size, r2

    // Payload
    phvwri p.enc_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    add r4, r0, k.t0_s2s_out_page_addr
    addi r4, r4, ESP_FIXED_HDR_SIZE 
    phvwr p.enc_pay_load_dma_cmd_addr, r4 
    add r3, r0, k.txdma2_global_payload_size
    add r3, r3, k.txdma2_global_pad_size
    phvwr p.enc_pay_load_dma_cmd_size, r3

    // padsize and l4 protocol
    phvwri p.tail_2_bytes_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.tail_2_bytes_dma_cmd_phv_start_addr, IPSEC_TAIL_2_BYTES_PHV_START
    phvwri p.tail_2_bytes_dma_cmd_phv_end_addr, IPSEC_TAIL_2_BYTES_PHV_END

    //ICV
    phvwri p.icv_header_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.icv_header_dma_cmd_size, k.txdma2_global_icv_size
    add r1, r0, k.t0_s2s_out_page_addr
    addi r1, r1, ESP_FIXED_HDR_SIZE 
    add r1, r1, k.txdma2_global_pad_size
    add r1, r1, k.t0_s2s_tailroom_offset
    addi r1, r1, 2
    phvwr p.icv_header_dma_cmd_addr, r1
    phvwri p.icv_header_dma_pkt_eop, 1
    phvwri p.icv_header_dma_cmd_eop, 1
    nop.e
    nop 
