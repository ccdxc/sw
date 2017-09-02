#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t3_k k;
struct rx_table_s2_t3_allocate_output_page_index_d d;
struct phv_ p;

%%
        .align

allocate_output_page_index:
    phvwri p.p42p4plus_hdr_table3_valid, 0
    sll r1, d.out_page_index, DESC_SHIFT_WIDTH 
    addi r1, r1, OUT_PAGE_ADDR_BASE
    phvwr p.t1_s2s_out_page_addr, r1
    phvwr p.ipsec_int_header_out_page, r1
    nop.e 

