#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t0_k k;
struct rx_table_s4_t0_esp_v4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc:
    phvwr p.ipsec_int_header_in_desc, k.t0_s2s_in_desc_addr
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
 
dma_cmd_to_write_ipsec_int_from_rxdma_to_txdma:
    add r2, r0, k.t0_s2s_in_desc_addr
    blti  r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_in_desc
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t0_s2s_in_desc_addr

dma_cmd_to_write_input_desc_aol:
    add r1, k.t0_s2s_in_desc_addr, 64
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    
dma_cmd_to_write_output_desc_aol:
    add r1, k.ipsec_to_stage4_out_desc_addr, 64
    blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_out_desc
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 

esp_ipv4_tunnel_n2h_post_to_cb_ring:
    and r3, k.ipsec_global_cb_pindex, IPSEC_CB_RING_INDEX_MASK

    sll r3, r3, IPSEC_CB_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.cb_ring_base_addr 
    blti  r3, CAPRI_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_cb_ring
    phvwr p.dma_cmd_post_cb_ring_dma_cmd_addr, r3
    phvwri p.{dma_cmd_post_cb_ring_dma_cmd_phv_end_addr...dma_cmd_post_cb_ring_dma_cmd_type}, ((IPSEC_CB_RING_IN_DESC_END << 18) | (IPSEC_CB_RING_IN_DESC_START << 8) | CAPRI_DMA_COMMAND_PHV_TO_MEM)

esp_ipv4_tunnel_n2h_dma_cmd_incr_pindex:
    add r7, k.ipsec_global_cb_pindex, 1
    andi r7, r7, IPSEC_CB_RING_INDEX_MASK 
dma_cmd_ring_doorbell:
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 1, k.ipsec_global_ipsec_cb_index, 0, r7, db_data_pid, db_data_index)
    add r1, r0, k.ipsec_global_ipsec_cb_index
    phvwri          p.doorbell_cmd_dma_cmd_eop, 1
    phvwri.e        p.doorbell_cmd_dma_cmd_wr_fence, 1
    nop



esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_in_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_IN_DESC_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop
esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_out_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_OUT_DESC_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop
esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_cb_ring:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_CB_RING_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop

