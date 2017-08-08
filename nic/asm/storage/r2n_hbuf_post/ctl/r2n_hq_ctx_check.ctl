#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_hq_ctx_k k;
struct r2n_hq_ctx_qcheck_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xE0002000; // 0 = phv[5=128]
};

d = {
    idx = 2;
    state = 0;
    c_ndx = 1;
    p_ndx = 3;
    p_ndx_db = 0xE1002000;
    c_ndx_db = 0xE1002000;
    base_addr = 0xE2002000;
    num_entries = 4;  // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};


