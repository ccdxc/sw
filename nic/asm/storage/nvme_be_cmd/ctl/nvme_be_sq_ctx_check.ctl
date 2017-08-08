#include "../include/ingress.h"
#include "../include/INGRESS_p.h"

struct nvme_be_sq_ctx_k k;
struct nvme_be_sq_ctx_qcheck_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x90001000; // 0 : phv[5:128]
};

d = {
    idx = 10;
    state = 0;
    c_ndx_lo = 10;
    c_ndx_med = 12;
    c_ndx_hi = 14;
    p_ndx_lo = 0;
    p_ndx_med = 2;
    p_ndx_hi = 4;
    p_ndx_lo_db = 0x91001000;
    p_ndx_med_db = 0x91002000;
    p_ndx_hi_db = 0x91000300;
    base_addr = 0x92001000;
    num_entries = 6; // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};


