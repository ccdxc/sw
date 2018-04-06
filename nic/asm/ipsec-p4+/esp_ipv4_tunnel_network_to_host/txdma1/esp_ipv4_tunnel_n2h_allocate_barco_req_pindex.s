#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_esp_v4_tunnel_n2h_allocate_barco_req_pindex_d d;
struct phv_ p;

%%
       .param BRQ_BASE
        .align
esp_ipv4_tunnel_n2h_allocate_barco_req_pindex:
    phvwri p.app_header_table1_valid, 0
   
    and r2, d.barco_pi, IPSEC_BARCO_RING_INDEX_MASK
    sll r2, r2, BRQ_RING_ENTRY_SIZE_SHIFT
    addui r2, r2, hiword(BRQ_BASE)
    addi r2, r2, loword(BRQ_BASE)
    phvwr p.ipsec_to_stage2_barco_req_addr, r2
    phvwr p.ipsec_to_stage3_barco_req_addr, r2
    add r1, d.barco_pi, 1
    and r1, r1, IPSEC_BARCO_RING_INDEX_MASK
    tblwr d.barco_pi, r1
    phvwr p.barco_dbell_pi, r1.wx
    nop.e
    nop 
