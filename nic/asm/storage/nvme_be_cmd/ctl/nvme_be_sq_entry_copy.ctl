#include "../include/ingress.h"
#include "../include/INGRESS_p.h"


struct nvme_be_sq_entry1_k k;
struct nvme_be_sq_entry1_copy_d d;
struct phv_ p;

k = {
    intrinsic_table_addr_raw = 0x9200387e; // 0 = phv[5=128]
};

d = {
    src_queue_id = 1;
    ssd_handle = 1;
    io_priority = 3;
    is_read = 0;
    cmd_handle = 0x80001000;
};


