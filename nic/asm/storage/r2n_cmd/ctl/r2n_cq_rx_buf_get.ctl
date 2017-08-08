#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_cq_rx_buf_k k;
struct r2n_cq_rx_buf_get_d d;
struct phv_ p;

k = {
    rcq_ctx_paired_q_idx = 2;
    intrinsic_table_addr_raw = 0xb000c000; // 0 = phv[5=128]
};

d = {
    cmd_handle = 0xe0010000;
    local_addr = 0xe0020000;
    data_size = 4096;
};
