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

    seq c1, k.txdma1_global_flags, 1
    bcf [c1], esp_ipv4_tunnel_h2n_hit_errors
    nop

    add r1, d.barco_pindex, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK
    seq c5, d.barco_cindex, r1
    bcf [c5], esp_ipv4_tunnel_h2n_barco_ring_full_error
    nop
    sll r3, d.barco_pindex, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    tblmincri.f d.barco_pindex, IPSEC_BARCO_RING_WIDTH, 1 
    add r3, r3, d.barco_ring_base_addr 
    blti  r3, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma_barco_ring 
    nop
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0

    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 0, LIF_IPSEC_ESP)
    phvwr p.barco_req_doorbell_address, r4.dx
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, r1)
    phvwr p.barco_req_doorbell_data, r3.dx

    phvwri p.barco_req_header_size, ESP_FIXED_HDR_SIZE_LI

    add r2, r0, k.ipsec_to_stage4_barco_req_addr
    blti r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_ipsec_write_barco_req_illegal_dma_barco_req
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

esp_ipv4_tunnel_h2n_barco_ring_full_error:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_BARCO_RING_FULL, 1)
    b esp_ipv4_tunnel_h2n_hit_errors
    nop

esp_ipv4_tunnel_h2n_hit_errors:
    phvwri p.brq_req_write_dma_cmd_type, 0
    phvwri p.dma_cmd_post_barco_ring_dma_cmd_type, 0
    phvwri p.brq_in_desc_zero_dma_cmd_type, 0
    phvwri p.brq_out_desc_zero_dma_cmd_type, 0
    addi r5, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    addi r5, r5, IPSEC_H2N_SEM_CINDEX_OFFSET
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_txdma1_ipsec_free_resources, r5, TABLE_SIZE_32_BITS)
    nop.e
    nop

