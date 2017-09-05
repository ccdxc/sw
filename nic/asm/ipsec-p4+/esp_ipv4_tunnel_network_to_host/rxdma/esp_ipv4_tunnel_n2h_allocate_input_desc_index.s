#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t0_k k;
struct rx_table_s2_t0_esp_v4_tunnel_n2h_allocate_input_desc_index_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_update_input_desc_aol
        .align

esp_ipv4_tunnel_n2h_allocate_input_desc_index:
    phvwri p.p42p4plus_hdr_table0_valid, 1
    addi r2, r0, esp_ipv4_tunnel_n2h_update_input_desc_aol
    srl r2, r2, 6
    phvwr p.common_te0_phv_table_pc, r2 
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_lock_en, 1
    //sll r1, d.in_desc_index, DESC_SHIFT_WIDTH
    //addi r1, r1, IN_DESC_ADDR_BASE
    phvwr p.common_te0_phv_table_addr, d.in_desc_index 
    phvwr.e p.t0_s2s_in_desc_addr, r1
     

