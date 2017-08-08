#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_sq_entry2_k k;
struct nvme_be_sq_entry2_qpop_d d;
struct phv_ p;

k = {
    nsq_ctx_idx = 10; // 200 : phv[2:0]
    nsq_ctx_state = 0; // 184 : phv[2:16]
    nsq_ctx_c_ndx_lo = 10; // 144 = phv[2=32]
    nsq_ctx_c_ndx_med = 12; // 128 = phv[2=48]
    nsq_ctx_c_ndx_hi = 14; // 112 = phv[2=64]
    nsq_ctx_num_entries = 6; // power of 2
    nvme_be_cmd_io_priority = 2; // 64 : phv[3:432]
    intrinsic_table_addr_raw = 0x9200388e; // 0 = phv[5=128]
};

d = {
    nvme_cmd_lo = 0;
    nvme_cmd_hi = 0;
};


