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

esp_ipv4_tunnel_h2n_dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    add r2, r0, k.ipsec_global_in_desc_addr
    blti  r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_desc
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.ipsec_global_in_desc_addr

esp_ipv4_tunnel_h2n_dma_cmd_fill_esp_hdr:
    add r6, k.ipsec_global_in_desc_addr, IPSEC_PAGE_OFFSET
    add r1, r6, IPSEC_SALT_HEADROOM
    add r1, r1, d.iv_size
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_page
    phvwr p.dma_cmd_fill_esp_hdr_dma_cmd_addr, r1
    smeqb c3, d.flags, IPSEC_FLAGS_RANDOM_MASK, IPSEC_FLAGS_RANDOM_MASK
    phvwri.!c3 p.{dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr...dma_cmd_fill_esp_hdr_dma_cmd_type}, ((IPSEC_ESP_HDR_PHV_END << 18) | (IPSEC_ESP_HDR_PHV_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwri.c3 p.{dma_cmd_fill_esp_hdr_dma_cmd_phv_end_addr...dma_cmd_fill_esp_hdr_dma_cmd_type}, ((IPSEC_ESP2_HDR_PHV_END << 18) | (IPSEC_ESP_HDR_PHV_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
  
esp_ipv4_tunnel_h2n_dma_cmd_to_write_input_desc_aol:
    add r1, k.ipsec_global_in_desc_addr, 64
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_in_desc
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    
esp_ipv4_tunnel_h2n_dma_cmd_to_write_output_desc_aol:
    add r1, k.ipsec_to_stage4_out_desc_addr, 64
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc2_illegal_dma_out_desc
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    phvwri.e p.{dma_cmd_out_desc_aol_dma_cmd_phv_end_addr...dma_cmd_out_desc_aol_dma_cmd_type}, ((IPSEC_OUT_DESC_AOL_END << 18) | (IPSEC_OUT_DESC_AOL_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
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

