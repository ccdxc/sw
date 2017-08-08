#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_push_cq_ctx_k k;
struct nvme_be_push_cq_ctx_qpush_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x200141c0;
};

d = {
    idx = 13;
    state = 0;
    c_ndx = 6;
    p_ndx = 11;
    p_ndx_db = 0x93008000;
    c_ndx_db = 0x93009000;
    base_addr = 0x94008000;
    num_entries = 5; // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};

