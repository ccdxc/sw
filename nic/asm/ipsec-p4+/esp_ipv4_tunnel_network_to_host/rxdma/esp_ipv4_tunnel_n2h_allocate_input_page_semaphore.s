#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t2_k k;
struct rx_table_s1_t2_esp_v4_tunnel_n2h_allocate_input_page_semaphore_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_allocate_input_page_index
        .param RNMPR_TABLE_BASE
        .align

esp_ipv4_tunnel_n2h_allocate_input_page_semaphore:
    phvwri p.app_header_table2_valid, 1
    sll r1, d.in_page_ring_index, 3 
    addi r1, r1, RNMPR_TABLE_BASE 
    phvwr  p.common_te2_phv_table_addr, r1
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_input_page_index[33:6] 
    phvwri p.common_te2_phv_table_lock_en, 0
    phvwri p.common_te2_phv_table_raw_table_size, 3 
    nop.e 
    nop
