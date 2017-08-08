#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct seq_sq_ctx_k k;
struct seq_sq_ctx_qcheck_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xB0002000; // 0 = phv[5=128]
};

d = {
    idx = 2;
    state = 0;
    c_ndx = 3;
    p_ndx = 4; 
    p_ndx_db = 0xB1002000;
    c_ndx_db = 0xB1002000;
    base_addr = 0xB2002000;
    num_entries = 5;  // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};


