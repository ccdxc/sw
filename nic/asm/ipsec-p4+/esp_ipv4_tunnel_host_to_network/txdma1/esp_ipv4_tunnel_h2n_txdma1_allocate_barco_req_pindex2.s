#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"


struct tx_table_s3_t2_allocate_barco_req_pindex_d d;
struct tx_table_s3_t2_k k;
struct phv_ p;

%%
       .param BRQ_BASE 
       .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
       .align
esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2:
    add r4, d.pi, 1
    andi r4, r4, IPSEC_BARCO_RING_INDEX_MASK
    seq c1, r4, d.ci 
    bcf [c1], esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_fail
    sll r2, d.pi, BRQ_RING_ENTRY_SIZE_SHIFT 
    addui r2, r2, hiword(BRQ_BASE)
    addi r2, r2, loword(BRQ_BASE)
    blti  r2, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2_illegal 
    nop
    phvwr p.ipsec_to_stage4_barco_req_addr, r2
    phvwr p.barco_dbell_pi, r4.wx
    tblwr.f d.pi, r4
    nop.e
    nop


esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_fail:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_BARCO_FULL_OFFSET, 1)
    tbladd.f d.stat_qfull, 1
    phvwri p.txdma1_global_flags, 1
    nop.e 
    nop


esp_ipv4_tunnel_h2n_txdma1_allocate_barco_req_pindex2_illegal:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_BAD_BARCO_ADDR_OFF, 1)
    phvwri p.txdma1_global_flags, 1
    nop.e
    nop
   
