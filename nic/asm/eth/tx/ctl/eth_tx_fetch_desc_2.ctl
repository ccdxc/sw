
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

p = {
    app_header_table0_valid = 0;
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
    p4_txdma_intr_qtype = 2;
    p4_txdma_intr_qid = 0;
};

d = {
    enable = 0x1;
    p_index0 = 0x0200;
    c_index0 = 0x0;
    ring_size = 0x0a00;
    ring_base = 0x8000000000000000;
    cq_ring_base = 0x8000000000010000;
};
