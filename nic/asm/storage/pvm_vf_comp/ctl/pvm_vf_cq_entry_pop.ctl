#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct pvm_vf_cq_entry_k k;
struct pvm_vf_cq_entry_qpop_d d;
struct phv_ p;

k = {
    pvm_vf_cq_ctx_idx = 0x3;
    pvm_vf_cq_ctx_c_ndx = 0x4;
    pvm_vf_cq_ctx_num_entries = 0x6;
    pvm_vf_cq_ctx_idx = 0x1;
    intrinsic_table_addr_raw = 0x99008030;
};

d = {
    cspec = 0; 
    rsvd0 = 0;
    sq_head = 7;
    sq_id = 3;
    cid = 4;
    phase = 0;
    status = 0;
    vf_id = 0;
    cq_ctx_idx = 1;
    rsvd = 0;
};
