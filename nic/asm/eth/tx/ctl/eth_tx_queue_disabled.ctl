
#include "ingress.h"
#include "INGRESS_p.h"

/*
 * TESTCASE:
 *      queue disabled
 *
 * ASSERTIONS:
 *      (Table)
 *      ci0 == ci0
 *      pi1 == pi0
 *      phv_drop_cnt == phv_drop_cnt + 1
 *
 *      (PHV)
 *      table0_valid == 0
 *      intr_global_drop == 1
 */

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

p = {
    app_header_table0_valid = 1;
    app_header_table1_valid = 1;
    app_header_table2_valid = 1;
    app_header_table3_valid = 1;
    p4_intr_global_drop = 0;
};

k = {
    p4_txdma_intr_qtype = 1;
    p4_txdma_intr_qid = 0;
};

d = {
    enable = 0x0;
    p_index0 = 0x0100;
    c_index0 = 0x0;
    ring_size = 0x0a00;
    ring_base = 0x8000000000000000;
    cq_ring_base = 0x8000000000010000;
    sg_ring_base = 0x8000000000030000;
#if 0
    phv_drop_cnt = 0x0;
#endif
};
