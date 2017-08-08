#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t3_k k;
struct rx_table_s1_t3_allocate_output_page_semaphore_d d;
struct phv_ p;


allocate_input_page_semaphore:
    phvwri p.p42p4plus_hdr_table3_valid, 0
    sll r1, d.out_page_index, DESC_SHIFT_WIDTH 
    addi r1, r1, OUT_PAGE_ADDR_BASE
    phvwr p.t3_s2s_out_page_addr, r1
    nop.e 

