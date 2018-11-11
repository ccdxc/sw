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
//    add r1, k.ipsec_to_stage3_ipsec_cb_addr, IPSEC_IP_HDR_OFFSET
 //   blti  r1, CAPRI_HBM_BASE, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc_illegal_dma_cb
    phvwr p.t0_s2s_in_page_addr, d.{addr0}.dx 
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 3
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 6 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet[33:6] 
    phvwr  p.common_te0_phv_table_addr, k.txdma2_global_in_desc_addr
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 6 
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2[33:6] 
    phvwr.e  p.common_te1_phv_table_addr, k.txdma2_global_in_desc_addr
    //phvwr.e  p.eth_hdr_dma_cmd_addr, r1
    nop


esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc_illegal_dma_cb:
    addi r7, r0, IPSEC_GLOBAL_BAD_DMA_COUNTER_BASE_H2N
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, H2N_CB_RING_OFFSET, 1)
    phvwri p.p4_intr_global_drop, 1
    phvwri.e p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
 
