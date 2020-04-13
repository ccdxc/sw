#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s3_t0_k k;
struct rx_table_s3_t0_update_input_desc_aol_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
esp_ipv4_tunnel_h2n_update_input_desc_aol:
    add r1, k.ipsec_to_stage3_iv_size, IPSEC_SALT_HEADROOM
    add r2, k.ipsec_to_stage3_iv_size, ESP_FIXED_HDR_SIZE
    add r7, k.ipsec_global_in_desc_addr, IPSEC_PAGE_OFFSET
    add r4, r1, r7 
    phvwr p.barco_desc_in_A0_addr, r4.dx
    phvwri p.barco_desc_in_L0, ESP_FIXED_HDR_SIZE_LI 
    add r5, r4, r2
    add r5, r5, k.t0_s2s_payload_start 
    phvwr p.barco_desc_in_A1_addr, r5.dx
    add r6, k.t0_s2s_payload_size, k.ipsec_to_stage3_pad_size
    addi r6, r6, 2 
    phvwr p.barco_desc_in_L1, r6.wx
    phvwr p.ipsec_int_header_in_page, r7
    phvwri p.{dma_cmd_in_desc_aol_dma_cmd_phv_end_addr...dma_cmd_in_desc_aol_dma_cmd_type}, ((IPSEC_IN_DESC_AOL_END << 18) | (IPSEC_IN_DESC_AOL_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE| CAPRI_DMA_COMMAND_PHV_TO_MEM)


    add r3, r7, IPSEC_SALT_HEADROOM
    blti  r3, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_update_input_desc_aol_illegal_dma_in_page
    nop
    phvwr p.dma_cmd_iv_dma_cmd_addr, r3
    seq c1, k.ipsec_to_stage3_iv_size, 16
    phvwri.!c1 p.{dma_cmd_iv_dma_cmd_phv_end_addr...dma_cmd_iv_dma_cmd_type}, ((IPSEC_IN_DESC_IV_END << 18) | (IPSEC_IN_DESC_IV_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM) 
    phvwri.c1.f p.{dma_cmd_iv_dma_cmd_phv_end_addr...dma_cmd_iv_dma_cmd_type}, ((IPSEC_IN_DESC_IV2_END << 18) | (IPSEC_IN_DESC_IV_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop.e 
    nop


esp_ipv4_tunnel_h2n_update_input_desc_aol_illegal_dma_in_page:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_T0_IN_PAGE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

