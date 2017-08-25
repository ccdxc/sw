#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_defines.h"

struct rx_table_s2_t0_k k;
struct rx_table_s2_t0_esp_v4_tunnel_n2h_allocate_input_desc_index_d d;
struct phv_ p;

%%
        .param          update_input_desc_aol
        .align

esp_ipv4_tunnel_n2h_allocate_input_desc_index:
    phvwri p.p42p4plus_hdr_table0_valid, 1
    phvwr p.common_te0_phv_table_pc, update_input_desc_aol 
    phvwri p.common_te0_phv_table_raw_table_size, 6
    phvwri p.common_te0_phv_table_lock_en, 1
    sll r1, d.in_desc_index, DESC_SHIFT_WIDTH
    addi r1, r1, IN_DESC_ADDR_BASE
    phvwr p.common_te0_phv_table_addr, r1
    phvwr.e p.t0_s2s_in_desc_addr, r1
     

