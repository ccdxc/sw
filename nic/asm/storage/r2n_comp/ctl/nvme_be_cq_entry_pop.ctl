#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_cq_entry_k k;
struct nvme_be_cq_entry_qpop_d d;
struct phv_ p;

k = {
    ncq_ctx_idx = 1;
    ncq_ctx_c_ndx = 10;
    ncq_ctx_num_entries = 4;
    intrinsic_table_addr_raw = 0xe200c140; // 0 = phv[5=128]
};

d = {
    time_us = 0;
    be_status = 0; 
    is_q0 = 0;
    rsvd = 0;
    cmd_handle = 0xa0405060;
    nvme_status = 0;
};
