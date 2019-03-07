#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_esp_v4_tunnel_n2h_txdma1_initial_table_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex
        .param esp_v4_tunnel_n2h_load_part2
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align
esp_ipv4_tunnel_n2h_txdma_initial_table:
    //seq c6, d.is_v6, 0xFF
    //bcf [c6], txdma1_freeze2
    //nop

    add r1, d.{barco_ring_pindex}.hx, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK
    seq c5, d.{barco_ring_cindex}.hx, r1
    bcf [c5], esp_ipv4_tunnel_n2h_txdma_initial_table_drop_pkt
    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    b.c1 esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing
    phvwri.c1 p.p4_intr_global_drop, 1

    and r2, d.cb_cindex, IPSEC_CB_RING_INDEX_MASK 
    tblmincri d.cb_cindex, IPSEC_PER_CB_RING_WIDTH, 1
    tblmincri.f     d.{rxdma_ring_cindex}.hx, IPSEC_PER_CB_RING_WIDTH, 1
    phvwr p.txdma1_global_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33} 
    phvwr p.barco_req_command, d.barco_enc_cmd
    phvwr p.t0_s2s_iv_size, d.iv_size

    sll r2, r2, IPSEC_CB_RING_ENTRY_SHIFT_SIZE 
    add r2, r2, d.cb_ring_base_addr
    phvwr p.common_te0_phv_table_addr, r2
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex, r2, TABLE_SIZE_64_BITS)

    add r4, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}, 64
    phvwr p.common_te2_phv_table_addr, r4 

    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN, esp_v4_tunnel_n2h_load_part2, r4, TABLE_SIZE_64_BITS)

    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA1_ENTER_OFFSET, 1)
    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    b.!c1 esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing 
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 1, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 0, 0)
    memwr.dx  r4, r3


esp_ipv4_tunnel_n2h_txdma1_initial_table_do_nothing:
    nop.e
    nop

esp_ipv4_tunnel_n2h_txdma_initial_table_drop_pkt:
    phvwri p.p4_intr_global_drop, 1
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA1_ENTER_DROP_OFFSET, 1)
    nop.e
    nop

txdma1_freeze2:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA1_FREEZE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop
    
