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

    and r1, d.{in_desc_ring_index}.wx, IPSEC_DESC_RING_INDEX_MASK 
    sll r1, r1, IPSEC_DESC_RING_ENTRY_SHIFT_SIZE 

    addui r2, r1, hiword(IPSEC_RNMPR_TABLE_BASE)
    addi r2, r2, loword(IPSEC_RNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_allocate_input_desc_index, r2, TABLE_SIZE_64_BITS)

    addui r4, r1, hiword(IPSEC_TNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_TNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_n2h_allocate_output_desc_index, r4, TABLE_SIZE_64_BITS)

    bcf [c1], esp_ipv4_tunnel_n2h_desc_ring_full
    nop
    nop.e
    nop
     
esp_ipv4_tunnel_n2h_desc_ring_full:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_N2H
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, N2H_DESC_RING_OFFSET, 1)
    phvwri.e p.ipsec_global_flags, IPSEC_N2H_GLOBAL_FLAGS
    nop
