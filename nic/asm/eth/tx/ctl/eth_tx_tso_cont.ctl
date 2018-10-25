
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t2_k.h"

#include "../../asm/eth/tx/defines.h"

struct phv_ p;
struct tx_table_s3_t2_k_ k;
struct tx_table_s3_t2_eth_tx_tso_d d;


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
    eth_tx_global_dma_cur_index = 0x20;
    eth_tx_global_num_sg_elems = 0x1;
    eth_tx_global_tso_sot = 0x1;
    eth_tx_global_sg_desc_addr = 0x30000000080;
    eth_tx_t2_s2s_tso_hdr_addr = 0x1efbeaddebaba;
    eth_tx_t2_s2s_tso_hdr_len = 0x3a;
    eth_tx_to_s3_addr_lo0 = 0xbabadeadbeef;
    eth_tx_to_s3_rsvd0 = 0;
    eth_tx_to_s3_addr_hi0 = 0x1;
    eth_tx_to_s3_opcode0 = 0x3;
    eth_tx_to_s3_num_sg_elems0 = 0x1;
    eth_tx_to_s3_len0 = 0x6e;
    eth_tx_to_s3_vlan_tci0 = 0;
    eth_tx_to_s3_hdr_len_lo0 = 0x3a;
    eth_tx_to_s3_encap0 = 0;
    eth_tx_to_s3_cq_entry0 = 0;
    eth_tx_to_s3_vlan_insert0 = 0;
    eth_tx_to_s3_rsvd20 = 0;
    eth_tx_to_s3_hdr_len_hi0 = 0;
    eth_tx_to_s3_mss_or_csumoff_lo0 = 0;
    eth_tx_to_s3_csum_l4_or_eot0 = 0;
    eth_tx_to_s3_csum_l3_or_sot0 = 0x1;
    eth_tx_to_s3_mss_or_csumoff_hi0 = 0;
};

d = {
    addr_lo0 = 0xbabababa;
    addr_hi0 = 0x1;
    len0 = 66;
};
