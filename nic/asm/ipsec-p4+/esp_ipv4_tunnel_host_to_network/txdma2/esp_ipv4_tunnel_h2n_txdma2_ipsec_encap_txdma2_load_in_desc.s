#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_ipsec_encap_txdma2_load_in_desc_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet 
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2 
        .param IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc:
    phvwr p.t0_s2s_in_page_addr, d.{addr0}.dx 
    add r5, r0, k.txdma2_global_in_desc_addr 
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet, r5, TABLE_SIZE_512_BITS)
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2, r5, TABLE_SIZE_512_BITS)
    nop.e
    nop


esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc_illegal_dma_cb:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_CB_RING_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
 
