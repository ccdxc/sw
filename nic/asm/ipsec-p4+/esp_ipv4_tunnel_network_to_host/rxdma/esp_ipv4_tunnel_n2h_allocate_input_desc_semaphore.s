#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_esp_v4_tunnel_n2h_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_allocate_input_desc_index 
        .param          esp_ipv4_tunnel_n2h_allocate_output_desc_index 
        .param          IPSEC_BIG_RNMPR_TABLE_BASE
        .param          IPSEC_BIG_TNMPR_TABLE_BASE
        .param          IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align

#define c_qfull c7

esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore:
    add r1, d.{in_desc_pi}.wx, 1
    seq c_qfull, r1, d.{in_desc_ci}.wx

    and r1, d.{in_desc_pi}.wx, IPSEC_DESC_RING_INDEX_MASK 
    tbladd.!c_qfull.f d.{in_desc_pi}.wx, 1

    sll r1, r1, IPSEC_DESC_RING_ENTRY_SHIFT_SIZE 
    addui r2, r1, hiword(IPSEC_BIG_RNMPR_TABLE_BASE)
    addi r2, r2, loword(IPSEC_BIG_RNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_allocate_input_desc_index, r2, TABLE_SIZE_64_BITS)
    addui r4, r1, hiword(IPSEC_BIG_TNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_BIG_TNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_allocate_output_desc_index, r4, TABLE_SIZE_64_BITS)
    bcf [c_qfull], esp_ipv4_tunnel_n2h_desc_ring_full
    nop
    nop.e
    nop
     
esp_ipv4_tunnel_n2h_desc_ring_full:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_DESC_RING_OFFSET, 1)
    phvwri p.p4_rxdma_intr_dma_cmd_ptr, N2H_RXDMA_IPSEC_DMA_COMMANDS_OFFSET
    phvwri p.t0_s2s_in_desc_addr, IPSEC_DESC_FULL_DESC_ADDR
    and r3, k.ipsec_global_cb_pindex, IPSEC_CB_RING_INDEX_MASK
    sll r3, r3, IPSEC_CB_RING_ENTRY_SHIFT_SIZE
    add r3, r3, k.ipsec_to_stage1_cb_ring_base_addr
    phvwr p.dma_cmd_post_cb_ring_dma_cmd_addr, r3
    phvwri p.{dma_cmd_post_cb_ring_dma_cmd_phv_end_addr...dma_cmd_post_cb_ring_dma_cmd_type}, ((IPSEC_CB_RING_IN_DESC_END << 18) | (IPSEC_CB_RING_IN_DESC_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    add r7, k.ipsec_global_cb_pindex, 1
    andi r7, r7, IPSEC_CB_RING_INDEX_MASK
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 1, k.ipsec_global_ipsec_cb_index, 0, r7, db_data_pid, db_data_index)
    add r1, r0, k.ipsec_global_ipsec_cb_index
    phvwri          p.doorbell_cmd_dma_cmd_eop, 1
    phvwri        p.doorbell_cmd_dma_cmd_wr_fence, 1
    nop.e
    nop
