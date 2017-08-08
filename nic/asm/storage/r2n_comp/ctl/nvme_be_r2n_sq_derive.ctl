#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_r2n_sq_k k;
struct nvme_be_r2n_sq_derive_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0xa04050c0; // 0 = phv[5=128]
};

d = {
    src_queue_id = 1;
    ssd_handle = 2;
    io_priority = 2;
    is_read = 1;
    cmd_handle = 0xa0203040;
};
