#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t0_k k;
struct rx_table_s2_t0_esp_v4_tunnel_n2h_allocate_input_desc_index_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_update_input_desc_aol
        .param          esp_v4_tunnel_n2h_rxdma_load_part2
        .align

esp_ipv4_tunnel_n2h_allocate_input_desc_index:

    add r1, k.ipsec_to_stage2_ipsec_cb_addr, 64
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN, esp_v4_tunnel_n2h_rxdma_load_part2, r1, TABLE_SIZE_128_BITS)

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_update_input_desc_aol[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14
    phvwr p.common_te0_phv_table_addr, d.in_desc_index 
    add r6, d.in_desc_index, IPSEC_PAGE_OFFSET
    phvwr p.t0_s2s_in_page_addr, r6
    phvwr p.ipsec_int_header_in_page, r6
    phvwr.e p.t0_s2s_in_desc_addr, d.in_desc_index
    nop
     

