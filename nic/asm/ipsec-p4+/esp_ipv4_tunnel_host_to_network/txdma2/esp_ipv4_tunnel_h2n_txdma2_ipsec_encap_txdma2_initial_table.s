#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_ipsec_encap_txdma2_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req 
        .param BRQ_BASE
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_table:
    seq c1, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx 
    b.c1 esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing
    phvwri.c1 p.p4_intr_global_drop, 1

    phvwr p.txdma2_global_iv_size, d.iv_size
    phvwr p.txdma2_global_icv_size, d.icv_size
    phvwri p.{p4_intr_global_tm_iport...p4_intr_global_tm_oport}, ((TM_OPORT_DMA << 4) | TM_OPORT_P4INGRESS) 
    phvwri p.p4_intr_global_lif, ARM_CPU_LIF 
    phvwri p.app_header_table0_valid, 1
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req[33:6] 
    sll r1, d.barco_cindex, BRQ_RING_ENTRY_SIZE_SHIFT 
    add r1, r1, d.barco_ring_base_addr 
    phvwr  p.common_te0_phv_table_addr, r1
    phvwr p.ipsec_to_stage2_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.ipsec_to_stage3_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    and r2, d.flags, 1
    smeqb c2, d.flags, IPSEC_FLAGS_V6_MASK, IPSEC_FLAGS_V6_MASK 
    phvwr.c2 p.ipsec_to_stage3_is_v6, 1 
    smeqb c3, d.flags, IPSEC_FLAGS_NATT_MASK, IPSEC_FLAGS_NATT_MASK 
    phvwr.c3.f p.ipsec_to_stage3_is_nat_t, 1
    add r7, d.barco_cindex, 1
    and r7, r7, IPSEC_BARCO_RING_INDEX_MASK  
    tblwr d.barco_cindex, r7 
    nop
    tblmincri.f  d.{barco_ring_cindex}.hx, CAPRI_SESQ_RING_SLOTS_SHIFT, 1
    nop
    seq c1, d.{barco_ring_pindex}.hx, d.{barco_ring_cindex}.hx 
    b.!c1 esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 1, 0)
    memwr.dx  r4, r3

esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_initial_do_nothing:
    nop.e
    nop


