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
        .align

esp_ipv4_tunnel_n2h_update_input_desc_aol:
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14 
    phvwr p.common_te0_phv_table_addr, k.ipsec_to_stage3_ipsec_cb_addr 

    add r2, k.ipsec_to_stage3_iv_salt_off, k.t0_s2s_in_page_addr
    subi r5, r2, ESP_FIXED_HDR_SIZE 
    phvwr p.barco_desc_in_A0_addr, r5.dx 
    phvwri p.barco_desc_in_L0, ESP_FIXED_HDR_SIZE_LI 
    addi r3, r2, IPSEC_SALT_HEADROOM 
    add r3, r3, k.ipsec_to_stage3_iv_size
    phvwr p.barco_desc_in_A1_addr, r3.dx 
    phvwr p.barco_desc_in_L1, k.{ipsec_to_stage3_payload_size}.wx

dma_cmd_to_move_input_pkt_to_mem:
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, k.t0_s2s_in_page_addr
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage3_iv_salt_off
 
    sub r3, r3, k.ipsec_to_stage3_iv_size 
    phvwr p.dma_cmd_pkt2mem2_dma_cmd_addr, r3 
    add r5, k.ipsec_to_stage3_payload_size, k.ipsec_to_stage3_iv_size
    add r5, r5, k.ipsec_global_icv_size
    phvwr p.dma_cmd_pkt2mem2_dma_cmd_size, r5 

dma_cmd_to_write_salt_after_seq_no:
    add r1, k.ipsec_to_stage3_iv_salt_off, k.t0_s2s_in_page_addr
    phvwr p.dma_cmd_iv_salt_dma_cmd_addr, r1
    nop.e
    nop
