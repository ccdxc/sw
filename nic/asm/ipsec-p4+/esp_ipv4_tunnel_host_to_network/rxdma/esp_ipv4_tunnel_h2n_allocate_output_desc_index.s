#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t1_k k;
struct rx_table_s2_t1_allocate_output_desc_index_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_update_output_desc_aol
        .align

esp_ipv4_tunnel_h2n_allocate_output_desc_index:
    phvwri p.app_header_table1_valid, 1
    addi r2, r0, esp_ipv4_tunnel_h2n_update_output_desc_aol 
    srl r2, r2, 6 
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 4
    phvwri p.common_te1_phv_table_lock_en, 0
    //sll r1, d.out_desc_index, DESC_SHIFT_WIDTH 
    //addi r1, r1, OUT_DESC_ADDR_BASE
    phvwr p.common_te1_phv_table_addr, d.out_desc_index 
    phvwr p.ipsec_int_header_out_desc, d.out_desc_index 
    phvwr p.t1_s2s_out_desc_addr, d.out_desc_index 
    phvwr p.ipsec_to_stage4_out_desc_addr, d.out_desc_index
    nop.e 
    nop
