#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_esp_v4_tunnel_n2h_txdma2_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_load_barco_req 
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align
esp_ipv4_tunnel_n2h_txdma2_initial_table:
    //seq c6, d.is_v6, 0xFF
    //bcf [c6], txdma2_freeze2
    //nop

    seq c2, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx
    bcf [c2], esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing
    nop

    and r1, d.{barco_ring_cindex}.hx, IPSEC_BARCO_RING_INDEX_MASK

    tblmincri.f  d.{barco_ring_cindex}.hx, IPSEC_BARCO_RING_WIDTH, 1

    phvwr p.t0_s2s_barco_sw_cindex, d.{barco_ring_cindex}.hx

    sll r1, r1, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE 
    add r1, r1, d.barco_ring_base_addr 

    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.t0_s2s_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.ipsec_to_stage4_vrf_vlan, d.vrf_vlan
    seq c1, d.is_v6, 1
    cmov r6, c1, IPV6_ETYPE, IPV4_ETYPE
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_load_barco_req, r1, TABLE_SIZE_512_BITS)
    phvwr p.ipsec_to_stage4_ip_etype, r6
    //addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    //CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA2_ENTER_OFFSET, 1)
    seq c2, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx
    b.!c2 esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing2
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 1, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, 0)
    memwr.dx  r4, r3
    nop.e
    nop


esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing:
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop

esp_ipv4_tunnel_n2h_txdma2_initial_table_do_nothing2:
    nop.e
    nop

txdma2_freeze2:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_TXDMA2_FREEZE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop

