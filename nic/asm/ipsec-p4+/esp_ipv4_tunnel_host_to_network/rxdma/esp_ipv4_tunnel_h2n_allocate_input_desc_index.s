#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s2_t0_k k;
struct rx_table_s2_t0_allocate_input_desc_index_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_h2n_update_input_desc_aol
        .param          esp_ipv4_tunnel_h2n_update_input_desc_aol2
        .align

esp_ipv4_tunnel_h2n_allocate_input_desc_index:
    add r5, r0, d.in_desc_index
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_update_input_desc_aol, r5, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(3, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_update_input_desc_aol2, r5, TABLE_SIZE_512_BITS)
    phvwr.e p.ipsec_global_in_desc_addr, d.in_desc_index 
    nop  

