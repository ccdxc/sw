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

    add r1, k.ipsec_to_stage2_ipsec_cb_addr, IPSEC_SCRATCH_OFFSET 
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_EN, esp_v4_tunnel_n2h_rxdma_load_part2, r1, TABLE_SIZE_128_BITS)
    add r2, r0, d.in_desc_index
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, esp_ipv4_tunnel_n2h_update_input_desc_aol)
    add r6, d.in_desc_index, IPSEC_PAGE_OFFSET
    phvwr p.t0_s2s_in_page_addr, r6
    phvwr p.ipsec_int_header_in_page, r6
    phvwr.e p.t0_s2s_in_desc_addr, d.in_desc_index
    nop
     

