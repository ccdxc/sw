#include "INGRESS_p.h"
#include "ingress.h"
#include "ipsec_defines.h"

struct tx_table_s2_t0_k k;
struct tx_table_s2_t0_ipsec_encap_txdma2_load_barco_req_d d;
struct phv_ p;

%%
        .param ipsec_encap_txdma2_load_in_desc 
        .param ipsec_encap_txdma2_load_out_desc
        .param ipsec_encap_txdma2_load_ipsec_int
        .align
ipsec_encap_txdma2_load_barco_req:
    phvwr p.txdma2_global_in_desc_addr, d.brq_in_addr

    phvwri p.app_header_table0_valid, 1
    phvwri p.common_te0_phv_table_lock_en, 1
    phvwri p.common_te0_phv_table_raw_table_size, 6 
    phvwri p.common_te0_phv_table_pc, ipsec_encap_txdma2_load_in_desc 
    add r1, r0, d.brq_in_addr
    addi r1, r1, 64
    phvwr  p.common_te0_phv_table_addr, r1 

    phvwri p.app_header_table1_valid, 1
    phvwri p.common_te1_phv_table_lock_en, 1
    phvwri p.common_te1_phv_table_raw_table_size, 6 
    phvwri p.common_te1_phv_table_pc, ipsec_encap_txdma2_load_out_desc 
    add r1, r0, d.brq_in_addr
    addi r1, r1, 64
    phvwr  p.common_te1_phv_table_addr, d.brq_out_addr

    phvwri p.app_header_table2_valid, 1
    phvwri p.common_te2_phv_table_lock_en, 1
    phvwri p.common_te2_phv_table_raw_table_size, 6 
    phvwri p.common_te2_phv_table_pc, ipsec_encap_txdma2_load_ipsec_int 
    phvwr  p.common_te0_phv_table_addr, d.brq_in_addr 
    nop.e

