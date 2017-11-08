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
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_update_input_desc_aol[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwr p.common_te0_phv_table_addr, d.in_desc_index 
    phvwr p.t0_s2s_in_desc_addr, d.in_desc_index
    nop.e
    nop
     

