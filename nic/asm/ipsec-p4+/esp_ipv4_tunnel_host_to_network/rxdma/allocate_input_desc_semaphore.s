#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          allocate_input_desc_index 
        .align

allocate_input_desc_semaphore:
    phvwri p.p42p4plus_hdr_table0_valid, 1
    phvwr p.common_te0_phv_table_pc, allocate_input_desc_index 
    phvwri p.common_te0_phv_table_raw_table_size, 3
    phvwri p.common_te0_phv_table_lock_en, 0
    sll r1, d.in_desc_ring_index, 3 
    addi r1, r1, IN_DESC_RING_BASE
    phvwr p.common_te0_phv_table_addr, r1
     

