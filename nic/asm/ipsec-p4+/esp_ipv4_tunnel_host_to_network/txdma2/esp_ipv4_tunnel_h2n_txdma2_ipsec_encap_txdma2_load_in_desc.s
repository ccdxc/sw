#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s3_t0_k k;
struct tx_table_s3_t0_ipsec_encap_txdma2_load_in_desc_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet 
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2 
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc:
    phvwr p.t0_s2s_in_page_addr, d.{addr0}.dx 
    phvwri p.{app_header_table0_valid...app_header_table1_valid}, 3
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet[33:6] 
    phvwr  p.common_te0_phv_table_addr, k.txdma2_global_in_desc_addr
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 14
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_build_encap_packet2[33:6] 
    phvwr  p.common_te1_phv_table_addr, k.txdma2_global_in_desc_addr

    add r1, k.ipsec_to_stage3_ipsec_cb_addr, IPSEC_IP_HDR_OFFSET
    phvwr.f  p.eth_hdr_dma_cmd_addr, r1


    nop.e
    nop
 
