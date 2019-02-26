#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_esp_v4_tunnel_n2h_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_n2h_allocate_input_desc_index 
        .param          esp_ipv4_tunnel_n2h_allocate_output_desc_index 
        .param          IPSEC_RNMPR_TABLE_BASE
        .param          IPSEC_TNMPR_TABLE_BASE
        .param          IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
        .align

esp_ipv4_tunnel_n2h_allocate_input_desc_semaphore:
    seq c1, d.full, 1
    bcf [c1], esp_ipv4_tunnel_n2h_desc_ring_full
    nop

    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 3 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_input_desc_index[33:6] 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 11 

    and r1, d.{in_desc_ring_index}.wx, IPSEC_DESC_RING_INDEX_MASK 
    sll r1, r1, IPSEC_DESC_RING_ENTRY_SHIFT_SIZE 

    addui r2, r1, hiword(IPSEC_RNMPR_TABLE_BASE)
    addi r2, r2, loword(IPSEC_RNMPR_TABLE_BASE)
    phvwr p.common_te0_phv_table_addr, r2

    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_n2h_allocate_output_desc_index[33:6]
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 11
    addui r4, r1, hiword(IPSEC_TNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_TNMPR_TABLE_BASE)
    phvwr.e p.common_te1_phv_table_addr, r4
    nop
     
esp_ipv4_tunnel_n2h_desc_ring_full:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_DESC_RING_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
