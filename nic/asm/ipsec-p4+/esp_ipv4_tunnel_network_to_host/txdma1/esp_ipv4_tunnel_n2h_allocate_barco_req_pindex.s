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
    /* Check for the Barco Decrypt bug workaround slot on the ring */
    and     r1, d.pi, CAPRI_BARCO_GCM_DECRYPT_BUG_REQ_STRIDE_MASK
    seq     c1, r1, 0
    tblmincri.c1    d.pi, CAPRI_BARCO_RING_SLOTS_SHIFT, 1
    nop

    and r2, d.pi, IPSEC_BARCO_RING_INDEX_MASK
    sll r2, r2, BRQ_RING_ENTRY_SIZE_SHIFT
    addui r2, r2, hiword(BRQ_GCM1_BASE)
    addi r2, r2, loword(BRQ_GCM1_BASE)
    phvwr p.ipsec_to_stage3_barco_req_addr, r2
    add r1, d.pi, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK
    tblwr.f d.pi, r1
    phvwr.e p.barco_dbell_pi, r1.wx
    nop 
