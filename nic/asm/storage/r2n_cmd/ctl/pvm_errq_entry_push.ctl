#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct pvm_errq_ctx_k k;
struct pvm_errq_ctx_errq_push_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x40010000; // 0 = phv[5=128]
};

d = {
    idx = 0;
    state = 0;
    c_ndx = 0;
    p_ndx = 0;
    p_ndx_db = 0xC1002000;
    c_ndx_db = 0xC1002000;
    base_addr = 0xC2002000;
    num_entries = 5;  // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};
