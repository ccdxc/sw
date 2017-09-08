#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t2_k k;
struct rx_table_s2_t2_allocate_input_page_index_d d;
struct phv_ p;

%%
        

esp_ipv4_tunnel_h2n_allocate_input_page_index:
    phvwri p.app_header_table2_valid, 0
    //sll r1, d.in_page_index, DESC_SHIFT_WIDTH 
    //addi r1, r1, IN_PAGE_ADDR_BASE
    phvwr p.t0_s2s_in_page_addr, d.in_page_index
    phvwr p.ipsec_int_header_in_page, r1
    nop.e 
    nop
