#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t3_k k;
struct rx_table_s2_t3_allocate_output_page_index_d d;
struct phv_ p;

%%
        .align

esp_ipv4_tunnel_h2n_allocate_output_page_index:
    phvwri p.app_header_table3_valid, 0
    //sll r1, d.out_page_index, DESC_SHIFT_WIDTH 
    //addi r1, r1, OUT_PAGE_ADDR_BASE
    phvwr p.t1_s2s_out_page_addr, d.out_page_index 
    phvwr p.ipsec_int_header_out_page, d.out_page_index 
    phvwr p.barco_desc_out_A0_addr, d.out_page_index
    phvwri p.barco_desc_out_O0, 0 
    phvwri p.barco_desc_out_L0, 0 
    nop.e 
    nop
