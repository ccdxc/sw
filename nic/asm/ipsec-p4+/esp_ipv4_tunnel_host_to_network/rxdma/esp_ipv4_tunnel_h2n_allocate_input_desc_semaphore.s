#include "ingress.h"
#include "INGRESS_p.h"
#include "ipsec_asm_defines.h"

struct rx_table_s1_t0_k k;
struct rx_table_s1_t0_allocate_input_desc_semaphore_d d;
struct phv_ p;

%%
        .param          esp_ipv4_tunnel_h2n_allocate_input_desc_index 
        .param          esp_ipv4_tunnel_h2n_allocate_output_desc_index
        .param          IPSEC_RNMPR_TABLE_BASE
        .param          IPSEC_TNMPR_TABLE_BASE
        .param          IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align

esp_ipv4_tunnel_h2n_allocate_input_desc_semaphore:
    phvwri p.p4_rxdma_intr_dma_cmd_ptr, H2N_RXDMA_IPSEC_DMA_COMMANDS_OFFSET
    and r1, d.{in_desc_ring_index}.wx, IPSEC_DESC_RING_INDEX_MASK
    sll r1, r1, IPSEC_DESC_RING_ENTRY_SHIFT_SIZE 

    addui r2, r1, hiword(IPSEC_RNMPR_TABLE_BASE)
    addi r2, r2, loword(IPSEC_RNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_allocate_input_desc_index, r2, TABLE_SIZE_64_BITS)

    addui r4, r1, hiword(IPSEC_TNMPR_TABLE_BASE)
    addi r4, r4, loword(IPSEC_TNMPR_TABLE_BASE)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_EN, esp_ipv4_tunnel_h2n_allocate_output_desc_index, r4, TABLE_SIZE_64_BITS)
    seq c1, d.full, 1
    bcf [c1], esp_ipv4_tunnel_h2n_desc_ring_full
    nop
    nop.e
    nop 

esp_ipv4_tunnel_h2n_desc_ring_full:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_DESC_EXHAUST_OFFSET, 1)
    phvwri.e p.ipsec_to_stage4_flags, IPSEC_N2H_GLOBAL_FLAGS
    nop

