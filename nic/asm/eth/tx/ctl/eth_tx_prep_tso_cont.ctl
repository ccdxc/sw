
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s1_t0_k.h"

#include "../../asm/eth/tx/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k_ k;
struct tx_table_s1_t0_eth_tx_prep_d d;


p = {
    app_header_table0_valid = 1;
    app_header_table1_valid = 0;
    app_header_table2_valid = 0;
    app_header_table3_valid = 0;
    common_te0_phv_table_lock_en = 0;
    common_te0_phv_table_pc = 0;
    common_te0_phv_table_addr = 0;
    common_te0_phv_table_raw_table_size = 0;
    p4_intr_global_drop = 0;
};

k = {
    eth_tx_t0_s2s_num_todo = 0x1;
    eth_tx_global_dma_cur_index = 0x20;
    eth_tx_to_s1_qstate_addr = 0x3ffffffff;
};

d = {
    addr_lo0 = 0xf6f5f4f3f2f1;
    addr_hi0 = 0x7;
    len0 = 0x6e;
    opcode0 = TXQ_DESC_OPCODE_TSO;
    csum_l3_or_sot0 = 0;
    csum_l4_or_eot0 = 0;
    num_sg_elems0 = 1;
    mss_or_csumoff_lo0 = 0xff;
    mss_or_csumoff_hi0 = 0x3f;
};
