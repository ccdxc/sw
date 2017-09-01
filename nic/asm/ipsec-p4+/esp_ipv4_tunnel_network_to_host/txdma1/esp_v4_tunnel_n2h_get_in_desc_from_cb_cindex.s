#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_asm_defines.h"
#include "capri-macros.h"

struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex_d d;
struct phv_ p;

%%
        .param esp_v4_tunnel_n2h_txdma1_dequeue_head_desc 
        .param esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header
        .align
esp_v4_tunnel_n2h_get_in_desc_from_cb_cindex:
    phvwri p.app_header_table2_valid, 1
    phvwri p.common_te2_phv_table_lock_en, 1
    addi r2, r0, esp_v4_tunnel_n2h_txdma1_dequeue_head_desc
    srl r2, r2, 6 
    phvwr p.common_te2_phv_table_pc, r2 
    phvwri p.common_te2_phv_table_raw_table_size, 6
    add r1, r0, d.in_desc_addr
    addi r1, r1, 64
    phvwr p.common_te2_phv_table_addr, r1

    phvwri p.app_header_table1_valid, 1
    phvwri p.common_te1_phv_table_lock_en, 1
    addi r2, r0, esp_v4_tunnel_n2h_txdma1_load_head_desc_int_header 
    srl r2, r2, 6
    phvwr p.common_te1_phv_table_pc, r2 
    phvwri p.common_te1_phv_table_raw_table_size, 6
    phvwr p.common_te1_phv_table_addr, d.in_desc_addr

