
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_tx_table_s3_t0_k.h"

#include "../../asm/eth/tx/defines.h"
#include "../../p4/iris/include/defines.h"

struct phv_ p;
struct tx_table_s3_t0_k_ k;


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
    p4_intr_global_drop = 0;
};

k = {
    eth_tx_t0_s2s_num_desc = 0x1;
    eth_tx_global_dma_cur_flit = 0x8;
    eth_tx_global_dma_cur_index = 0x0;
    eth_tx_to_s3_addr_lo0 = 0xbabadeadbeef;
    eth_tx_to_s3_addr_hi0 = 0x1;
    eth_tx_to_s3_len0 = 0x6e;
};
