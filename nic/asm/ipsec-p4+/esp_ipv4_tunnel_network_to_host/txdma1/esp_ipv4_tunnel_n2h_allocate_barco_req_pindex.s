#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t1_k k;
struct tx_table_s2_t1_esp_v4_tunnel_n2h_allocate_barco_req_pindex_d d;
struct phv_ p;

%%
       .param BRQ_GCM1_BASE
       .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
       .align
esp_ipv4_tunnel_n2h_allocate_barco_req_pindex:
    phvwri p.app_header_table1_valid, 0

    //sne c2, r0, d.stat_qfull
    //bcf [c2], esp_ipv4_tunnel_n2h_txdma1_allocate_barco_req_fail_freeze
    //nop

    add r4, d.pi, 1
    andi r4, r4, ASIC_BARCO_RING_SLOTS_MASK
    seq c1, r4, d.ci
    bcf [c1], esp_ipv4_tunnel_n2h_txdma1_allocate_barco_req_fail
    nop

    /* Check for the Barco Decrypt bug workaround slot on the ring */
    seq c1, d.pi[CAPRI_BARCO_GCM_DECRYPT_BUG_REQ_STRIDE_SHIFT-1:0], 0
    tblmincri.c1 d.pi, ASIC_BARCO_RING_SLOTS_SHIFT, 1
    nop

    add r4, d.pi, 1
    andi r4, r4, ASIC_BARCO_RING_SLOTS_MASK
    seq c1, r4, d.ci
    bcf [c1], esp_ipv4_tunnel_n2h_txdma1_allocate_barco_req_fail
    nop

    and r2, d.pi, ASIC_BARCO_RING_SLOTS_MASK 
    sll r2, r2, BRQ_RING_ENTRY_SIZE_SHIFT
    addui r2, r2, hiword(BRQ_GCM1_BASE)
    addi r2, r2, loword(BRQ_GCM1_BASE)
    phvwr p.ipsec_to_stage3_barco_req_addr, r2
    add r1, d.pi, 1
    and r1, r1, ASIC_BARCO_RING_SLOTS_MASK 
    tblwr.f d.pi, r1
    phvwr.e p.barco_dbell_pi, r1.wx
    nop 

esp_ipv4_tunnel_n2h_txdma1_allocate_barco_req_fail:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_BARCO_FULL_OFFSET, 1)
    tbladd.f d.stat_qfull, 1
    phvwri p.txdma1_global_flags, IPSEC_GLOBAL_FLAGS_BARCO_FULL
    nop.e
    nop


esp_ipv4_tunnel_n2h_txdma1_allocate_barco_req_fail_freeze:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_BARCO_FULL_FREEZE_OFFSET, 1)
    phvwri p.txdma1_global_flags, IPSEC_GLOBAL_FLAGS_BARCO_FULL
    nop.e
    nop

