#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t1_k k;
struct rx_table_s2_t1_allocate_output_desc_index_d d;
struct phv_ p;

%%
        .param update_output_desc_aol
        .align

allocate_output_desc_index:
    phvwri p.p42p4plus_hdr_table1_valid, 1
    addi r2, r0, update_output_desc_aol
    srl r2, r2, 6 
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 4
    phvwri p.common_te1_phv_table_lock_en, 0
    sll r1, d.out_desc_index, DESC_SHIFT_WIDTH 
    addi r1, r1, OUT_DESC_ADDR_BASE
    phvwr p.common_te1_phv_table_addr, r1
    phvwr p.t1_s2s_out_desc_addr, r1
    nop.e 

