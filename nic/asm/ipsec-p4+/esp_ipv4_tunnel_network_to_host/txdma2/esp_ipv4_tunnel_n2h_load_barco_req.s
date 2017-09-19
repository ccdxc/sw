#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_esp_v4_tunnel_n2h_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param esp_ipv4_tunnel_n2h_txdma2_load_in_desc 
        .param esp_ipv4_tunnel_n2h_txdma2_load_out_desc
        .param esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int
        .align
esp_ipv4_tunnel_n2h_load_barco_req:
    phvwr p.txdma2_global_in_desc_addr, d.brq_in_addr

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6 
    phvwri p.common_te0_phv_table_pc, esp_ipv4_tunnel_n2h_txdma2_load_in_desc[33:6] 
    add r1, r0, d.brq_in_addr
    addi r1, r1, 64
    phvwr  p.common_te0_phv_table_addr, r1 

    phvwri p.app_header_table1_valid, 1
    phvwri p.common_te1_phv_table_lock_en, 1
    phvwri p.common_te1_phv_table_raw_table_size, 6
    phvwri p.common_te1_phv_table_pc, esp_ipv4_tunnel_n2h_txdma2_load_out_desc[33:6] 
    add r1, r0, d.brq_in_addr
    addi r1, r1, 64
    phvwr  p.common_te1_phv_table_addr, d.brq_out_addr

    phvwri p.app_header_table2_valid, 1
    phvwri p.common_te2_phv_table_lock_en, 1
    phvwri p.common_te2_phv_table_raw_table_size, 6 
    phvwri p.common_te2_phv_table_pc, esp_ipv4_tunnel_n2h_txdma2_load_ipsec_int[33:6] 
    phvwr  p.common_te0_phv_table_addr, d.brq_in_addr 
    nop.e
    nop
