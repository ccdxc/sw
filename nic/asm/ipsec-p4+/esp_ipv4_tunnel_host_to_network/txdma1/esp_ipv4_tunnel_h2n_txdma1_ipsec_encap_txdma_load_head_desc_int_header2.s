#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_ipsec_encap_txdma_load_head_desc_int_header_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2:
    phvwri p.app_header_table1_valid, 0
    phvwri p.{brq_req_write_dma_cmd_phv_end_addr...brq_req_write_dma_cmd_type}, ((IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END << 18) | (IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwri p.{dma_cmd_post_barco_ring_dma_cmd_phv_end_addr...dma_cmd_post_barco_ring_dma_cmd_type}, ((IPSEC_TXDMA1_BARCO_CB_REQ_PHV_OFFSET_END << 18) | (IPSEC_TXDMA1_BARCO_CB_REQ_PHV_OFFSET_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)

    add r2, d.in_desc, 96
    add r3, d.out_desc, 96
    blti  r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2_illegal_dma_in_desc
    phvwr p.brq_in_desc_zero_dma_cmd_addr, r2 
    phvwri p.{brq_in_desc_zero_dma_cmd_phv_end_addr...brq_in_desc_zero_dma_cmd_type}, ((IPSEC_DESC_ZERO_CONTENT_END << 18) | (IPSEC_DESC_ZERO_CONTENT_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    blti  r3, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2_illegal_dma_out_desc
    phvwr p.brq_out_desc_zero_dma_cmd_addr, r3 
    phvwri.e p.{brq_out_desc_zero_dma_cmd_phv_end_addr...brq_out_desc_zero_dma_cmd_type}, ((IPSEC_DESC_ZERO_CONTENT_END << 18) | (IPSEC_DESC_ZERO_CONTENT_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop 


esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2_illegal_dma_in_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_IN_DESC_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2_illegal_dma_out_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_OUT_DESC_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

