#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_vf_cq_ctx_k k;
struct nvme_vf_cq_ctx_qpush_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x40004040; // 0 = phv[5=128]
};

d = {
    idx = 3;
    state = 0;
    c_ndx = 0;
    p_ndx = 10;
    p_ndx_db = 0x97001000;
    c_ndx_db = 0x97002000;
    base_addr = 0x97008000;
    num_entries = 6; // power of 2
    vf_id = 2;
    sq_id = 4;
    cq_id = 1;
    sq_ctx_idx = 0;
    cq_ctx_idx = 0;
    paired_q_idx = 0;
    rsvd = 0;
};
