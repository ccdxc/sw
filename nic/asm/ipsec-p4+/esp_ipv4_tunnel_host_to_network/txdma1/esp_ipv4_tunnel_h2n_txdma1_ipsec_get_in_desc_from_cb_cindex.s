#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_get_in_desc_from_cb_cindex_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header 
        .param esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2 
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma1_ipsec_get_in_desc_from_cb_cindex:
    addi r1, r0, IPSEC_DESC_FULL_DESC_ADDR
    seq c1, d.in_desc_addr, r1
    bcf [c1], esp_v4_tunnel_h2n_get_in_desc_from_cb_cindex_abort_txdma1
    nop
    phvwr p.txdma1_global_in_desc_addr, d.in_desc_addr
    add r5, r0, d.in_desc_addr
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header, r5, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2, r5, TABLE_SIZE_512_BITS)
    nop.e
    nop

esp_v4_tunnel_h2n_get_in_desc_from_cb_cindex_abort_txdma1:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_TXDMA1_BAD_INDESC_FROM_CB, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop
    
