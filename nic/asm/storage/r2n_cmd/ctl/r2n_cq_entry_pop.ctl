#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct r2n_cq_entry_k k;
struct r2n_cq_entry_qpop_d d;
struct phv_ p;

k = {
    rcq_ctx_idx = 3;
    rcq_ctx_state = 0;
    rcq_ctx_c_ndx = 2;
    rcq_ctx_num_entries = 5;  // power of 2
    intrinsic_table_addr_raw = 0xa2002030; // 0 = phv[5=128]
};

d = {
    comp_type = 2;
    rsvd = 0;
    cmd_handle = 0xb000f000;
    status = 0;
    qp_index = 1;
};
