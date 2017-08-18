#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t2_k k;
struct rx_table_s1_t2_allocate_input_page_semaphore_d d;
struct phv_ p;

%%
        .param allocate_input_page_index
        .align

allocate_input_page_semaphore:
    phvwri p.p42p4plus_hdr_table2_valid, 1
    sll r1, d.in_page_ring_index, 3 
    addi r1, r1, IN_PAGE_RING_BASE
    phvwr  p.common_te2_phv_table_addr, r1
    phvwri p.common_te2_phv_table_pc, allocate_input_page_index
    phvwri p.common_te2_phv_table_lock_en, 0
    phvwri p.common_te2_phv_table_raw_table_size, 3 
    nop.e 

