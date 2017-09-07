#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_tx_table_s4_t0_cfg_action_d d;
struct phv_ p;

%%
        .param IPSEC_CB_BASE
        .param IPSEC_IP_HDR_BASE
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet:
    // intrinsic
    phvwri p.intrinsic_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_start_addr, 0
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_end_addr, 32
    // Ethernet Header 
    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr  p.eth_hdr_dma_cmd_addr, k.t0_s2s_in_page_addr
    phvwr  p.eth_hdr_dma_cmd_size, k.t0_s2s_headroom_offset
    // Outer-IP  
    phvwri p.ip_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    add r1, r0, k.txdma2_global_ipsec_cb_index
    sll r1, r1, IPSEC_CB_SHIFT_SIZE 
    addi r1, r1, IPSEC_CB_BASE
    addi r1, r1, IPSEC_IP_HDR_OFFSET
    phvwr  p.ip_hdr_dma_cmd_addr, r1 
    phvwri  p.ip_hdr_dma_cmd_size, IPV4_HDR_SIZE 
    //ESP Header with IV 
    phvwri p.esp_iv_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    add r1, r0, k.txdma2_global_ipsec_cb_index
    sll r1, r1, IPSEC_CB_SHIFT_SIZE 
    addi r1, r1, IPSEC_CB_BASE
    addi r1, r1, ESP_BASE_OFFSET
    phvwr p.esp_iv_hdr_dma_cmd_addr, r1
    add r2, r0, k.txdma2_global_iv_size
    addi r2, r2, 8
    phvwr p.esp_iv_hdr_dma_cmd_size, r2
    // Payload
    phvwri p.enc_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.enc_pay_load_dma_cmd_addr, k.t0_s2s_out_page_addr
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
    add r1, r1, k.t0_s2s_tailroom_offset
    addi r1, r1, 2
    phvwr p.icv_header_dma_cmd_addr, r1
    nop.e 
