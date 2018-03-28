
#include "ingress.h"
#include "INGRESS_p.h"

/*
 * TESTCASE:
 *      first spurious doorbell (pi0 == ci0 && spurious_db_cnt == 0)
 *
 * ASSERTIONS:
 *      (Table)
 *      spurious_db_cnt == 1
 *      eval_db_cnt == eval_db_cnt + 1
 *      phv_drop_cnt == phv_drop_cnt +1
 *
 *      (PHV)
 *      table_valid == 0
 *      intr_global_drop == 1
 */

struct phv_ p;
struct tx_table_s0_t0_k k;
struct tx_table_s0_t0_eth_tx_fetch_desc_d d;

p = {
    app_header_table0_valid = 0;
    app_header_table1_valid = 0;
    app_header_table2_valid = 0;
    app_header_table3_valid = 0;
    p4_intr_global_drop = 0;
};

k = {
    p4_txdma_intr_qtype = 1;
    p4_txdma_intr_qid = 0;
};

d = {
    enable = 0x1;
    p_index0 = 0x0100;
    c_index0 = 0x0100;
    spurious_db_cnt = 0x0;
#if 0
    eval_db_cnt = 0x0;
    phv_drop_cnt = 0x0;
#endif
};
