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
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 3 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 6 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header[33:6] 
    phvwr p.common_te0_phv_table_addr, d.in_desc_addr
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 6 
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_txdma1_ipsec_encap_txdma_load_head_desc_int_header2[33:6] 
    phvwr.e p.common_te1_phv_table_addr, d.in_desc_addr
    nop

esp_v4_tunnel_h2n_get_in_desc_from_cb_cindex_abort_txdma1:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_ABORT_TXDMA1_DUMMY_OFFSET, 1)
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwri.e p.p4_intr_global_drop, 1
    nop
    
