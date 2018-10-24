#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_h2n_allocate_input_desc_index 
        .param          esp_ipv4_tunnel_h2n_allocate_input_page_index
        .param          esp_ipv4_tunnel_h2n_allocate_output_desc_index
        .param          esp_ipv4_tunnel_h2n_allocate_output_page_index
        .param          IPSEC_RNMDR_TABLE_BASE
        .param          IPSEC_RNMPR_TABLE_BASE
        .param          IPSEC_TNMDR_TABLE_BASE
        .param          IPSEC_TNMPR_TABLE_BASE
        .align

esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore:
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 15 
    and r1, d.{in_desc_ring_index}.dx, IPSEC_DESC_RING_INDEX_MASK
    sll r1, r1, 3
    addui r2, r1, hiword(IPSEC_RNMPR_TABLE_BASE)
    addi r2, r2, loword(IPSEC_RNMPR_TABLE_BASE)
    phvwr  p.common_te2_phv_table_addr, r2
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_input_page_index[33:6]
    phvwri p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, 11

    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_input_desc_index[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 11 
    addui r3, r1, hiword(IPSEC_RNMDR_TABLE_BASE)
    addi r3, r3, loword(IPSEC_RNMDR_TABLE_BASE)
    phvwr p.common_te0_phv_table_addr, r3


    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_output_desc_index[33:6]
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 11
    addui r4, r1, hiword(IPSEC_TNMDR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_TNMDR_TABLE_BASE)
    phvwr p.common_te1_phv_table_addr, r4

    phvwri p.{common_te3_phv_table_lock_en...common_te3_phv_table_raw_table_size}, ((1 << 3) | 3)
    phvwri p.common_te3_phv_table_pc, esp_ipv4_tunnel_h2n_allocate_output_page_index[33:6]
    addui r5, r1, hiword(IPSEC_TNMPR_TABLE_BASE)
    addi r5, r5, loword(IPSEC_TNMPR_TABLE_BASE)
    phvwr.e p.common_te3_phv_table_addr, r5
    nop 

