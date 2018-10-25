
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s2_t0_k.h"

#include "../../asm/eth/tx/defines.h"


struct phv_ p;
struct tx_table_s2_t0_k_ k;
struct tx_table_s2_t0_eth_tx_commit_d d;


p = {
    app_header_table0_valid = 0;
    app_header_table1_valid = 0;
    app_header_table2_valid = 0;
    app_header_table3_valid = 0;
    common_te0_phv_table_lock_en = 0;
    common_te0_phv_table_pc = 0;
    common_te0_phv_table_addr = 0;
    common_te0_phv_table_raw_table_size = 0;
    common_te1_phv_table_lock_en = 0;
    common_te1_phv_table_pc = 0;
    common_te1_phv_table_addr = 0;
    common_te1_phv_table_raw_table_size = 0;
    common_te2_phv_table_lock_en = 0;
    common_te2_phv_table_pc = 0;
    common_te2_phv_table_addr = 0;
    common_te2_phv_table_raw_table_size = 0;
    p4_intr_global_drop = 0;
};

k = {
    eth_tx_t0_s2s_num_desc = 0x1;
    eth_tx_t0_s2s_do_tso = 0x1;
    eth_tx_t0_s2s_do_sg = 0x1;
    eth_tx_global_dma_cur_index = 0x20;
    eth_tx_global_num_sg_elems = 0x1;
    eth_tx_global_tso_sot = 0x0;
    eth_tx_global_tso_eot = 0x0;
    eth_tx_to_s2_tso_hdr_addr = 0x5a8; // mss
};

d = {
    enable = 0x1;
    p_index0 = 0x0100;
    c_index0 = 0x0;
    comp_index = 0x0;
    ring_size = 0x0a00;
    ring_base = 0x8000000000010000;
    cq_ring_base = 0x8000000000020000;
    sg_ring_base = 0x8000000000030000;
    tso_ipid_delta = 0x0;
    tso_seq_delta = 0x0;
    tso_hdr_addr = 0x1ffffffffffff;
    tso_hdr_len = 0xff;
};
