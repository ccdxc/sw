#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_allocate_input_desc_semaphore_d d;
struct phv_ p;


allocate_input_desc_semaphore:
    phvwri p.p42p4plus_hdr_table0_valid, 1
    phvwr p.common_te0_phv_table0_pc, update_input_desc_aol 
    phvwri p.common_te0_phv_table0_raw_table_size, 4
    phvwri p.common_te0_phv_table0_lock_en, 0
    sll r1, d.in_desc_index, DESC_SHIFT_WIDTH 
    addi r1, r1, IN_DESC_ADDR_BASE
    phvwr p.common_te0_phv_table0_addr, r1
    phvwr p.t0_s2s_in_desc_addr, r1
    phvwr.e p.ipsec_global_in_desc_addr, r1
     

