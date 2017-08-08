#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t2_k k;
struct rx_table_s1_t2_allocate_input_page_semaphore_d d;
struct phv_ p;


allocate_input_page_semaphore:
    phvwri p.p42p4plus_hdr_table2_valid, 0
    sll r1, d.in_page_index, DESC_SHIFT_WIDTH 
    addi r1, r1, IN_PAGE_ADDR_BASE
    phvwr p.t2_s2s_in_page_addr, r1
    nop.e 

