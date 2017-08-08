#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct ssd_cq_entry_k k;
struct ssd_cq_entry_qpop_d d;
struct phv_ p;

k = {
    scq_ctx_idx = 0x7;
    scq_ctx_c_ndx = 0x3;
    scq_ctx_num_entries = 0x5;
    intrinsic_table_addr_raw = 0x92002070; // 0 = phv[5=128]
};

d = {
    nvme_status = 0x0;
};

