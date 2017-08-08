#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_sq_ctx_k k;
struct r2n_sq_ctx_qpush_d d;
struct phv_ p;

k = {
    nvme_be_cmd_is_read = 0;
    r2n_rbuf_hdr_addr = 0xa0203040;
    intrinsic_table_addr_raw = 0x2000c040; // 0 : phv[5:128]
};

d = {
    idx = 1;
    state = 0;
    c_ndx = 10;
    p_ndx = 15;
    p_ndx_db = 0xC1008000;
    c_ndx_db = 0xC1009000;
    base_addr = 0xC2008000;
    num_entries = 4; // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};


