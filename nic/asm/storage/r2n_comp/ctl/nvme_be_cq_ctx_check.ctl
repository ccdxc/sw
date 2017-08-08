#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_cq_ctx_k k;
struct nvme_be_cq_ctx_qcheck_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xE000C000; // 0 = phv[5=128]
};

d = {
    idx = 1;
    state = 0;
    c_ndx = 10;
    p_ndx = 15; 
    p_ndx_db = 0xE100C000;
    c_ndx_db = 0xE100C000;
    base_addr = 0xE200C000;
    num_entries = 4;  // power of 2
    paired_q_idx = 0;
    rsvd = 0;
};
