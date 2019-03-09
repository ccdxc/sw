#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t2_k k;
struct rx_table_s2_t2_esp_v4_tunnel_n2h_allocate_input_page_index_d d;
struct phv_ p;

%%
        
.align
.param esp_v4_tunnel_n2h_rxdma_load_part2

esp_ipv4_tunnel_n2h_allocate_input_page_index:
    phvwr p.t0_s2s_in_page_addr, d.in_page_index 
    phvwr p.ipsec_int_header_in_page, d.in_page_index
    add r1, k.ipsec_to_stage2_ipsec_cb_addr, 64
    phvwr p.common_te2_phv_table_addr, r1 
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN, esp_v4_tunnel_n2h_rxdma_load_part2, r1, TABLE_SIZE_64_BITS)
    nop.e 
    nop

