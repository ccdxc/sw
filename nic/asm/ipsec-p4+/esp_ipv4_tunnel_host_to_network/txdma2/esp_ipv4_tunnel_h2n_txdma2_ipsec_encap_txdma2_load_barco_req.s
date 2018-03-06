#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_ipsec_encap_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc 
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_out_desc
        .param esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int
        .align
esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_barco_req:
    phvwr p.txdma2_global_in_desc_addr, d.{input_list_address}.dx
    phvwri p.{app_header_table0_valid...app_header_table3_valid}, 14 
    phvwri p.{common_te0_phv_table_lock_en...common_te0_phv_table_raw_table_size}, 14
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_in_desc[33:6] 
    phvwr  p.common_te0_phv_table_addr, d.{input_list_address}.dx 
    phvwri p.{common_te1_phv_table_lock_en...common_te1_phv_table_raw_table_size}, 14
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_out_desc[33:6] 
    phvwr  p.common_te1_phv_table_addr, d.{output_list_address}.dx
    phvwri p.{common_te2_phv_table_lock_en...common_te2_phv_table_raw_table_size}, 14
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_h2n_txdma2_ipsec_encap_txdma2_load_ipsec_int[33:6] 
    sub r2, d.{input_list_address}.dx, 64 
    phvwr  p.common_te2_phv_table_addr, r2 
    nop.e
    nop
