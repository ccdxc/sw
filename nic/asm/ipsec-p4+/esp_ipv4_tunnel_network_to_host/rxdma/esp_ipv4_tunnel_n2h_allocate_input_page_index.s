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
    phvwri p.common_te2_phv_table_pc, esp_v4_tunnel_n2h_rxdma_load_part2[33:6]
    phvwri p.common_te2_phv_table_raw_table_size, 4
    phvwri p.common_te2_phv_table_lock_en, 1
    add r1, r0, k.ipsec_to_stage2_ipsec_cb_addr
    addi r1, r1, 64
    phvwr p.common_te2_phv_table_addr, r1 
    nop.e 
    nop

