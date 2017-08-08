#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct ssd_cq_ctx_k k;
struct ssd_cq_ctx_qcheck_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x90002000; // 0 = phv[5=128]
};

d = {
    idx = 7;
    state = 0;
    c_ndx = 3;
    p_ndx = 7;
    p_ndx_db = 0x91002000;
    c_ndx_db = 0x91002000;
    base_addr = 0x92002000;
    num_entries = 5;  // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};
