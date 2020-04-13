#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s4_t0_k k;
struct tx_table_s4_t0_ipsec_write_barco_req_d d;
struct phv_ p;

%%
        .align
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_free_resources

esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req:
    seq c1, k.ipsec_to_stage4_cb_cindex[3:0], 0
    tblwr.c1 d.cb_cindex, k.ipsec_to_stage4_cb_cindex
    sne c1, k.txdma1_global_flags, 0
    bcf [c1], esp_ipv4_tunnel_h2n_hit_errors
    nop

    tblwr.f d.barco_full_count, 0

    sll r3, k.ipsec_to_stage4_barco_pindex, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.barco_ring_base_addr 
    blti  r3, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma_barco_ring 
    nop
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 0, LIF_IPSEC_ESP)
    phvwr p.barco_req_doorbell_address, r4.dx
    add r1, k.ipsec_to_stage4_barco_pindex, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK 
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, r1)
    phvwr p.barco_req_doorbell_data, r3.dx
    add r2, r0, k.ipsec_to_stage4_barco_req_addr
    blti r2, ASIC_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma_barco_req
    nop

    CAPRI_DMA_CMD_PHV2MEM_SETUP_I(dma_cmd_incr_pindex_dma_cmd, CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX, barco_dbell_pi, barco_dbell_pi)
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd_incr_pindex_dma_cmd)
    phvwr.e p.brq_req_write_dma_cmd_addr, k.ipsec_to_stage4_barco_req_addr 
    nop


esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma_barco_ring:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_BARCO_CB_OFFSET, 1)
    b esp_ipv4_tunnel_h2n_hit_errors
    nop

esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma_barco_req:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_BARCO_REQ_OFFSET, 1)
    b esp_ipv4_tunnel_h2n_hit_errors
    nop

esp_ipv4_tunnel_h2n_hit_errors:
    // Queue to record ring even on error
    sll r3, k.ipsec_to_stage4_barco_pindex, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.barco_ring_base_addr 
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3
    phvwr p.barco_req_input_list_address, 0

    // count consecutive barco full
    tbladd d.barco_full_count, 1
    sne c1, d.barco_full_count, (IPSEC_BARCO_RING_SIZE - 1)
    bcf [c1], esp_ipv4_tunnel_h2n_skip_doorbell
    nop


    // We have queued ring full of dummy barco full requests
    // Doorbell to TxDMA2
    add r7, k.ipsec_to_stage4_barco_pindex, 1
    andi r7, r7, IPSEC_BARCO_RING_INDEX_MASK
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_FENCE(doorbell_cmd_dma_cmd, LIF_IPSEC_ESP, 0, d.ipsec_cb_index, 1, r7, db_data_pid, db_data_index)

esp_ipv4_tunnel_h2n_skip_doorbell:
    phvwri p.brq_req_write_dma_cmd_type, 0
    phvwri p.brq_in_desc_zero_dma_cmd_type, 0
    phvwri p.brq_out_desc_zero_dma_cmd_type, 0
    addi r5, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    addi r5, r5, IPSEC_H2N_SEM_CINDEX_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_txdma1_ipsec_free_resources, r5, TABLE_SIZE_32_BITS)
    nop.e
    nop

