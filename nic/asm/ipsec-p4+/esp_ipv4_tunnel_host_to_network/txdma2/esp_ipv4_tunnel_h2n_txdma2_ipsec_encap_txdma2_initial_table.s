#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_ipsec_encap_txdma2_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_dummy 
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table:
    //seq c6, d.flags, 0xFF
    //bcf [c6], txdma2_freeze
    //nop

    seq c1, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx 
    bcf [c1], esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing
    nop

    and r1, d.{barco_ring_cindex}.hx, IPSEC_BARCO_RING_INDEX_MASK 
    tblmincri.f  d.{barco_ring_cindex}.hx, IPSEC_BARCO_RING_WIDTH, 1

    phvwr p.ipsec_to_stage2_barco_sw_cindex, d.{barco_ring_cindex}.hx

    sll r1, r1, IPSEC_BARCO_RING_ENTRY_SHIFT_SIZE 
    add r1, r1, d.barco_ring_base_addr 
    phvwr  p.ipsec_to_stage1_barco_desc_addr, r1
    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.ipsec_to_stage4_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.ipsec_to_stage2_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    and r2, d.flags, 1
    smeqb c2, d.flags, IPSEC_FLAGS_V6_MASK, IPSEC_FLAGS_V6_MASK 
    phvwr.c2 p.ipsec_to_stage4_is_v6, 1 
    smeqb c4, d.flags, IPSEC_ENCAP_VLAN_MASK, IPSEC_ENCAP_VLAN_MASK
    phvwr.c4 p.ipsec_to_stage4_is_vlan_encap, 1
    smeqb c3, d.flags, IPSEC_FLAGS_NATT_MASK, IPSEC_FLAGS_NATT_MASK 
    phvwr.c3 p.ipsec_to_stage4_is_nat_t, 1
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, esp_ipv4_tunnel_h2n_txdma2_ipsec_dummy)

    //addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    //CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA2_ENTER_OFFSET, 1)
    seq c1, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx 
    bcf [!c1], esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing2
    nop
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, 0)
    memwr.dx  r4, r3
    nop.e
    nop

esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing:
    phvwri p.p4_intr_global_drop, 1
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing2:
    nop.e
    nop

txdma2_freeze:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_FREEZE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop

