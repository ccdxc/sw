#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_esp_v4_tunnel_n2h_txdma1_write_barco_req_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_update_cb
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align
esp_v4_tunnel_n2h_write_barco_req:
    add r2, r0, k.ipsec_to_stage3_barco_req_addr
    blti  r2, CAPRI_HBM_BASE,  esp_v4_tunnel_n2h_write_barco_req_illegal_dma_barco_req
    nop
    phvwr p.brq_req_write_dma_cmd_addr, k.ipsec_to_stage3_barco_req_addr 
    seq c1, k.ipsec_to_stage3_new_key, 1
    phvwr.c1 p.barco_req_key_desc_index, d.{new_key_index}.wx
    phvwr.!c1 p.barco_req_key_desc_index, d.{key_index}.wx

esp_v4_tunnel_n2h_post_to_barco_ring:
    add r4, d.barco_pindex, 1
    seq c1, d.barco_cindex, r4
    bcf [c1], esp_v4_tunnel_n2h_barco_ring_error
    nop
    and r3, d.barco_pindex, IPSEC_BARCO_RING_INDEX_MASK
    sll r3, r3, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE
    add r3, r3, d.barco_ring_base_addr
    blti  r3, CAPRI_HBM_BASE,  esp_v4_tunnel_n2h_write_barco_req_illegal_dma_barco_cb
    nop
    phvwr p.dma_cmd_post_barco_ring_dma_cmd_addr, r3

    tblmincri.f d.barco_pindex, IPSEC_BARCO_RING_WIDTH, 1

    add r2, r0, k.txdma1_global_ipsec_cb_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_v4_tunnel_n2h_txdma1_update_cb, r2, TABLE_SIZE_512_BITS)

    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 1, LIF_IPSEC_ESP)
    phvwr p.barco_req_doorbell_address, r4.dx
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, d.barco_pindex)
    phvwr.e p.barco_req_doorbell_data, r3.dx
    nop

esp_v4_tunnel_n2h_write_barco_req_illegal_dma_barco_req:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_BARCO_REQ_OFFSET, 1)    
    phvwri.e p.txdma1_global_flags, 1
    nop

esp_v4_tunnel_n2h_write_barco_req_illegal_dma_barco_cb:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_BARCO_CB_OFFSET, 1)    
    phvwri.e p.txdma1_global_flags, 1
    nop

esp_v4_tunnel_n2h_barco_ring_error:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA1_BARCO_RING_FULL_OFFSSET, 1)    
    phvwri.e p.txdma1_global_flags, 1
    nop

