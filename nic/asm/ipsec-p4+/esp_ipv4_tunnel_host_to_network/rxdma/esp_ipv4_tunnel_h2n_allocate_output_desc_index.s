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
    phvwr p.ipsec_int_header_out_desc, d.out_desc_index 
    phvwr p.t1_s2s_out_desc_addr, d.out_desc_index
    add r5, r0, d.out_desc_index
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_update_output_desc_aol, r5, TABLE_SIZE_128_BITS)
    add r6, d.out_desc_index, IPSEC_PAGE_OFFSET 
    phvwr p.ipsec_int_header_out_page, r6 
    phvwr p.barco_desc_out_A0_addr, r6.dx
    phvwr.e p.ipsec_to_stage4_out_desc_addr, d.out_desc_index
    nop
