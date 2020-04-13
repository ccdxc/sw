#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct rx_table_s4_t0_k k;
struct rx_table_s4_t0_ipsec_cb_tail_enqueue_input_desc_d d;
struct phv_ p;

%%
        .param IPSEC_CB_BASE
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .param esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats
        .param IPSEC_PAGE_ADDR_RX
        .align

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc:

    phvwr p.ipsec_int_header_in_desc, k.ipsec_global_in_desc_addr
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0

    add r2, r0, d.cb_ring_base_addr
    and r3, k.ipsec_global_ipsec_cb_pindex, IPSEC_CB_RING_INDEX_MASK 
    sll r3, r3, IPSEC_CB_RING_ENTRY_SHIFT_SIZE
    add r3, r3, r2
    blti  r3, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc_illegal_cb_ring_dma 
    nop
    phvwr p.dma_cmd_post_cb_ring_dma_cmd_addr, r3
    phvwri p.{dma_cmd_post_cb_ring_dma_cmd_phv_end_addr...dma_cmd_post_cb_ring_dma_cmd_type}, ((IPSEC_H2N_CB_RING_IN_DESC_END << 18) | (IPSEC_H2N_CB_RING_IN_DESC_START << 8) | IPSEC_PHV2MEM_CACHE_ENABLE | CAPRI_DMA_COMMAND_PHV_TO_MEM)
    add r7, k.ipsec_global_ipsec_cb_pindex, 1
    andi r7, r7, IPSEC_CB_RING_INDEX_MASK 

    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 0, k.ipsec_global_ipsec_cb_index, 0, r7, db_data_pid, db_data_index)
    and r6, k.ipsec_to_stage4_flags, IPSEC_N2H_GLOBAL_FLAGS
    seq c1, r6, IPSEC_N2H_GLOBAL_FLAGS
    bcf [c1], esp_ipv4_tunnel_h2n_rxdma_disbale_dma_cmds 
    nop
    //add r4, k.ipsec_global_ipsec_cb_addr, IPSEC_H2N_STATS_CB_OFFSET
    //CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats, r4, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(1, esp_ipv4_tunnel_h2n_rxdma_ipsec_update_rx_stats)
    nop.e
    nop

esp_ipv4_tunnel_h2n_ipsec_cb_tail_enqueue_input_desc_illegal_cb_ring_dma:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_CB_RING_DMA_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop

esp_ipv4_tunnel_h2n_rxdma_disbale_dma_cmds:
    phvwri p.ipsec_global_in_desc_addr, IPSEC_DESC_FULL_DESC_ADDR
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri p.dma_cmd_phv2mem_ipsec_int_dma_cmd_type, 0
    phvwri p.{dma_cmd_pkt2mem_dma_cmd_cache...dma_cmd_pkt2mem_dma_cmd_type}, (IPSEC_MEM2PKT_CACHE_ENABLE | CAPRI_DMA_COMMAND_PKT_TO_MEM)
    addui r3, r0, hiword(IPSEC_PAGE_ADDR_RX)
    addi r3, r3, loword(IPSEC_PAGE_ADDR_RX)
    addi r3, r3, 9600
    phvwr p.dma_cmd_pkt2mem_dma_cmd_addr, r3
    phvwr p.dma_cmd_pkt2mem_dma_cmd_size, k.ipsec_to_stage4_packet_len[13:0]
    phvwri p.dma_cmd_iv_salt_dma_cmd_type, 0
    phvwri p.dma_cmd_out_desc_aol_dma_cmd_type, 0
    phvwri p.dma_cmd_fill_esp_hdr_dma_cmd_type, 0
    //phvwri p.dma_cmd_pad_byte_src_dma_cmd_type, 0
    //phvwri p.dma_cmd_pad_byte_dst_dma_cmd_type, 0
    phvwri p.dma_cmd_pad_mem_dma_cmd_type, 0
    phvwri p.dma_cmd_in_desc_aol_dma_cmd_type, 0
    phvwri p.dma_cmd_iv_dma_cmd_type, 0
    phvwri.e p.tail_2_bytes_dma_cmd_type, 0
    nop
