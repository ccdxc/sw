#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_cq_ctx_k k;
struct r2n_cq_ctx_qcheck_d d;
struct phv_ p;


k = {
    intrinsic_table_addr_raw = 0xA0002000; // 0 = phv[5=128]
};

d = {
    idx = 3;
    state = 0;
    c_ndx = 2;
    p_ndx = 4;
    p_ndx_db = 0xA1002000;
    c_ndx_db = 0xA1002000;
    base_addr = 0xA2002000;
    num_entries = 5;  // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};
