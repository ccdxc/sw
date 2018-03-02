#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_update_input_desc_aol_d d;
struct phv_ p;

%%
        //.param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc
        //.param esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2
        .align

esp_ipv4_tunnel_h2n_update_input_desc_aol:
    //phvwri p.app_header_table0_valid...p.app_header_table3_valid}, 3 
    //phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc[33:6] 
    //phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, ((1 << 3) | 6 ) 
    //phvwr p.common_te0_phv_table_addr, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33} 
    //phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2[33:6] 
    //phvwr p.common_te1_phv_table_addr, k.{ipsec_global_ipsec_cb_addr_sbit0_ebit31...ipsec_global_ipsec_cb_addr_sbit32_ebit33} 
    //phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, ((1 << 3) | 6 ) 
    add r1, k.ipsec_to_stage3_iv_size, IPSEC_SALT_HEADROOM
    add r2, k.ipsec_to_stage3_iv_size, ESP_FIXED_HDR_SIZE
    add r4, r1, k.t0_s2s_in_page_addr 
    phvwr p.barco_desc_in_A0_addr, r4.dx
    phvwri p.barco_desc_in_L0, ESP_FIXED_HDR_SIZE_LI 
    add r5, r4, r2
    add r5, r5, k.t0_s2s_payload_start 
    phvwr p.barco_desc_in_A1_addr, r5.dx
    add r6, k.t0_s2s_payload_size, k.ipsec_to_stage3_pad_size
    addi r6, r6, 2 
    phvwr p.barco_desc_in_L1, r6.wx


dma_cmd_write_iv_to_in_desc:
    add r3, k.t0_s2s_in_page_addr, IPSEC_SALT_HEADROOM
    phvwri p.dma_cmd_iv_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr p.dma_cmd_iv_dma_cmd_addr, r3
    seq c1, k.ipsec_to_stage3_iv_size, 16
    phvwri p.dma_cmd_iv_dma_cmd_phv_start_addr, IPSEC_IN_DESC_IV_START
    phvwri.!c1 p.dma_cmd_iv_dma_cmd_phv_end_addr, IPSEC_IN_DESC_IV_END 
    phvwri.c1 p.dma_cmd_iv_dma_cmd_phv_end_addr, IPSEC_IN_DESC_IV2_END 
    nop.e 
    nop
