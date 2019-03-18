#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_ipsec_encap_txdma_initial_table_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_s1_dummy
        .param esp_ipv4_tunnel_h2n_txdma1_ring_full_error 
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table:
    //seq c6, d.flags, 0xFF
    //bcf [c6], txdma1_freeze
    //nop

    add r1, d.barco_pindex, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK 
    seq c5, d.barco_cindex, r1
    bcf [c5], esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table_barco_ring_full
    nop
    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    bcf [c1], esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_do_nothing
    nop
    // Fill the barco command and key-index
    phvwr p.ipsec_to_stage4_barco_pindex, d.barco_pindex
    tblmincri  d.barco_pindex, IPSEC_BARCO_RING_WIDTH, 1
 
    and r2, d.cb_cindex, IPSEC_CB_RING_INDEX_MASK 
    tblmincri d.cb_cindex, IPSEC_PER_CB_RING_WIDTH, 1 
    tblmincri.f     d.{rxdma_ring_cindex}.hx, IPSEC_PER_CB_RING_WIDTH, 1
    phvwr p.txdma1_global_ipsec_cb_addr, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1...p4_txdma_intr_qstate_addr_sbit2_ebit33}
    phvwr p.barco_req_command, d.barco_enc_cmd
    add r6, r0, d.{key_index}
    phvwr p.barco_req_key_desc_index, r6.wx 
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, esp_ipv4_tunnel_h2n_txdma1_s1_dummy)
    sll r2, r2, 3
    add r2, r2, d.cb_ring_base_addr
    phvwr p.ipsec_to_stage1_cb_ring_slot_addr, r2
    //addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    //CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_ENTER_OFFSET, 1)
    seq c1, d.{rxdma_ring_pindex}.hx, d.{rxdma_ring_cindex}.hx
    bcf [!c1], esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_do_nothing2
    nop 
    addi r4, r0, CAPRI_DOORBELL_ADDR(0, DB_IDX_UPD_NOP, DB_SCHED_UPD_EVAL, 0, LIF_IPSEC_ESP)
    CAPRI_RING_DOORBELL_DATA(0, d.ipsec_cb_index, 0, 0)
    memwr.dx  r4, r3
    nop.e
    nop

esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_do_nothing:
    phvwri p.p4_intr_global_drop, 1
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_do_nothing2:
    nop.e
    nop
   
esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_initial_table_barco_ring_full:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_BARCO_RING_FULL, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop


txdma1_freeze:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_FREEZE_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    nop.e
    nop

