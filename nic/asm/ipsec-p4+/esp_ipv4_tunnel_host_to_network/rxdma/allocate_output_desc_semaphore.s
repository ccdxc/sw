#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t1_k k;
struct rx_table_s1_t1_allocate_output_desc_semaphore_d d;
struct phv_ p;

%%
        .param          allocate_output_desc_index 
        .align

allocate_output_desc_semaphore:
    phvwri p.p42p4plus_hdr_table1_valid, 1
    phvwri p.common_te1_phv_table_pc, allocate_output_desc_index 
    phvwri p.common_te1_phv_table_raw_table_size, 3
    phvwri p.common_te1_phv_table_lock_en, 0
    sll r1, d.out_desc_ring_index, 3 
    addi r1, r1, OUT_DESC_RING_BASE 
    phvwr p.common_te1_phv_table_addr, r1
    nop.e 

