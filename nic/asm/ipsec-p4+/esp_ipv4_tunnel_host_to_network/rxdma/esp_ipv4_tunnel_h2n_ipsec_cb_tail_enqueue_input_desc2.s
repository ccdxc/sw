#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t2_k k;
struct rx_table_s4_t2_ipsec_cb_tail_enqueue_input_desc2_d d;
struct phv_ p;

%%
        .param IPSEC_CB_BASE
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2:

    add r2, r0, k.ipsec_global_in_desc_addr
    blti  r2, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_desc
    nop
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.ipsec_global_in_desc_addr

    add r6, k.ipsec_global_in_desc_addr, IPSEC_PAGE_OFFSET
    add r1, r6, IPSEC_SALT_HEADROOM
    add r1, r1, d.iv_size
    blti  r1, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_page
    nop
    phvwr p.dma_cmd_fill_esp_hdr_dma_cmd_addr, r1
  
    add r1, k.ipsec_global_in_desc_addr, 64
    blti  r1, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_desc
    nop
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    
    add r1, k.ipsec_to_stage4_out_desc_addr, 64
    blti  r1, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_out_desc
    nop
    phvwr.e p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    nop

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_IN_DESC_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_page:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_T2_IN_PAGE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_out_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_OUT_DESC_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

