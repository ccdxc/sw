#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_rq_ctx_k k;
struct r2n_rq_ctx_rq_push_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x200040c0; // 0 : phv[5:128]
};

d = {
    idx = 3;
    state = 0;
    c_ndx = 10;
    p_ndx = 15;
    p_ndx_db = 0xD1008000;
    c_ndx_db = 0xD1009000;
    base_addr = 0xD2008000;
    num_entries = 4; // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};


