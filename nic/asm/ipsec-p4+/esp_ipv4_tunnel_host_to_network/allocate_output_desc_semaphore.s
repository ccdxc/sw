#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s1_t1_k k;
struct rx_table_s1_t1_allocate_output_desc_semaphore_d d;
struct phv_ p;


allocate_output_desc_semaphore:
    phvwri p.p42p4plus_hdr_table1_valid, 1
    phvwr p.common_te1_phv_table_pc, update_output_desc_aol 
    phvwri p.common_te1_phv_table_raw_table_size, 4
    phvwri p.common_te1_phv_table_lock_en, 0
    sll r1, d.out_desc_index, DESC_SHIFT_WIDTH 
    addi r1, r1, OUT_DESC_ADDR_BASE
    phvwri p.common_te1_phv_table0_addr, r1
    phvwr p.t1_s2s_out_desc_addr, r1
    // need to come back for below line once s2s vs to_stage contention is fixed.
    //phvwr.e p.ipsec_global_in_desc_addr, r1
    nop.e 

