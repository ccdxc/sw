#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct pvm_vf_sq_ctx_k k;
struct pvm_vf_sq_ctx_qpush_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x40008000; // 0 = phv[5=128]
};


d = {
    idx = 3;
    state = 0;
    c_ndx = 5;
    p_ndx = 9;
    p_ndx_db = 0x98001000;
    c_ndx_db = 0x98002000;
    base_addr = 0x98008000;
    num_entries = 6; // power of 2
    vf_id = 0;
    sq_id = 1;
    cq_id = 0;
    sq_ctx_idx = 0;
    cq_ctx_idx = 0;
    paired_q_idx = 0;
    rsvd = 0;
};
