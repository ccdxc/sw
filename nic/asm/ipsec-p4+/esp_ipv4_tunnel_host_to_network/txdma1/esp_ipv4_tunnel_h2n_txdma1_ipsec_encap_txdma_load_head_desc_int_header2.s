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
    phvwri.e p.{dma_cmd_post_barco_ring_dma_cmd_phv_end_addr...dma_cmd_post_barco_ring_dma_cmd_type}, ((IPSEC_TXDMA1_BARCO_CB_REQ_PHV_OFFSET_END << 18) | (IPSEC_TXDMA1_BARCO_CB_REQ_PHV_OFFSET_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    nop 


esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2_illegal_dma_in_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_BAD_INDESC_FREE, 1)
    phvwri p.txdma1_global_flags, IPSEC_GLOBAL_FLAGS_INVALID_ADDR
    nop.e
    nop

esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2_illegal_dma_out_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_OUT_DESC_OFFSET, 1)
    phvwri p.txdma1_global_flags, IPSEC_GLOBAL_FLAGS_INVALID_ADDR
    nop.e
    nop

