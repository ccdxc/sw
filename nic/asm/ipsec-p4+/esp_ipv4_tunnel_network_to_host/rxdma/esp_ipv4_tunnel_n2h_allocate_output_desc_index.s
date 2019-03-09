#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t1_k k;
struct rx_table_s2_t1_esp_v4_tunnel_n2h_allocate_output_desc_index_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_update_output_desc_aol
        .align

esp_ipv4_tunnel_n2h_allocate_output_desc_index:
    add r2, r0, d.out_desc_index
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, esp_ipv4_tunnel_n2h_update_output_desc_aol, r2, TABLE_SIZE_512_BITS)
    phvwr p.t1_s2s_out_desc_addr, d.out_desc_index
    add r6, d.out_desc_index, IPSEC_PAGE_OFFSET
    phvwr p.t1_s2s_out_page_addr, r6
    phvwr p.ipsec_int_header_out_page, r6
    phvwr p.ipsec_to_stage4_out_desc_addr, d.out_desc_index
    phvwr.e p.ipsec_int_header_out_desc, d.out_desc_index
    nop 

