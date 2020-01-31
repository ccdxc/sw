#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .param TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE
        .param esp_ipv4_tunnel_n2h_allocate_barco_req_pindex
        .align
esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex:
    phvwri p.{brq_req_write_dma_cmd_phv_end_addr...brq_req_write_dma_cmd_type}, ((IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END << 18) | (IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    phvwri p.p4_txdma_intr_dma_cmd_ptr, N2H_TXDMA1_DMA_COMMANDS_OFFSET
    phvwri p.{dma_cmd_post_barco_ring_dma_cmd_phv_end_addr...dma_cmd_post_barco_ring_dma_cmd_type}, ((IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_END << 18) | (IPSEC_TXDMA1_BARCO_REQ_PHV_OFFSET_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    add r5, r0, d.in_desc_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header, r5, TABLE_SIZE_512_BITS)
    addi r1, r0, IPSEC_DESC_FULL_DESC_ADDR
    seq c1, d.in_desc_addr, r1 
    bcf [c1], esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex_abort_txdma1
    nop
    addui       r5, r0, hiword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    addi        r5, r5, loword(TLS_PROXY_BARCO_GCM1_PI_HBM_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_allocate_barco_req_pindex, r5, TABLE_SIZE_512_BITS)
    nop.e
    nop

esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex_abort_txdma1:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_ABORT_TXDMA1_DUMMY_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0

    // queue dummy request to txdma2
    sll r3, k.ipsec_to_stage1_barco_pindex, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, k.ipsec_to_stage1_barco_cb_ring_base_addr
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3
    phvwr p.barco_req_input_list_address, 0
    phvwri p.{dma_cmd_post_barco_ring_dma_cmd_phv_end_addr...dma_cmd_post_barco_ring_dma_cmd_type}, ((IPSEC_TXDMA1_BARCO_CB_REQ_PHV_OFFSET_END << 18) | (IPSEC_TXDMA1_BARCO_CB_REQ_PHV_OFFSET_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)

    // TODO : Handle record ring filling up with DESC_EXHAUST
    // cannot ring doorbell to wake txdma2. TODO: handle in stage 4
    nop.e
    nop
 
