#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"


struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_rx_table_s3_t0_cfg_action_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc 
        .param IPSEC_CB_BASE
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align

esp_ipv4_tunnel_n2h_update_input_desc_aol:

    add r6, k.t0_s2s_in_desc_addr, IPSEC_PAGE_OFFSET
    phvwr p.ipsec_int_header_in_page, r6 
    add r2, k.ipsec_to_stage3_iv_salt_off, r6 
    subi r5, r2, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_desc_in_A0_addr, r5.dx 
    phvwri p.barco_desc_in_L0, ESP_FIXED_HDR_SIZE_LI 
    addi r3, r2, IPSEC_SALT_HEADROOM 
    add r3, r3, k.ipsec_to_stage3_iv_size
    phvwr p.barco_desc_in_A1_addr, r3.dx 
    phvwr p.barco_desc_in_L1, k.{ipsec_to_stage3_payload_size}.wx

dma_cmd_to_move_input_pkt_to_mem:
    add r2, r0, r6 
    blti  r2, ASIC_HBM_BASE, esp_ipv4_tunnel_n2h_update_input_desc_aol_illegal_dma_in_page
    nop
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r6 
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage3_iv_salt_off
 
    sub r3, r3, k.ipsec_to_stage3_iv_size 
    blti  r3, ASIC_HBM_BASE, esp_ipv4_tunnel_n2h_update_input_desc_aol_illegal_dma_in_page
    nop
    phvwr p.dma_cmd_pkt2mem2_dma_cmd_addr, r3 
    add r5, k.ipsec_to_stage3_payload_size, k.ipsec_to_stage3_iv_size
    add r5, r5, IPSEC_DEFAULT_ICV_SIZE 
    phvwr p.dma_cmd_pkt2mem2_dma_cmd_size, r5 

dma_cmd_to_write_salt_after_seq_no:
    add r1, k.ipsec_to_stage3_iv_salt_off, r6 
    phvwr.e p.dma_cmd_iv_salt_dma_cmd_addr, r1
    nop

esp_ipv4_tunnel_n2h_update_input_desc_aol_illegal_dma_in_page:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_IN_DESC_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop

