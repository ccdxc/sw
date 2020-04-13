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
        .param IPSEC_PAGE_ADDR_RX 
esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc:
    phvwr p.ipsec_int_header_in_desc, k.t0_s2s_in_desc_addr
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
 
    add r2, r0, k.t0_s2s_in_desc_addr
    blti  r2, ASIC_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_in_desc
    nop
    phvwr p.dma_cmd_phv2mem_ipsec_int_dma_cmd_addr, k.t0_s2s_in_desc_addr
    add r1, k.t0_s2s_in_desc_addr, IPSEC_SCRATCH_OFFSET 
    phvwr p.dma_cmd_in_desc_aol_dma_cmd_addr, r1
    add r1, k.ipsec_to_stage4_out_desc_addr, IPSEC_SCRATCH_OFFSET 
    blti  r1, ASIC_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_out_desc
    nop
    phvwr p.dma_cmd_out_desc_aol_dma_cmd_addr, r1 
    and r3, k.ipsec_global_cb_pindex, IPSEC_CB_RING_INDEX_MASK
    sll r3, r3, IPSEC_CB_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.cb_ring_base_addr 
    blti  r3, ASIC_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_cb_ring
    nop
    phvwr p.dma_cmd_post_cb_ring_dma_cmd_addr, r3
    phvwri p.{dma_cmd_post_cb_ring_dma_cmd_phv_end_addr...dma_cmd_post_cb_ring_dma_cmd_type}, ((IPSEC_CB_RING_IN_DESC_END << 18) | (IPSEC_CB_RING_IN_DESC_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    add r7, k.ipsec_global_cb_pindex, 1
    andi r7, r7, IPSEC_CB_RING_INDEX_MASK 
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 1, k.ipsec_global_ipsec_cb_index, 0, r7, db_data_pid, db_data_index)
    and r6, k.ipsec_global_flags, IPSEC_N2H_GLOBAL_FLAGS 
    seq c1, r6, IPSEC_N2H_GLOBAL_FLAGS 
    bcf [c1], esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc_inc_counter
    nop
    nop.e
    nop

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc_inc_counter: 
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_RXDMA_DUMMY_DESC_OFFSET, 1)
    b esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc
    nop

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_in_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_IN_DESC_OFFSET, 1)
    b esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc
    nop

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_out_desc:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_OUT_DESC_OFFSET, 1)
    b esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc
    nop

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_cb_ring:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_CB_RING_OFFSET, 1)
    phvwri.e p.p4_intr_global_drop, 1 
    nop 
    //b esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc
    //nop

esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_dummy_desc:
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, 0
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, 0
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, 0
    phvwri p.{dma_cmd_pkt2mem_dma_cmd_cache...dma_cmd_pkt2mem_dma_cmd_type}, (IPSEC_PKT2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PKT_TO_MEM)
    addui r3, r0, hiword(IPSEC_PAGE_ADDR_RX)
    addi r3, r3, loword(IPSEC_PAGE_ADDR_RX)
    addi r3, r3, 9600
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r3
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_global_packet_length

    phvwri p.dma_cmd_pkt2mem2_dma_cmd_type, 0
    phvwri p.dma_cmd_iv_salt_dma_cmd_type, 0
    add r2, r0, d.cb_ring_base_addr
    and r3, k.ipsec_global_cb_pindex, IPSEC_CB_RING_INDEX_MASK
    sll r3, r3, IPSEC_CB_RING_ENTRY_SHIFT_SIZE
    add r3, r3, r2 
    phvwr p.dma_cmd_post_cb_ring_dma_cmd_addr, r3
    phvwri p.{dma_cmd_post_cb_ring_dma_cmd_phv_end_addr...dma_cmd_post_cb_ring_dma_cmd_type}, ((IPSEC_CB_RING_IN_DESC_END << 18) | (IPSEC_CB_RING_IN_DESC_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    blti  r2, ASIC_HBM_BASE, esp_ipv4_tunnel_n2h_ipsec_cb_tail_enqueue_input_desc_illegal_dma_cb_ring
    nop
    add r7, k.ipsec_global_cb_pindex, 1
    andi r7, r7, IPSEC_CB_RING_INDEX_MASK 
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 1, k.ipsec_global_ipsec_cb_index, 0, r7, db_data_pid, db_data_index)
    phvwri          p.doorbell_cmd_dma_cmd_eop, 1
    phvwri        p.doorbell_cmd_dma_cmd_wr_fence, 1
    phvwri.e p.t0_s2s_in_desc_addr, IPSEC_DESC_FULL_DESC_ADDR 
    nop
