#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct ssd_sq_ctx_k k;
struct ssd_sq_ctx_qpush_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x20018280; // 0 : phv[5:128]
};

d = {
    idx = 10;
    state = 0;
    c_ndx = 7;
    p_ndx = 10;
    p_ndx_db = 0x91008000;
    c_ndx_db = 0x91009000;
    base_addr = 0x92008000;
    num_entries = 6; // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};


