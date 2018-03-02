#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_h2n_allocate_input_desc_index 
        .param          RNMDR_TABLE_BASE
        .align

esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore:
    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_input_desc_index[33:6] 
    phvwri p.common_te0_phv_table_raw_table_size, 3
    sll r1, d.{in_desc_ring_index}.dx, 3 
    addui r1, r1, hiword(RNMDR_TABLE_BASE)
    addi r1, r1, loword(RNMDR_TABLE_BASE)
    phvwr.f p.common_te0_phv_table_addr, r1
    nop.e
    nop 

