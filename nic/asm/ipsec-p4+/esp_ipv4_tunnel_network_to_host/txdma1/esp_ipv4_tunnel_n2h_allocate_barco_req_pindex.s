#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

struct tx_table_s1_t1_k k;
struct tx_table_s1_t1_esp_v4_tunnel_n2h_allocate_barco_req_pindex_d d;
struct phv_ p;

%%
       .param esp_v4_tunnel_n2h_txdma1_get_barco_req_index_ptr
        .align
esp_v4_tunnel_n2h_txdma1_allocate_barco_req_pindex:
    phvwri p.app_header_table0_valid, 1
    addi r2, r0, esp_v4_tunnel_n2h_txdma1_get_barco_req_index_ptr
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 3
    add r1, r0, d.barco_pindex
    sll r1, r1, BRQ_RING_ENTRY_SIZE_SHIFT 
    addi r1, r1, BRQ_REQ_RING_BASE_ADDR
    phvwr p.common_te0_phv_table_addr, r1 
    
