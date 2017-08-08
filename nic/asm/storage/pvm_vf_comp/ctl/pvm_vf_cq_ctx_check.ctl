#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct pvm_vf_cq_ctx_k k;
struct pvm_vf_cq_ctx_qcheck_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x98001000; // 0 = phv[5=128]
};


d = {
    idx = 3;
    state = 0;
    c_ndx = 4;
    p_ndx = 13;
    p_ndx_db = 0x99001000;
    c_ndx_db = 0x99002000;
    base_addr = 0x99008000;
    num_entries = 6; // power of 2
    vf_id = 0;
    sq_id = 0;
    cq_id = 0;
    sq_ctx_idx = 0;
    cq_ctx_idx = 1;
    paired_q_idx = 0;
    rsvd = 0;
};

