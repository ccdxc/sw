#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_tx_table_s3_t0_cfg_action_d d;
struct phv_ p;

%%
        .align
esp_v4_tunnel_n2h_txdma2_build_decap_packet:
    phvwri p.intrinsic_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_start_addr, 0
    phvwri p.intrinsic_app_hdr_dma_cmd_phv_end_addr, 16
 
    phvwri p.ipsec_app_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwri p.ipsec_app_hdr_dma_cmd_phv_start_addr, IPSEC_TXDMA2_APP_HEADER_START
    phvwri p.ipsec_app_hdr_dma_cmd_phv_end_addr, IPSEC_TXDMA2_APP_HEADER_END 
    
    phvwri p.eth_hdr_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr  p.eth_hdr_dma_cmd_addr, k.ipsec_to_stage3_in_page
    phvwr  p.eth_hdr_dma_cmd_size, k.ipsec_to_stage3_headroom
    
    phvwri p.dec_pay_load_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr p.dec_pay_load_dma_cmd_addr, k.t0_s2s_out_page_addr
    add r3, r0, k.txdma2_global_payload_size
    //payload-size includes pad - subtract pad_size now
    sub r3, r3, k.txdma2_global_pad_size
    phvwr p.dec_pay_load_dma_cmd_size, r3
    phvwri p.dec_pay_load_dma_cmd_eop, 1
    phvwri p.dec_pay_load_dma_pkt_eop, 1

    phvwri p.p4_txdma_intr_dma_cmd_ptr, N2H_TXDMA2_DMA_COMMANDS_OFFSET

    phvwri p.app_header_table0_valid, 0 
    phvwri p.app_header_table1_valid, 0 
    phvwri p.app_header_table2_valid, 0 
    phvwri p.app_header_table3_valid, 0 
    nop.e 
    nop
